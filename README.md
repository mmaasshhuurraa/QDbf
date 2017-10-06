# QDbf - Qt DBF library
[![Linux Build Status](https://travis-ci.org/IvanPinezhaninov/QDbf.svg?branch=master)](https://travis-ci.org/IvanPinezhaninov/QDbf)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/github/IvanPinezhaninov/qdbf?svg=true)](https://ci.appveyor.com/project/IvanPinezhaninov/qdbf)

#### Example:
```c++
#include "qdbftable.h"
#include "qdbfrecord.h"

QDbf::QDbfTable table;
if (!table.open(filePath)) {
    qDebug() << "file open error";
    return;
}

while (table.next()) {
    QString output;
    QDbf::QDbfRecord record = table.record(); 
    for (int i = 0; i < record.count(); ++i) {
        output.append(record.fieldName(i));
        output.append(QLatin1String(": "));
        output.append(record.value(i).toString().trimmed());
        output.append(QLatin1String("; "));
    } 
    qDebug() << output;
}

table.close();
```
