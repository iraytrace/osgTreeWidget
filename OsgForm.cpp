#include "OsgForm.h"
#include "ui_OsgForm.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

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
}

OsgForm::~OsgForm()
{
    delete ui;
}

void OsgForm::openFile(const QString fileName)
{
    osg::Node *loaded = osgDB::readNodeFile(fileName.toStdString());
    ui->osgTreeWidget->addObject(loaded);

    m_root->addChild(loaded);
    if (m_root->getNumChildren() == 1) {
        m_viewingCore->fitToScreen();
    }
    ui->osg3dView->update();
}

bool OsgForm::saveFile(const QString fileName)
{
    if (m_root->getNumChildren() == 1) {
        return osgDB::writeNodeFile(*m_root->getChild(0), fileName.toStdString());

    }

    return osgDB::writeNodeFile(*m_root, fileName.toStdString());
}
