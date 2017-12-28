/****************************************************************************
**
** Copyright (C) 2017 Ivan Pinezhaninov <ivan.pinezhaninov@gmail.com>
**
** This file is part of the QDbf - Qt DBF library
** which can be found at https://github.com/IvanPinezhaninov/qdbf/.
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License,
** or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/


#include "mainwindow.h"

#include "qdbftablemodel.h"

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

static const int MESSAGE_TIMEOUT = 1000;

static const char *const ADD_ROW_BUTTON_TEXT = "Add row";
static const char *const ADD_ROW_BUTTON_TOOL_TIP = "Add row to the end of the table";
static const char *const ADD_ROW_ERROR_TEXT = "Add row failed";
static const char *const ERROR_MESSAGE_TITLE = "Error";
static const char *const FILE_OPENED_MESSAGE = "File opened";
static const char *const OPEN_FILE_CAPTION = "Select dbf file";
static const char *const OPEN_FILE_FILTER = "xBase files (*.dbf)";
static const char *const OPEN_FILE_ERROR_TEXT = "Can not open file %1";
static const char *const REMOVE_ROW_BUTTON_TEXT = "Remove row";
static const char *const REMOVE_ROW_BUTTON_TOOL_TIP = "Remove selected row";
static const char *const REMOVE_ROW_ERROR_TEXT = "Remove row failed";
static const char *const ROW_ADDED_MESSAGE = "Row added";
static const char *const ROW_REMOVED_MESSAGE = "Row removed";
static const char *const SELECT_FILE_TEXT = "Select dbf file by clicking on the button ----->";
static const char *const TABLE_LAST_UPDATE_LABEL_TOOLTIP = "Table last update";

namespace Example {
namespace Internal {

class MainWindowPrivate
{
    Q_DECLARE_TR_FUNCTIONS(MainWindow)

public:
    MainWindowPrivate();
    ~MainWindowPrivate();

    void init();
    void openFile();
    void openFile(const QString &file);

    MainWindow *q;
    QWidget *const m_centralWidget;
    QStatusBar *const m_statusBar;
    QLabel *const m_tableLastUpdateLabel;
    QVBoxLayout *const m_baseLayout;
    QHBoxLayout *const m_fileLocationLayout;
    QLineEdit *const m_fileLocationEditor;
    QPushButton *const m_selectFileButton;
    QTableView *const m_tableView;
    QDbf::QDbfTableModel *const m_model;
    QDialogButtonBox *const m_buttonBox;
    QPushButton *const m_addRowButton;
    QPushButton *const m_removeRowButton;
    QDir m_dir;
};

} // namespace Internal
} // namespace Example

using namespace Example;
using namespace Example::Internal;

MainWindowPrivate::MainWindowPrivate() :
    q(0),
    m_centralWidget(new QWidget()),
    m_statusBar(new QStatusBar()),
    m_tableLastUpdateLabel(new QLabel()),
    m_baseLayout(new QVBoxLayout(m_centralWidget)),
    m_fileLocationLayout(new QHBoxLayout()),
    m_fileLocationEditor(new QLineEdit()),
    m_selectFileButton(new QPushButton(QString::fromLatin1("..."), 0)),
    m_tableView(new QTableView()),
    m_model(new QDbf::QDbfTableModel()),
    m_buttonBox(new QDialogButtonBox()),
    m_addRowButton(new QPushButton(trUtf8(ADD_ROW_BUTTON_TEXT))),
    m_removeRowButton(new QPushButton(trUtf8(REMOVE_ROW_BUTTON_TEXT))),
    m_dir(QCoreApplication::applicationDirPath())
{
}

MainWindowPrivate::~MainWindowPrivate()
{
    delete m_removeRowButton;
    delete m_addRowButton;
    delete m_buttonBox;
    delete m_model;
    delete m_tableView;
    delete m_selectFileButton;
    delete m_fileLocationEditor;
    delete m_fileLocationLayout;
    delete m_baseLayout;
    delete m_tableLastUpdateLabel;
    delete m_statusBar;
    delete m_centralWidget;
}

void MainWindowPrivate::init()
{
    q->setCentralWidget(m_centralWidget);

    m_tableLastUpdateLabel->setToolTip(trUtf8(TABLE_LAST_UPDATE_LABEL_TOOLTIP));
    m_statusBar->addPermanentWidget(m_tableLastUpdateLabel);
    q->setStatusBar(m_statusBar);

    m_baseLayout->setContentsMargins(0, 0, 0, 0);

    m_fileLocationLayout->setContentsMargins(2, 2, 2, 2);
    m_baseLayout->addLayout(m_fileLocationLayout);

    m_fileLocationEditor->setReadOnly(true);
    m_fileLocationEditor->setText(trUtf8(SELECT_FILE_TEXT));
    m_fileLocationLayout->addWidget(m_fileLocationEditor);

    m_selectFileButton->setFixedWidth(30);
    m_selectFileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    q->connect(m_selectFileButton, SIGNAL(clicked()), SLOT(openFile()));
    m_fileLocationLayout->addWidget(m_selectFileButton);

    q->connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(setLastUpdate()));
    m_tableView->setModel(m_model);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_baseLayout->addWidget(m_tableView);

    m_buttonBox->addButton(m_addRowButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_removeRowButton, QDialogButtonBox::ActionRole);
    q->connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(processButtonClicked(QAbstractButton*)));
    m_baseLayout->addWidget(m_buttonBox);

    m_addRowButton->setToolTip(trUtf8(ADD_ROW_BUTTON_TOOL_TIP));
    m_addRowButton->setEnabled(false);

    m_removeRowButton->setToolTip(trUtf8(REMOVE_ROW_BUTTON_TOOL_TIP));
    m_removeRowButton->setEnabled(false);

    QTimer::singleShot(100, q, SLOT(openArgFile()));
}

void MainWindowPrivate::openFile()
{
    const QString &caption = trUtf8(OPEN_FILE_CAPTION);
    const QString &filter = trUtf8(OPEN_FILE_FILTER);
    const QString &file = QFileDialog::getOpenFileName(q, caption, m_dir.absolutePath(), filter);

    if (!file.isEmpty()) {
        openFile(file);
    }
}

void MainWindowPrivate::openFile(const QString &file)
{
    QFileInfo fileInfo(file);
    m_dir = fileInfo.dir();

    m_tableView->setModel(0);
    m_addRowButton->setEnabled(false);
    m_removeRowButton->setEnabled(false);

    if (!m_model->open(file)) {
        m_fileLocationEditor->setText(QLatin1String(SELECT_FILE_TEXT));
        const QString &title = trUtf8(ERROR_MESSAGE_TITLE);
        const QString &text = trUtf8(OPEN_FILE_ERROR_TEXT).arg(file);
        QMessageBox::warning(q, title, text, QMessageBox::Ok);
    } else {
        m_tableView->setModel(m_model);
        m_addRowButton->setEnabled(true);
        m_fileLocationEditor->setText(file);
        m_statusBar->showMessage(trUtf8(FILE_OPENED_MESSAGE), MESSAGE_TIMEOUT);
        q->setLastUpdate();
        q->connect(m_tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(processSelectionChanged()));
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    d(new MainWindowPrivate())
{
    d->q = this;
    d->init();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::openFile()
{
    d->openFile();
}

void MainWindow::openArgFile()
{
    const QStringList &args = QCoreApplication::arguments();
    if (1 < args.size()) {
        d->openFile(args.last());
    }
}

void MainWindow::processSelectionChanged()
{
    Q_ASSERT(d->m_tableView && d->m_tableView->selectionModel());
    d->m_removeRowButton->setEnabled(d->m_tableView->selectionModel()->currentIndex().isValid());
}

void MainWindow::processButtonClicked(QAbstractButton* button)
{
    Q_ASSERT(button);

    const QString &errorMessageTitle = trUtf8(ERROR_MESSAGE_TITLE);

    if (button == d->m_addRowButton) {
      if (!d->m_model->insertRow(d->m_model->rowCount())) {
          QMessageBox::warning(this, errorMessageTitle, trUtf8(ADD_ROW_ERROR_TEXT), QMessageBox::Ok);
      } else {
          d->m_statusBar->showMessage(trUtf8(ROW_ADDED_MESSAGE), MESSAGE_TIMEOUT);
          setLastUpdate();
      }
    } else if (button == d->m_removeRowButton) {
        Q_ASSERT(d->m_tableView->selectionModel()->currentIndex().isValid());
        if (!d->m_model->removeRow(d->m_tableView->selectionModel()->currentIndex().row())) {
            QMessageBox::warning(this, errorMessageTitle, trUtf8(REMOVE_ROW_ERROR_TEXT), QMessageBox::Ok);
        } else {
            d->m_statusBar->showMessage(trUtf8(ROW_REMOVED_MESSAGE), MESSAGE_TIMEOUT);
            setLastUpdate();
        }
    } else {
        Q_ASSERT(false);
    }
}

void MainWindow::setLastUpdate()
{
    Q_ASSERT(d->m_model);
    d->m_tableLastUpdateLabel->setText(d->m_model->lastUpdate().toString(Qt::SystemLocaleShortDate));
}
