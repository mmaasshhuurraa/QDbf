# QDbf - Qt DBF library

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
