#ifndef QDBF_COMPAT_H
#define QDBF_COMPAT_H

#include <QtGlobal>

#if QT_VERSION < 0x050000
# define Q_NULLPTR NULL
# define Q_DECL_OVERRIDE
#endif

#endif // QDBF_COMPAT_H
