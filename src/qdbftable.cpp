#include "qdbffield.h"

#include "qdbfrecord.h"
#include "qdbftable.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QVarLengthArray>

namespace QDbf {
namespace Internal {

class QDbfTablePrivate
{
public:
    QDbfTablePrivate();
    QDbfTablePrivate(const QString &dbfFileName);
    QDbfTablePrivate(const QDbfTablePrivate &other);
    ~QDbfTablePrivate();

    enum QDbfTableType
    {
        SimpleTable,
        TableWithDbc
    };

    enum Location
    {
        BeforeFirstRow = -1,
        FirstRow = 0
    };

    bool open(const QString &fileName, QDbfTable::OpenMode openMode = QDbfTable::ReadOnly);
    bool open(QDbfTable::OpenMode openMode = QDbfTable::ReadOnly);
    void close();

    bool setCodepage(QDbfTable::Codepage m_codepage);
    QDbfTable::Codepage codepage() const;

    bool isOpen() const;
    int size() const;
    int at() const;
    bool previous() const;
    bool next() const;
    bool first() const;
    bool last() const;
    bool seek(int index) const;

    QDbfRecord record() const;
    QVariant value(int index) const;
    bool addRecord();
    bool addRecord(const QDbfRecord &record);
    bool updateRecordInTable(const QDbfRecord &record);
    bool removeRecord(int index);

    void setTextCodec();
    QByteArray recordData(const QDbfRecord &record, bool addEndOfFileMark = false) const;

    QAtomicInt ref;
    mutable QDbfTable::DbfTableError m_error;
    QString m_fileName;
    mutable QFile m_file;
    QDbfTable::OpenMode m_openMode;
    QTextCodec *m_textCodec;
    QDbfTableType m_type;
    QDbfTable::Codepage m_codepage;
    int m_headerLength;
    int m_recordLength;
    int m_fieldsCount;
    int m_recordsCount;
    QDbfRecord m_record;
    mutable bool m_bufered;
    mutable int m_currentIndex;
    mutable QDbfRecord m_currentRecord;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfTablePrivate::QDbfTablePrivate() :
    ref(1),
    m_error(QDbfTable::NoError),
    m_openMode(QDbfTable::ReadOnly),
    m_textCodec(QTextCodec::codecForLocale()),
    m_type(QDbfTablePrivate::SimpleTable),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(-1),
    m_recordLength(-1),
    m_fieldsCount(-1),
    m_recordsCount(-1),
    m_bufered(false),
    m_currentIndex(-1)
{
}

QDbfTablePrivate::QDbfTablePrivate(const QString &dbfFileName) :
    m_fileName(dbfFileName),
    ref(1),
    m_error(QDbfTable::NoError),
    m_textCodec(0),
    m_type(QDbfTablePrivate::SimpleTable),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(-1),
    m_recordLength(-1),
    m_fieldsCount(-1),
    m_recordsCount(-1),
    m_currentIndex(-1)
{
}

QDbfTablePrivate::QDbfTablePrivate(const QDbfTablePrivate &other) :
    m_fileName(other.m_fileName),
    ref(1),
    m_textCodec(other.m_textCodec),
    m_type(other.m_type),
    m_codepage(other.m_codepage),
    m_headerLength(other.m_headerLength),
    m_recordLength(other.m_recordLength),
    m_fieldsCount(other.m_fieldsCount),
    m_recordsCount(other.m_recordsCount),
    m_currentIndex(other.m_currentIndex)
{
    m_file.setFileName(other.m_fileName);
    if (other.isOpen()) {
        m_file.open(other.m_file.openMode());
    }
}

QDbfTablePrivate::~QDbfTablePrivate()
{
    if (isOpen()) {
        m_file.close();
    }
}

bool QDbfTablePrivate::open(const QString &fileName, QDbfTable::OpenMode openMode)
{
    m_fileName = fileName;
    return open(openMode);
}

bool QDbfTablePrivate::open(QDbfTable::OpenMode openMode)
{
    m_openMode = openMode;
    m_error = QDbfTable::NoError;
    m_headerLength = -1;
    m_recordLength = -1;
    m_fieldsCount = -1;
    m_recordsCount = -1;
    m_currentIndex = -1;
    m_bufered = false;
    m_record = QDbfRecord();
    m_currentRecord = QDbfRecord();

    if (isOpen()) {
        m_file.close();
    }

    m_file.setFileName(m_fileName);

    if (!m_file.open(openMode == QDbfTable::ReadWrite ? QIODevice::ReadWrite : QIODevice::ReadOnly)) {
        m_error = QDbfTable::OpenError;
        return false;
    }

    unsigned char headerData[32];

    if (m_file.read((char *) &headerData, 32) != 32) {
        return false;
    }

    // QDbfTableType
    switch(headerData[0]) {
    case 2:
    case 3:
    case 4:
    case 5:
        m_type = QDbfTablePrivate::SimpleTable;
        break;
    case 48:
    case 49:
        m_type = QDbfTablePrivate::TableWithDbc;
        break;
    default:
        return false;
    }

    m_recordsCount = (int) headerData[4] + (headerData[5] << 8) + (headerData[6] << 16) + (headerData[7] << 24);
    m_headerLength = (int) headerData[8] + (headerData[9] << 8);
    m_recordLength = (int) headerData[10] + (headerData[11] << 8);
    m_fieldsCount  = (m_headerLength - (m_type == QDbfTablePrivate::TableWithDbc ? 296 : 33)) / 32;

    // Codepage
    switch(headerData[29]) {
    case 0:
        m_codepage = QDbfTable::CodepageNotSet;
        break;
    case 101:
        m_codepage = QDbfTable::IBM866;
        break;
    case 201:
        m_codepage = QDbfTable::Windows1251;
        break;
    default:
        m_codepage = QDbfTable::UnspecifiedCodepage;
    }

    // set text codec
    setTextCodec();

    // Fields headers
    const int fieldsHeadersLength = m_fieldsCount * 32;

    QVarLengthArray<char> fieldsHeadersData(fieldsHeadersLength);

    if (m_file.read(fieldsHeadersData.data(), fieldsHeadersLength) != fieldsHeadersLength) {
        return false;
    }

    int offset = 1;
    for (int i = 0; i < fieldsHeadersLength; i += 32) {
        QString fieldName = QString::null;
        for (int j = 0; j < 11; j++) {
            if (fieldsHeadersData[i + j] == 0) continue;
            fieldName.append(QString(fieldsHeadersData[i + j]));
        }

        QVariant::Type fieldType;
        QDbfField::QDbfType fieldQDbfType;
        unsigned char fieldTypeChar = (unsigned char) fieldsHeadersData[i + 11];
        switch (fieldTypeChar) {
        case 67: // C
            fieldType = QVariant::String;
            fieldQDbfType = QDbfField::Character;
            break;
        case 68: // D
            fieldType = QVariant::Date;
            fieldQDbfType = QDbfField::Date;
            break;
        case 70: // F
            fieldType = QVariant::Double;
            fieldQDbfType = QDbfField::FloatingPoint;
            break;
        case 76: // L
            fieldType = QVariant::Bool;
            fieldQDbfType = QDbfField::Logical;
            break;
        /*case 77: // M
            fieldType = QVariant::String;
            break;*/
        case 78: // N
            fieldType = QVariant::Double;
            fieldQDbfType = QDbfField::Number;
            break;
        default:
            fieldType = QVariant::Invalid;
        }

        const int fieldLength = (unsigned char) fieldsHeadersData[i + 16];
        const int fieldPrecision = (unsigned char) fieldsHeadersData[i + 17];
        const int fieldOffset = offset;

        QDbfField field(fieldName, fieldType);
        field.setQDbfType(fieldQDbfType);
        field.setLength(fieldLength);
        field.setPrecision(fieldPrecision);
        field.setOffset(fieldOffset);
        m_record.append(field);

        offset += fieldLength;
    }

    return true;
}

void QDbfTablePrivate::close()
{
    if (isOpen()) {
        m_file.close();
    }
}

bool QDbfTablePrivate::setCodepage(QDbfTable::Codepage codepage)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::setCodepage(): IODevice is not open");
        return false;
    }

    if (!m_file.isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_file.seek(29);
    unsigned char byte;
    switch(codepage) {
    case QDbfTable::CodepageNotSet:
        byte = static_cast<unsigned char>(0);
    case QDbfTable::IBM866:
        byte = static_cast<unsigned char>(101);
        break;
    case QDbfTable::Windows1251:
        byte = static_cast<unsigned char>(201);
        break;
    default:
        return false;
    }

    if (m_file.write(reinterpret_cast<char *>(&byte), 1) != 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_codepage = codepage;
    setTextCodec();

    m_error = QDbfTable::NoError;

    return true;
}

QDbfTable::Codepage QDbfTablePrivate::codepage() const
{
    return m_codepage;
}

bool QDbfTablePrivate::isOpen() const
{
    return m_file.isOpen();
}

int QDbfTablePrivate::size() const
{
    return m_recordsCount;
}

int QDbfTablePrivate::at() const
{
    return m_currentIndex;
}

bool QDbfTablePrivate::previous() const
{
    if (at() <= QDbfTablePrivate::FirstRow) {
        return false;
    }

    if (at() > (size() - 1)) {
        return last();
    }

    return seek(at() - 1);
}

bool QDbfTablePrivate::next() const
{
    if (at() < QDbfTablePrivate::FirstRow) {
        return first();
    }

    if (at() >= (size() - 1)) {
        return false;
    }

    return seek(at() + 1);
}

bool QDbfTablePrivate::first() const
{
    return seek(QDbfTablePrivate::FirstRow);
}

bool QDbfTablePrivate::last() const
{
    return seek(size() - 1);
}

bool QDbfTablePrivate::seek(int index) const
{
    const int previousIndex = m_currentIndex;

    if (index < QDbfTablePrivate::FirstRow) {
        m_currentIndex = QDbfTablePrivate::BeforeFirstRow;
    } else if (index > (size() - 1)) {
        m_currentIndex = size() - 1;
    } else {
        m_currentIndex = index;
    }

    if (previousIndex != m_currentIndex) {
        m_bufered = false;
    }

    return true;
}

QDbfRecord QDbfTablePrivate::record() const
{
    if (m_bufered) {
        return m_currentRecord;
    }

    m_currentRecord = m_record;
    m_bufered = true;

    if (m_currentIndex < QDbfTablePrivate::FirstRow) {
        return m_currentRecord;
    }

    if (!isOpen()) {
        qWarning("QDbfTablePrivate::record(): IODevice is not open");
        return m_currentRecord;
    }

    if (!m_file.isReadable()) {
        m_error = QDbfTable::ReadError;
        return m_currentRecord;
    }

    const qint64 position = m_headerLength + m_recordLength * m_currentIndex;

    if (!m_file.seek(position)) {
        m_error = QDbfTable::ReadError;
        return m_currentRecord;
    }

    m_currentRecord.setRecordIndex(m_currentIndex);

    const QByteArray recordData = m_file.read(m_recordLength);

    if (recordData.count() == 0) {
        m_error = QDbfTable::UnspecifiedError;
        return m_currentRecord;
    }

    m_currentRecord.setDeleted(recordData.at(0) == 42 ? true : false);

    for (int i = 0; i < m_currentRecord.count(); ++i) {
        const QByteArray byteArray = recordData.mid(m_currentRecord.field(i).offset(),
                                                    m_currentRecord.field(i).length());
        QVariant value;
        switch (m_currentRecord.field(i).type()) {
        case QVariant::String:
            value = m_textCodec->toUnicode(byteArray);
            break;
        case QVariant::Date:
            value = QVariant(QDate(byteArray.mid(0, 4).toInt(),
                                   byteArray.mid(4, 2).toInt(),
                                   byteArray.mid(6, 2).toInt()));
            break;
        case QVariant::Double:
            value = byteArray.toDouble();
            break;
        case QVariant::Bool: {
            QString val = byteArray.toUpper();
            if (val == QLatin1String("T") ||
                val == QLatin1String("Y")) {
                value = true;
            } else {
                value = false;
            }
            break; }
        default:
            value = QVariant::Invalid;
        }

        m_currentRecord.setValue(i, value);
    }

    m_error = QDbfTable::NoError;

    return m_currentRecord;
}

QVariant QDbfTablePrivate::value(int index) const
{
    return record().value(index);
}

bool QDbfTablePrivate::addRecord()
{
    QDbfRecord newRecord(record());
    newRecord.clearValues();
    newRecord.setDeleted(false);
    return addRecord(newRecord);
}

bool QDbfTablePrivate::addRecord(const QDbfRecord &record)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::addRecord(): IODevice is not open");
        return false;
    }

    if (!m_file.isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    QByteArray data = recordData(record, true);

    const qint64 position = m_headerLength + m_recordLength * m_recordsCount;

    if (!m_file.seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file.write(data) != static_cast<qint64>(m_recordLength) + 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    int recordsCount = m_recordsCount + 1;

    unsigned char recordsCountChars[4];
    int shift = 0;
    for (int i = 0; i < 4; ++i) {
        recordsCountChars[i] = recordsCount >> shift;
        shift += 8;
    }

    if (!m_file.seek(4)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file.write((const char *) recordsCountChars, 4) != 4) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_recordsCount++;

    m_error = QDbfTable::NoError;

    return true;
}

bool QDbfTablePrivate::updateRecordInTable(const QDbfRecord &record)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::addRecord(): IODevice is not open");
        return false;
    }

    if (!m_file.isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    QByteArray data = recordData(record);

    const qint64 position = m_headerLength + m_recordLength * record.recordIndex();

    if (!m_file.seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file.write(data) != static_cast<qint64>(m_recordLength)) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_error = QDbfTable::NoError;

    return true;
}

bool QDbfTablePrivate::removeRecord(int index)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::removeRecord(): IODevice is not open");
        return false;
    }

    if (!m_file.isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    if (index < QDbfTablePrivate::FirstRow ||
        index > (size() - 1)) {
        m_error = QDbfTable::UnspecifiedError;
        return false;
    }

    const qint64 position = m_headerLength + m_recordLength * index;

    if (!m_file.seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    const QByteArray recordData = m_file.read(m_recordLength);

    if (recordData.count() != m_recordLength) {
        m_error = QDbfTable::UnspecifiedError;
        return false;
    }

    unsigned char byte = static_cast<unsigned char>(42);

    if (m_file.write(reinterpret_cast<char *>(&byte), 1) != 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_error = QDbfTable::NoError;

    return true;
}

void QDbfTablePrivate::setTextCodec()
{
    switch (m_codepage) {
    case QDbfTable::Windows1251:
        m_textCodec = QTextCodec::codecForName("Windows-1251");
        break;
    case QDbfTable::IBM866:
        m_textCodec = QTextCodec::codecForName("IBM 866");
        break;
    default:
        m_textCodec = QTextCodec::codecForLocale();
    }
}

QByteArray QDbfTablePrivate::recordData(const QDbfRecord &record, bool addEndOfFileMark) const
{
    QByteArray data;
    // delete flag
    data.append(record.isDeleted() ? '*' : ' ');
    // field
    for (int i = 0; i < m_record.count(); ++i) {
        if (m_record.field(i).d != record.field(i).d) {
            m_error = QDbfTable::UnspecifiedError;
            return false;
        }
        switch (record.field(i).dbfType()) {
        case QDbfField::Character:
            data.append(m_textCodec->fromUnicode(record.field(i).value().toString().leftJustified(record.field(i).length(), ' ', true)));
            break;
        case QDbfField::Date:
            data.append(record.field(i).value().toDate().toString(QString("yyyyMMdd")).leftJustified(record.field(i).length(), ' ', true));
            break;
        case QDbfField::FloatingPoint:
        case QDbfField::Number:
            data.append(QString("%1").arg(record.field(i).value().toDouble(), 0, 'f', record.field(i).precision()).rightJustified(record.field(i).length(), ' ', true));
            break;
        case QDbfField::Logical:
            data.append(record.field(i).value().toBool() ? 'T' : 'F');
            break;
        default:
            data.append(QString("").leftJustified(record.field(i).length(), QChar(' '), true));
        }
    }

    if (addEndOfFileMark) {
        data.append(QChar(26));
    }

    return data;
}

QDbfTable::QDbfTable() :
    d(new QDbfTablePrivate())
{
}

QDbfTable::QDbfTable(const QString &dbfFileName) :
    d(new QDbfTablePrivate(dbfFileName))
{
}

QDbfTable::QDbfTable(const QDbfTable &other) :
    d(other.d)
{
    d->ref.ref();
}

QDbfTable &QDbfTable::operator=(const QDbfTable &other)
{
    if (this == &other) {
        return *this;
    }
    qAtomicAssign(d, other.d);
    return *this;
}

bool QDbfTable::operator==(const QDbfTable &other) const
{
    return (d->m_file.fileName() == other.d->m_file.fileName() &&
            d->m_type == other.d->m_type &&
            d->m_codepage == other.d->m_codepage &&
            d->m_headerLength == other.d->m_headerLength &&
            d->m_recordLength == other.d->m_recordLength &&
            d->m_fieldsCount == other.d->m_fieldsCount &&
            d->m_recordsCount == other.d->m_recordsCount);
}

QDbfTable::~QDbfTable()
{
    if (!d->ref.deref()) {
        delete d;
    }
}

QString QDbfTable::fileName() const
{
    return d->m_file.fileName();
}

QDbfTable::OpenMode QDbfTable::openMode() const
{
    return d->m_openMode;
}

QDbfTable::DbfTableError QDbfTable::error() const
{
    return d->m_error;
}

bool QDbfTable::open(const QString &fileName, OpenMode openMode)
{
    return d->open(fileName, openMode);
}

void QDbfTable::close()
{
    d->close();
}

bool QDbfTable::open(OpenMode openMode)
{
    return d->open(openMode);
}

bool QDbfTable::setCodepage(QDbfTable::Codepage codepage)
{
    return d->setCodepage(codepage);
}

QDbfTable::Codepage QDbfTable::codepage() const
{
    return d->codepage();
}

bool QDbfTable::isOpen() const
{
    return d->isOpen();
}

int QDbfTable::size() const
{
    return d->size();
}

int QDbfTable::at() const
{
    return d->at();
}

bool QDbfTable::previous() const
{
    return d->previous();
}

bool QDbfTable::next() const
{
    return d->next();
}

bool QDbfTable::first() const
{
    return d->first();
}

bool QDbfTable::last() const
{
    return d->last();
}

bool QDbfTable::seek(int index) const
{
    return d->seek(index);
}

QDbfRecord QDbfTable::record() const
{
    return d->record();
}

QVariant QDbfTable::value(int index) const
{
    return d->value(index);
}

bool QDbfTable::addRecord()
{
    return d->addRecord();
}

bool QDbfTable::addRecord(const QDbfRecord &record)
{
    return d->addRecord(record);
}

bool QDbfTable::updateRecordInTable(const QDbfRecord &record)
{
    return d->updateRecordInTable(record);
}

bool QDbfTable::removeRecord(int index)
{
    return d->removeRecord(index);
}

QDebug operator<<(QDebug debug, const QDbfTable &table)
{
    debug.nospace() << "QDbfTable("
                    << table.fileName() << ", "
                    << "size: " << table.record().count()
                    << " x " << table.size() << ')';

    return debug.space();
}
