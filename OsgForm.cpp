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

void OsgForm::readNodesFinished()
{
    osg::ref_ptr<osg::Node> loaded = m_watcher.future().result();
    ui->osgTreeWidget->addObject(loaded);

    m_root->addChild(loaded);
    if (m_root->getNumChildren() == 1) {
        m_viewingCore->fitToScreen();
    }
    ui->osg3dView->update();
    ui->progressBar->hide();
    ui->progressBar->setMaximum(100);
}

void OsgForm::openFile(const QString fileName)
{
    if (m_watcher.isRunning()) {
        QMessageBox::warning(this, "Loading in progress", "Please wait until the previous load has completed");
        return;
    }

    // Start the file load.
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(readNodesFinished()));
    QFuture< osg::ref_ptr<osg::Node> > future = QtConcurrent::run(this, &OsgForm::readNodes, fileName);
    m_watcher.setFuture(future);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->show();
}

bool OsgForm::saveFile(const QString fileName)
{
    if (m_root->getNumChildren() == 1) {
        return osgDB::writeNodeFile(*m_root->getChild(0), fileName.toStdString());
    }

    return osgDB::writeNodeFile(*m_root, fileName.toStdString());
}
