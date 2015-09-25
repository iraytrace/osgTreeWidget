#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "VSLapp.h"
#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMdiSubWindow>

#include "OsgForm.h"
#include "GDALloader.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_recentFiles(this),
    m_pte(new QPlainTextEdit),
    m_osgForm(new OsgForm)

{
    ui->setupUi(this);

    QMdiSubWindow *msw;

    msw = ui->mdiArea->addSubWindow(m_osgForm);
    msw->showMaximized();
    msw->setAttribute(Qt::WA_DeleteOnClose, false);

    msw= ui->mdiArea->addSubWindow(m_pte);
    msw->showMaximized();
    msw->setAttribute(Qt::WA_DeleteOnClose, false);




    m_recentFiles.attachToMenuAfterItem(ui->menuFile, "Open...", SLOT(loadFile(QString)));
    VSLapp::mainWindowSetup(this);
    connect(m_osgForm, SIGNAL(showMessage(QString)),
            ui->statusBar, SLOT(showMessage(QString)));
    connect(ui->actionClose, SIGNAL(triggered(bool)),
            m_osgForm, SLOT(removeAllNodes()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFileOpen_triggered()
{
    QSettings settings;

    QString FileTypes = QString("%1;;All Files (*.*)").arg(OsgForm::getFileExtensions());
    loadFile(QFileDialog::getOpenFileName(this, "Select File",
                                          settings.value("currentDirectory").toString(),
                                          FileTypes));
}

void MainWindow::on_actionFileSave_triggered()
{
    m_osgForm->saveFile(
                m_recentFiles.getRecentFiles().at(0));
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    QSettings settings;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save File as",
                                                    settings.value("currentDirectory").toString()
                                                    );
    m_osgForm->saveFile(fileName);
    m_recentFiles.setMostRecentFile(fileName);
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

void MainWindow::doGdalLoad(QString fileName)
{
    GDALloader *gdal = new GDALloader();

    gdal->loadTerrain(fileName);
    QString msg = gdal->getInfo();
    qDebug("%s", qPrintable(msg));
    m_pte->appendPlainText(msg);

    if (gdal->getBandCount() <= 0)
        return;

    osg::ref_ptr<osg::Geode> geode = gdal->buildGeometry(1);
    m_osgForm->addNode(geode);

}

void MainWindow::makeWidgetSubWindowVisible(QWidget *w)
{
    if (!w->isVisible()) {
        foreach (QMdiSubWindow *sw, ui->mdiArea->subWindowList()) {
            if (sw->widget() == w) {
                sw->show();
                w->setVisible(true);
            }
        }
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
    m_recentFiles.setMostRecentFile(filename);

    if (filename.endsWith(".tif")) {
        makeWidgetSubWindowVisible(m_pte);
        doGdalLoad(filename);
    } else {
        makeWidgetSubWindowVisible(m_osgForm);
        m_osgForm->openFile(filename);
    }
}

bool MainWindow::shouldAbortClose()
{
    // Ask all open documents to save/close
    // If application should not close then return "true"

    return false;
}
