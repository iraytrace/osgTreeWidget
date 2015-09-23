#include "OsgForm.h"
#include "ui_OsgForm.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QMessageBox>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>

#include "VariantPtr.h"

OsgForm::OsgForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OsgForm),
    m_root(new osg::Group)
{
    ui->setupUi(this);
    setupUserInterface();
    connect(ui->osgTreeWidget, SIGNAL(currentObject(osg::ref_ptr<osg::Object>)),
            ui->osgPropertyTable, SLOT(displayObject(osg::ref_ptr<osg::Object>)));
    connect(&m_loadWatcher, SIGNAL(finished()),
            this, SLOT(readNodesFinished()));
    connect(&m_saveWatcher, SIGNAL(finished()),
            this, SLOT(wrieNodesFinished()) );
    connect(ui->osgPropertyTable, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(itemWasChangedInTable(QTableWidgetItem*)));
    connect(ui->osgTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(itemWasChangedInTree(QTreeWidgetItem*, int)));

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

void OsgForm::setupUserInterface()
{
#define numRows 4
#define numCols 8
    QGroupBox *gb = ui->layersGroupBox;
    QGridLayout *grid = new QGridLayout(gb);
    for (int row=0 ; row < numRows ; row++) {
        for (int col=0 ; col < numCols ; col++) {
            QCheckBox * cb = new QCheckBox(gb);
            cb->setCheckable(true);
            cb->setChecked(true);
            grid->addWidget(cb, row, col);
            connect(cb, SIGNAL(stateChanged(int)),
                    this, SLOT(tweakCameraMaskBit(int)));
            m_checkBoxes.append(cb);
        }
    }
}

void OsgForm::setCameraMask(osg::Node::NodeMask mask)
{
    qDebug("SET Mask %08x", (unsigned)mask);
    ui->osg3dView->getCamera()->setNodeMask(mask);
    for (int i=0 ; i < numRows*numCols ; i++) {
        bool tf = mask & (1<<i);
        m_checkBoxes.at(i)->setChecked(tf);
    }
    ui->osg3dView->update();
}


void OsgForm::tweakCameraMaskBit(int state)
{
    QCheckBox * cb = dynamic_cast<QCheckBox *>(sender());
    if (!cb) return;

    int idx = m_checkBoxes.indexOf(cb);
    if (idx < 0) return;
    osg::Node::NodeMask mask = ui->osg3dView->getCamera()->getNodeMask();
    qDebug("old Mask %08x", (unsigned)mask);
    unsigned bit = 1 << idx;
    if (state == Qt::Checked) {
        qDebug("set bit %d to %s", idx, "yes");
        mask |= bit;
    } else {
        qDebug("set bit %d to %s", idx, "no");
        mask &= ~bit;
    }
    qDebug("new Mask %08x", (unsigned)mask);
    setCameraMask(mask);
}

void OsgForm::itemWasChangedInTable(QTableWidgetItem *tabwi)
{

    // get identity of value changed
    // update in scenegraph
    // update in tree if appropriate
}

void OsgForm::itemWasChangedInTree(QTreeWidgetItem *treewi, int col)
{
    osg::Object *obj = VariantPtr<osg::Object>::asPtr(treewi->data(0,  Qt::UserRole));
    if (col == 0) {
        // user set name
        obj->setName(treewi->text(col).toStdString());
    } else if ( col == 2) {
        // user set flags
        if (osg::Node *n = dynamic_cast<osg::Node *>(obj)) {
            bool ok = false;
            unsigned mask = treewi->text(col).toUInt(&ok, 16);
            if (ok) {
                n->setNodeMask(mask);
            }
        }
    }
    ui->osgPropertyTable->displayObject(obj);
}


void OsgForm::setNodeMask(osg::ref_ptr<osg::Node> n, unsigned mask)
{

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
    ui->osg3dView->setCursor(m_stashedCursor);

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
	m_stashedCursor = ui->osg3dView->cursor();
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

