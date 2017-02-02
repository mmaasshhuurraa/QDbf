#ifndef QDBFTABLE_H
#define QDBFTABLE_H

#include "qdbf_global.h"

QT_BEGIN_NAMESPACE
class QDate;
class QVariant;
QT_END_NAMESPACE

namespace QDbf {
namespace Internal {
class QDbfTablePrivate;
} // namespace Internal

class QDbfRecord;

class QDBF_EXPORT QDbfTable
{
public:
    enum Codepage {
        CodepageNotSet = 0,
        IBM437,
        IBM850,
        IBM866,
        Windows1250,
        Windows1251,
        Windows1252,
        UnsupportedCodepage
    };

    enum OpenMode {
        ReadOnly = 0,
        ReadWrite
    };

    enum DbfTableError {
        NoError = 0,
        FileOpenError,
        FileReadError,
        FileWriteError,
        InvalidValue,
        InvalidIndexError,
        InvalidTypeError,
        UnsupportedFile
    };

    QDbfTable();
    explicit QDbfTable(const QString &dbfFileName);
    ~QDbfTable();

    bool open(const QString &fileName, OpenMode openMode = QDbfTable::ReadOnly);
    bool open(OpenMode openMode = QDbfTable::ReadOnly);
    void close();

    QString fileName() const;
    QDbfTable::OpenMode openMode() const;
    DbfTableError error() const;

    bool setCodepage(QDbfTable::Codepage codepage);
    QDbfTable::Codepage codepage() const;

    bool isOpen() const;

    int size() const;
    int at() const;
    bool previous() const;
    bool next() const;
    bool first() const;
    bool last() const;
    bool seek(int index) const;

    QDate lastUpdate() const;

    bool setRecord(const QDbfRecord &record);
    QDbfRecord record() const;

    bool setValue(int fieldIndex, const QVariant &value);
    QVariant value(int fieldIndex) const;

    bool setValue(const QString &name, const QVariant &value);
    QVariant value(const QString &name) const;

    bool addRecord();
    bool addRecord(const QDbfRecord &record);

    bool removeRecord(int index);
    bool removeRecord();

private:
    Q_DISABLE_COPY(QDbfTable)
    Internal::QDbfTablePrivate *const d;
};

} // namespace QDbf

QDebug operator<<(QDebug, const QDbf::QDbfTable&);

#endif // QDBFTABLE_H
