#include "qdbfrecord.h"
#include "qdbftable.h"
#include "qdbftablemodel.h"
#include <QDebug>

#define DBF_PREFETCH 255

namespace QDbf {
namespace Internal {

class QDbfTableModelPrivate
{
public:
    QDbfTableModelPrivate(const QString &dbfFileName,
                          QDbfTable::OpenMode openMode,
                          QDbfTableModel *parent);

    ~QDbfTableModelPrivate();

    QDbfTableModel *const q;
    QDbfTable *const m_dbfTable;

    QDbfRecord m_record;
    QVector<QDbfRecord> m_records;
    QVector<QHash<int, QVariant> > m_headers;
    int m_deletedRecordsCount;
    int m_lastRecordIndex;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfTableModelPrivate::QDbfTableModelPrivate(const QString &dbfFileName,
                                             QDbfTable::OpenMode openMode,
                                             QDbfTableModel *parent) :
    q(parent),
    m_dbfTable(new QDbfTable(dbfFileName)),
    m_deletedRecordsCount(0),
    m_lastRecordIndex(-1)
{
    m_dbfTable->open(openMode);
    m_record = m_dbfTable->record();
}

QDbfTableModelPrivate::~QDbfTableModelPrivate()
{
    m_dbfTable->close();
    delete m_dbfTable;
}

QDbfTableModel::QDbfTableModel(const QString &dbfFileName, QDbfTable::OpenMode openMode, QObject *parent) :
    QAbstractTableModel(parent),
    d(new QDbfTableModelPrivate(dbfFileName, openMode, this))
{
    if (canFetchMore()) {
        fetchMore();
    }
}

QDbfTableModel::~QDbfTableModel()
{
    delete d;
}

int QDbfTableModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return d->m_records.count();
}

int QDbfTableModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return d->m_record.count();
}

QVariant QDbfTableModel::data(const QModelIndex &index, int role) const
{
    QVariant val;

    if (!index.isValid()) {
        return val;
    }

    if (role & ~(Qt::DisplayRole | Qt::EditRole)) {
        return val;
    }

    if (index.row() >= rowCount()) {
        return val;
    }

    if (index.column() >= columnCount()) {
        return val;
    }

    return d->m_records.at(index.row()).value(index.column());
}

bool QDbfTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section) {
        return false;
    }

    if (d->m_headers.size() <= section) {
        d->m_headers.resize(qMax(section + 1, 16));
    }

    d->m_headers[section][role] = value;

    emit headerDataChanged(orientation, section, section);

    return true;
}

QVariant QDbfTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        QVariant val = d->m_headers.value(section).value(role);

        if (role == Qt::DisplayRole && !val.isValid()) {
            val = d->m_headers.value(section).value(Qt::EditRole);
        }

        if (val.isValid()) {
            return val;
        }

        if (role == Qt::DisplayRole && d->m_record.count() > section) {
            return d->m_record.fieldName(section);
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags QDbfTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool QDbfTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);

    if (!d->m_dbfTable->isOpen()) {
        return false;
    }

    if (index.isValid() && role == Qt::EditRole) {
        QVariant oldValue = d->m_records.at(index.row()).value(index.column());
        d->m_records[index.row()].setValue(index.column(), value);

        if (!d->m_dbfTable->updateRecordInTable(d->m_records.at(index.row()))) {
            d->m_records[index.row()].setValue(index.column(), oldValue);
            return false;
        }

        emit dataChanged(index, index);

        return true;
    }

    return false;
}

bool QDbfTableModel::canFetchMore(const QModelIndex &index) const
{
    if (!index.isValid() && d->m_dbfTable->isOpen() &&
        (d->m_records.size() + d->m_deletedRecordsCount < d->m_dbfTable->size())) {
        return true;
    }

    return false;
}

void QDbfTableModel::fetchMore(const QModelIndex &index)
{
    if (index.isValid()) {
        return;
    }

    if (!d->m_dbfTable->seek(d->m_lastRecordIndex)) {
        return;
    }

    const int fetchSize = qMin(d->m_dbfTable->size() - d->m_records.count() -
                               d->m_deletedRecordsCount, DBF_PREFETCH);

    beginInsertRows(index, d->m_records.size() + 1, d->m_records.size() + fetchSize);

    int fetchedRecordsCount = 0;
    while (d->m_dbfTable->next()) {
        const QDbfRecord record(d->m_dbfTable->record());
        if (record.isDeleted()) {
            ++d->m_deletedRecordsCount;
            continue;
        }
        d->m_records.append(record);
        d->m_lastRecordIndex = d->m_dbfTable->at();
        if (++fetchedRecordsCount >= fetchSize) break;
    }

    endInsertRows();
}
