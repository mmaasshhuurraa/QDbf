#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Example {
namespace Internal {

class MainWindowPrivate;

} // namespace Internal


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Internal::MainWindowPrivate *const d;

public slots:
    void openFile();

    friend class Internal::MainWindowPrivate;
};

} // namespace Example

#endif // MAINWINDOW_H
