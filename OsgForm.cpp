#include "OsgForm.h"
#include "ui_OsgForm.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QMessageBox>

OsgForm::OsgForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OsgForm),
    m_root(new osg::Group)
{
    ui->setupUi(this);
    connect(ui->osgTreeWidget, SIGNAL(currentObject(osg::ref_ptr<osg::Object>)),
            ui->osgPropertyTable, SLOT(displayObject(osg::ref_ptr<osg::Object>)));
    connect(&m_loadWatcher, SIGNAL(finished()),
            this, SLOT(readNodesFinished()));
    connect(&m_saveWatcher, SIGNAL(finished()),
            this, SLOT(wrieNodesFinished()) );
    ui->osg3dView->setScene(m_root);
    m_viewingCore = ui->osg3dView->getViewingCore();
    QList<int> sz;
    sz.append(100);
    sz.append(0);
    ui->splitterLeftRight->setSizes(sz);
    ui->progressBar->hide();
}

OsgForm::~OsgForm()
{
    delete ui;
}

osg::ref_ptr<osg::Node> OsgForm::readNodes(const QString fileName)
{
    osg::ref_ptr<osg::Node> loaded = osgDB::readNodeFile(fileName.toStdString());
    return loaded;
}
void OsgForm::setProgressBarState(bool turnOn)
{
    if (turnOn) {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
        ui->progressBar->show();

    } else {
        ui->osg3dView->update();
        ui->progressBar->hide();
        ui->progressBar->setMaximum(100);

    }
}

void OsgForm::readNodesFinished()
{
    osg::ref_ptr<osg::Node> loaded = m_loadWatcher.future().result();
    QString loadedFileName = m_loadWatcher.property("filename").toString();
    emit showMessage(QString("Load '%1'' finished...").arg(loadedFileName));
    if (!loaded) {
        QMessageBox::warning(this,
                             "Nothing loaded",
                             QString("Loading %1 did not load any geometry")
                             .arg(loadedFileName));
        setProgressBarState(false);
        return;
    } else {
        qDebug("loaded Node of type %s", loaded->className());
    }
    addNode(loaded);
}
void OsgForm::addNode(osg::ref_ptr<osg::Node> n)
{
    setProgressBarState(true);
    ui->osgTreeWidget->addObject(n);

    m_root->addChild(n);
    if (m_root->getNumChildren() == 1) {
        m_viewingCore->fitToScreen();
    }
    setProgressBarState(false); // triggers an update()
}

void OsgForm::openFile(const QString fileName)
{
    if (m_loadWatcher.isRunning() || m_saveWatcher.isRunning()) {
        QMessageBox::warning(this, "I/O in progress",
                             "Please wait until the previous load/save is done");
        return;
    }

    m_loadWatcher.setProperty("filename", fileName);
    emit showMessage(QString("loading %1").arg(fileName));
    // Start the file load.

    QFuture< osg::ref_ptr<osg::Node> > future =
            QtConcurrent::run(this, &OsgForm::readNodes, fileName);
    m_loadWatcher.setFuture(future);

    setProgressBarState(true);
}
bool OsgForm::saveThread(osg::ref_ptr<osg::Node> node,
                         const QString fileName)
{
    qDebug("saving");
    return osgDB::writeNodeFile(*node, fileName.toStdString());
    qDebug("saved");
}
void OsgForm::wrieNodesFinished()
{
    qDebug("finished..");
    QString fileName = m_saveWatcher.property("filename").toString();
    if (m_saveWatcher.result())
        emit showMessage(QString("save %1 complete").arg(fileName));
    else
        emit showMessage(QString("save %1 failed").arg(fileName));

    setProgressBarState(false);
    qDebug("saved!");
}

bool OsgForm::saveFile(const QString fileName)
{
    if (m_loadWatcher.isRunning() || m_saveWatcher.isRunning()) {
        QMessageBox::warning(this, "I/O in progress",
                             "Please wait until the previous load/save is done");
        return false;
    }

    m_saveWatcher.setProperty("filename", fileName);
    emit showMessage(QString("saving %1").arg(fileName));

    osg::ref_ptr<osg::Node> n;
    if (m_root->getNumChildren() == 1)
        n = m_root->getChild(0);
    else
        n = m_root;

    QFuture< bool > future =
            QtConcurrent::run(this, &OsgForm::saveThread, n, fileName);
    m_saveWatcher.setFuture(future);

    setProgressBarState(true);
    return true;
}

