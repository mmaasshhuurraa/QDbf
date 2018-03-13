#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAbstractButton;
QT_END_NAMESPACE

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

public slots:
    void openFile();

private slots:
    void openArgFile();
    void processSelectionChanged();
    void processButtonClicked(QAbstractButton *button);
    void setLastUpdate();

protected:
    void showEvent(QShowEvent* event);

private:
    Internal::MainWindowPrivate *const d;

    friend class Internal::MainWindowPrivate;
};

} // namespace Example

#endif // MAINWINDOW_H
