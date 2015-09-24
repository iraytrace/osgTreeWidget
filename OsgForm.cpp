#include "OsgForm.h"
#include "ui_OsgForm.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QRegExpValidator>

#include "VariantPtr.h"

#define numRowsOfLayerButtons 2
#define numColsOfLayerButtons 16

OsgForm::OsgForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OsgForm),
    m_root(new osg::Group),
    m_loadedModel(new osg::Group)
{
    ui->setupUi(this);
    setupUserInterface();
    m_root->setName("root");
    m_loadedModel->setName("loadedModel");
    m_root->addChild(m_loadedModel);

    ui->osg3dView->setScene(m_root);
    m_viewingCore = ui->osg3dView->getViewingCore();

    connect(ui->osg3dView, SIGNAL(pickObject(QVector<osg::ref_ptr<osg::Node> >)),
            this, SLOT(handlePick(QVector<osg::ref_ptr<osg::Node> >)));
}

OsgForm::~OsgForm()
{
    delete ui;
}

QString OsgForm::getFileExtensions()
{
    return QString("OpenSceneGraph (*.osg *.ive *.osgb *.osgt *.osgx *.obj *.lwo *.stl *.dxf *.bsp *.3ds *.3dc)");
}

void OsgForm::removeAllNodes()
{
    m_loadedModel->removeChildren(0, m_loadedModel->getNumChildren());
    ui->osg3dView->update();
    ui->osgTreeWidget->clear();
    ui->osgPropertyTable->clear();
}

void OsgForm::buildLayerBox()
{
    QGridLayout *grid = new QGridLayout(ui->layerFrame);
    for (int row=0 ; row < numRowsOfLayerButtons ; row++) {
        for (int col=0 ; col < numColsOfLayerButtons ; col++) {
            QCheckBox * cb = new QCheckBox(ui->layerFrame);
            cb->setCheckable(true);
            cb->setChecked(false);
            cb->setMinimumWidth(12);
            cb->setMinimumHeight(16);
            grid->addWidget(cb, row, (numColsOfLayerButtons-1)-col);
            connect(cb, SIGNAL(stateChanged(int)),
                    this, SLOT(tweakCameraMaskBit(int)));
            m_checkBoxes.append(cb);
        }
    }
    grid->setHorizontalSpacing(0);
    grid->setVerticalSpacing(0);
    grid->setContentsMargins(0,0,0,0);
}

void OsgForm::setupUserInterface()
{
    buildLayerBox();
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
    connect(ui->cameraMaskLineEdit, SIGNAL(editingFinished()),
            this, SLOT(setCameraMaskFromLineEdit()));

    QList<int> sz;
    sz.append(100);
    for (int i=1 ; i < ui->splitterLeftRight->children().size() ; i++)
        sz.append(0);
    ui->splitterLeftRight->setSizes(sz);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);
    ui->splitter->setStretchFactor(2, 0);

    ui->progressBar->hide();
    this->setCameraMask(ui->osg3dView->getCamera()->getCullMask());

    QRegExpValidator *rev = new QRegExpValidator(QRegExp("[0-9a-fA-F]{1,8}"));
    ui->cameraMaskLineEdit->setValidator(rev);

}

void OsgForm::setCameraMask(osg::Node::NodeMask mask)
{
    ui->osg3dView->getCamera()->setCullMask(mask);
    for (int i=0 ; i < numRowsOfLayerButtons*numColsOfLayerButtons ; i++) {
        bool tf = mask & (1<<i);
        if (i < m_checkBoxes.size())
            m_checkBoxes.at(i)->setChecked(tf);
    }
    unsigned newMask = ui->osg3dView->getCamera()->getCullMask();
    QString s = QString("%1").arg(newMask,8, 16,QChar('0'));
    ui->cameraMaskLineEdit->setText(s);
    ui->osg3dView->update();
}


void OsgForm::tweakCameraMaskBit(int state)
{
    QCheckBox * cb = dynamic_cast<QCheckBox *>(sender());
    if (!cb) return;

    int idx = m_checkBoxes.indexOf(cb);
    if (idx < 0) return;
    osg::Node::NodeMask mask = ui->osg3dView->getCamera()->getCullMask();
    unsigned bit = 1 << idx;
    if (state == Qt::Checked) {
        mask |= bit;
    } else {
        mask &= ~bit;
    }
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
                setNodeMask(n, mask);
            }
        }
    }
    ui->osgPropertyTable->displayObject(obj);
}

void OsgForm::setCameraMaskFromLineEdit()
{
    QLineEdit *le = dynamic_cast<QLineEdit *>(sender());
    if (!le) return;

    QString maskString = le->text();
    bool ok = false;
    unsigned newMask = maskString.toUInt(&ok, 16);
    if (ok) {
        setCameraMask(newMask);
    }
}

void OsgForm::handlePick(QVector<osg::ref_ptr<osg::Node> > nodePath)
{
    QTreeWidgetItem *root = ui->osgTreeWidget->invisibleRootItem();

    for (int i=2 ; i < nodePath.size() ; i++) {
        root = ui->osgTreeWidget->childThatMatches(root, nodePath.at(i));
        if (!root)
            return;
    }

    ui->osgTreeWidget->scrollToItem(root);
    ui->osgTreeWidget->setCurrentItem(root);
}


void OsgForm::setNodeMask(osg::ref_ptr<osg::Node> n, unsigned mask)
{
    n->setNodeMask(mask);
    ui->osg3dView->update();
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

    m_loadedModel->addChild(n);
    if (m_loadedModel->getNumChildren() == 1) {
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
    if (m_loadedModel->getNumChildren() == 1)
        n = m_loadedModel->getChild(0);
    else
        n = m_loadedModel;

    QFuture< bool > future =
            QtConcurrent::run(this, &OsgForm::saveThread, n, fileName);
    m_saveWatcher.setFuture(future);

    setProgressBarState(true);
    return true;
}

