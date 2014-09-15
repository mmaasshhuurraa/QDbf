#include "qdbffield.h"

#include <QDebug>

namespace QDbf {
namespace Internal {

class QDbfFieldPrivate
{
public:
    QDbfFieldPrivate(const QString &name, QVariant::Type type);
    QDbfFieldPrivate(const QDbfFieldPrivate &other);
    bool operator==(const QDbfFieldPrivate &other) const;

    QAtomicInt ref;
    QString m_name;
    QVariant::Type m_type;
    QDbfField::QDbfType m_dbfType;
    bool m_isReadOnly;
    int m_length;
    int m_precision;
    int m_offset;
    QVariant m_defaultValue;
};

QDbfFieldPrivate::QDbfFieldPrivate(const QString &name, QVariant::Type type) :
    ref(1),
    m_name(name),
    m_type(type),
    m_dbfType(QDbfField::UnknownDataType),
    m_isReadOnly(false),
    m_length(-1),
    m_precision(-1),
    m_offset(0)
{
}

QDbfFieldPrivate::QDbfFieldPrivate(const QDbfFieldPrivate &other) :
    ref(1),
    m_name(other.m_name),
    m_type(other.m_type),
    m_dbfType(other.m_dbfType),
    m_isReadOnly(other.m_isReadOnly),
    m_length(other.m_length),
    m_precision(other.m_precision),
    m_offset(other.m_offset),
    m_defaultValue(other.m_defaultValue)
{
}

bool QDbfFieldPrivate::operator==(const QDbfFieldPrivate &other) const
{
    return (m_name == other.m_name &&
            m_type == other.m_type &&
            m_dbfType == other.m_dbfType &&
            m_isReadOnly == other.m_isReadOnly &&
            m_length == other.m_length &&
            m_precision == other.m_precision &&
            m_offset == other.m_offset &&
            m_defaultValue == other.m_defaultValue);
}

} // namespace Internal

QDbfField::QDbfField(const QString &fieldName, QVariant::Type type) :
    d(new Internal::QDbfFieldPrivate(fieldName, type))
{
}

QDbfField::QDbfField(const QDbfField &other) :
    d(other.d)
{
    d->ref.ref();
    val = other.val;
}

bool QDbfField::operator==(const QDbfField &other) const
{
    return ((d == other.d || *d == *other.d) && val == other.val);
}

QDbfField &QDbfField::operator=(const QDbfField &other)
{
    if (this == &other) {
        return *this;
    }

    qAtomicAssign(d, other.d);
    val = other.val;

    return *this;
}

QDbfField::~QDbfField()
{
    if (!d->ref.deref()) {
        delete d;
        d = 0;
    }
}

void QDbfField::setValue(const QVariant &value)
{
    if (isReadOnly()) {
        return;
    }

    val = value;
}

void QDbfField::setName(const QString &name)
{
    detach();
    d->m_name = name;
}

QString QDbfField::name() const
{
    return d->m_name;
}

bool QDbfField::isNull() const
{
    return val.isNull();
}

void QDbfField::setReadOnly(bool readOnly)
{
    detach();
    d->m_isReadOnly = readOnly;
}

bool QDbfField::isReadOnly() const
{
    return d->m_isReadOnly;
}

void QDbfField::clear()
{
    if (isReadOnly()) {
        return;
    }

    val = QVariant(type());
}

void QDbfField::setType(QVariant::Type type)
{
    detach();
    d->m_type = type;
}

QVariant::Type QDbfField::type() const
{
    return d->m_type;
}

void QDbfField::setQDbfType(QDbfType type)
{
    detach();
    d->m_dbfType = type;
}

QDbfField::QDbfType QDbfField::dbfType() const
{
    return d->m_dbfType;
}

void QDbfField::setLength(int fieldLength)
{
    detach();
    d->m_length = fieldLength;
}

int QDbfField::length() const
{
    return d->m_length;
}

void QDbfField::setPrecision(int precision)
{
    detach();
    d->m_precision = precision;
}

int QDbfField::precision() const
{
    return d->m_precision;
}

void QDbfField::setOffset(int offset)
{
    detach();
    d->m_offset = offset;
}

int QDbfField::offset() const
{
    return d->m_offset;
}

void QDbfField::setDefaultValue(const QVariant &value)
{
    detach();
    d->m_defaultValue = value;
}

QVariant QDbfField::defaultValue() const
{
    return d->m_defaultValue;
}

void QDbfField::detach()
{
    qAtomicDetach(d);
}

} // namespace QDbf

QDebug operator<<(QDebug debug, const QDbf::QDbfField &field)
{
    debug.nospace() << "QDbfField("
                    << field.name() << ", "
                    << QVariant::typeToName(field.type());

    if (field.length() >= 0) {
        debug.nospace() << ", length: " << field.length();
    }

    if (field.precision() >= 0) {
        debug.nospace() << ", precision: " << field.precision();
    }

    if (!field.defaultValue().isNull()) {
        debug.nospace() << ", auto-value: \"" << field.defaultValue() << '\"';
    }

    debug.nospace() << ')';

    return debug.space();
}
