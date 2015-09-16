#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "VSLapp.h"
#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_recentMenu(this)

{
    ui->setupUi(this);
    m_recentMenu.attachToMenuAfterItem(ui->menuFile, "Open...", SLOT(loadFile(QString)));
    setWindowTitle(qApp->applicationName());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFileOpen_triggered()
{
    QSettings setttings;

    QString fileName = QFileDialog::getOpenFileName(this, "Select File",
                                                    setttings.value("currentDirectory").toString(),
                                                    "All Files (*.*)");

    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

void MainWindow::on_actionFileSave_triggered()
{
    qDebug("save");
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    qDebug("saveAs");

}

void MainWindow::on_actionHelpAbout_triggered()
{
    VSLapp::showAboutDialog(this);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (shouldAbortClose()) {
        event->ignore();
    } else {
        /* writeSettings(); */
        event->accept();
    }
}

void MainWindow::loadFile(QString fileName)
{
    QSettings settings;
    QFileInfo fi(fileName);

    settings.setValue("currentDirectory", fi.absolutePath());
    m_recentMenu.setMostRecentFile(fileName);

    ui->osgForm->openFile(fileName);
}

bool MainWindow::shouldAbortClose()
{
    // Ask all open documents to save/close
    // if any object is not done then return "true"

    return false;
}



