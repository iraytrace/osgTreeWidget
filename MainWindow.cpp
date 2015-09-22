#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "VSLapp.h"
#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_recentMenu(this)

{
    ui->setupUi(this);
    m_recentMenu.attachToMenuAfterItem(ui->menuFile, "Open...", SLOT(loadFile(QString)));
    VSLapp::mainWindowSetup(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFileOpen_triggered()
{
    QSettings settings;
    loadFile(QFileDialog::getOpenFileName(this, "Select File",
                                          settings.value("currentDirectory").toString(),
                                          "All Files (*.*)"));
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
        VSLapp::mainWindowSave(this);
        event->accept();
    }
}
void MainWindow::loadFile(QString filename)
{
    if (filename.isEmpty())
        return;

    QSettings settings;
    QFileInfo fi(filename);

    if (!fi.exists()) {
        QMessageBox::warning(this, "File Open Error", QString("File '%1' does not exist").arg(filename));
        return;
    }
    settings.setValue("currentDirectory", fi.absolutePath());
    m_recentMenu.setMostRecentFile(filename);

    if (filename.endsWith(".osg") ||
            filename.endsWith(".ive") ||
            filename.endsWith(".osgt") ||
            filename.endsWith(".osgb") ||
            filename.endsWith(".osgx") ||
            filename.endsWith(".obj")
            ) {
        ui->osgForm->openFile(filename);
    }
}

bool MainWindow::shouldAbortClose()
{
    // Ask all open documents to save/close
    // If application should not close then return "true"

    return false;
}
