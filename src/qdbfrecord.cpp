#include "qdbffield.h"

#include "qdbfrecord.h"

#include <QDebug>
#include <QVariant>
#include <QVector>

namespace QDbf {
namespace Internal {

class QDbfRecordPrivate
{
public:
    QDbfRecordPrivate();
    QDbfRecordPrivate(const QDbfRecordPrivate &other);

    inline bool contains(int index) { return index >= 0 && index < m_fields.count(); }

    QAtomicInt ref;
    int m_index;
    bool m_isDeleted;
    QVector<QDbfField> m_fields;
};

QDbfRecordPrivate::QDbfRecordPrivate() :
    ref(1),
    m_index(-1),
    m_isDeleted(false)
{
}

QDbfRecordPrivate::QDbfRecordPrivate(const QDbfRecordPrivate &other) :
    ref(1),
    m_index(other.m_index),
    m_isDeleted(other.m_isDeleted),
    m_fields(other.m_fields)
{
}

} // namespace Internal

QDbfRecord::QDbfRecord() :
    d(new Internal::QDbfRecordPrivate())
{
}

QDbfRecord::QDbfRecord(const QDbfRecord &other) :
    d(other.d)
{
    d->ref.ref();
}

QDbfRecord &QDbfRecord::operator=(const QDbfRecord &other)
{
    if (this == &other) return *this;
    qAtomicAssign(d, other.d);
    return *this;
}

bool QDbfRecord::operator==(const QDbfRecord &other) const
{
    return (recordIndex() == other.recordIndex() &&
            isDeleted() == other.isDeleted() &&
            d->m_fields == other.d->m_fields);
}

QDbfRecord::~QDbfRecord()
{
    if (!d->ref.deref()) {
        delete d;
    }
}

void QDbfRecord::setRecordIndex(int index)
{
    d->m_index = index;
}

int QDbfRecord::recordIndex() const
{
    return d->m_index;
}

void QDbfRecord::setValue(int index, const QVariant &val)
{
    if (!d->contains(index)) {
        return;
    }

    detach();
    d->m_fields[index].setValue(val);
}

QVariant QDbfRecord::value(int index) const
{
    return d->m_fields.value(index).value();
}

void QDbfRecord::setValue(const QString &name, const QVariant &val)
{
    setValue(indexOf(name), val);
}

QVariant QDbfRecord::value(const QString &name) const
{
    return value(indexOf(name));
}

void QDbfRecord::setNull(int index)
{
    if (!d->contains(index)) {
        return;
    }

    detach();
    d->m_fields[index].clear();
}

bool QDbfRecord::isNull(int index) const
{
    return d->m_fields.value(index).isNull();
}

void QDbfRecord::setNull(const QString &name)
{
    setNull(indexOf(name));
}

bool QDbfRecord::isNull(const QString &name) const
{
    return isNull(indexOf(name));
}

int QDbfRecord::indexOf(const QString &name) const
{
    QString nm = name.toUpper();

    for (int i = 0; i < count(); ++i) {
        if (d->m_fields.at(i).name().toUpper() == nm) return i;
    }

    return -1;
}

QString QDbfRecord::fieldName(int index) const
{
    return d->m_fields.value(index).name();
}

QDbfField QDbfRecord::field(int index) const
{
    return d->m_fields.value(index);
}

QDbfField QDbfRecord::field(const QString &name) const
{
    return field(indexOf(name));
}

void QDbfRecord::append(const QDbfField &field)
{
    detach();
    d->m_fields.append(field);
}

void QDbfRecord::replace(int pos, const QDbfField &field)
{
    if (!d->contains(pos)) {
        return;
    }

    detach();
    d->m_fields[pos] = field;
}

void QDbfRecord::insert(int pos, const QDbfField &field)
{
    detach();
    d->m_fields.insert(pos, field);
}

void QDbfRecord::remove(int pos)
{
    if (!d->contains(pos)) {
        return;
    }

    detach();
    d->m_fields.remove(pos);
}

bool QDbfRecord::isEmpty() const
{
    return d->m_fields.isEmpty();
}

void QDbfRecord::setDeleted(bool deleted)
{
    detach();
    d->m_isDeleted = deleted;
}

bool QDbfRecord::isDeleted() const
{
    return d->m_isDeleted;
}

bool QDbfRecord::contains(const QString &name) const
{
    return indexOf(name) >= 0;
}

void QDbfRecord::clear()
{
    detach();
    d->m_fields.clear();
}

void QDbfRecord::clearValues()
{
    detach();
    int count = d->m_fields.count();
    for (int i = 0; i < count; ++i) {
        d->m_fields[i].clear();
    }
}

int QDbfRecord::count() const
{
    return d->m_fields.count();
}

void QDbfRecord::detach()
{
    qAtomicDetach(d);
}

} // namespace QDbf

QDebug operator<<(QDebug debug, const QDbf::QDbfRecord &record)
{
    debug.nospace() << "QDbfRecord(" << record.count() << ')';

    for (int i = 0; i < record.count(); ++i) {
        debug.nospace() << '\n' << QString::fromLatin1("%1:").arg(i, 2)
                        << record.field(i) << record.value(i).toString();
    }

    return debug.space();
}
