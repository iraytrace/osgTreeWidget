#include "OsgTreeWidget.h"
#include <QHeaderView>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/MatrixTransform>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMessageBox>

#include <string>

static const bool debugTreeWidget = false;
#define twDebug if (debugTreeWidget) qDebug

#include "VariantPtr.h"

OsgTreeWidget::OsgTreeWidget(QWidget *parent) :QTreeWidget(parent)
{
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            this, SLOT(resizeAllColumns()));
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
            this, SLOT(resizeAllColumns()));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(resizeAllColumns()));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(currentItemWasChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    this->buildPopupMenu();
    resizeAllColumns();

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
#if 0  // other interesting slots to consider
    void	currentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
    void	itemActivated(QTreeWidgetItem * item, int column)
    void	itemChanged(QTreeWidgetItem * item, int column)
    void	itemClicked(QTreeWidgetItem * item, int column)
    void	itemCollapsed(QTreeWidgetItem * item)
    void	itemDoubleClicked(QTreeWidgetItem * item, int column)

    void	itemExpanded(QTreeWidgetItem * item)
    void	itemPressed(QTreeWidgetItem * item, int column)
    void	itemSelectionChanged()
#endif
}

QTreeWidgetItem * OsgTreeWidget::createItemForObject(osg::Object *object,
                                                     int parentNumberInChild)
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem;

    twDebug("adding <%s>", object->getName().c_str());
    newItem->setText(0, QString::fromStdString(object->getName()));
    newItem->setText(1, object->className());
    newItem->setData(0, Qt::UserRole, VariantPtr<osg::Object>::asQVariant(object));
    newItem->setData(1, Qt::UserRole, parentNumberInChild);
    newItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);

    if (osg::Node *n = dynamic_cast<osg::Node*>(object)) {
        newItem->setText(2, QString("").setNum(n->getNodeMask(), 16));
    }
    return newItem;
}
void OsgTreeWidget::ensureObjectHasName(osg::Object *object,
                                                  std::string parentName)
{
    if (object->getName().size() != 0)
        return;

    QString name;
    if (parentName.size() > 0)
        name = QString("%1_%2").arg(parentName.c_str()).arg(object->className());
     else
        name = object->className();

    object->setName(qPrintable(name));

}

unsigned OsgTreeWidget::getIndexOfParentInChild(osg::Object *object,
                                           osg::Object *parentObject)
{
    if (!parentObject)
        return (unsigned)-1;

    if (osg::Node *n = dynamic_cast<osg::Node*>(object)) {
        osg::Group *parentGroup = dynamic_cast<osg::Group*>(parentObject);
        for (unsigned i=0 ; i < n->getParents().size() ; i++) {
            if (n->getParent(i) == parentGroup)
                return i;
        }
        return (unsigned)-1;
    }

    // while the Drawable API uses the same names as the Node API they
    // do not share a common base class, so we must duplicate code to
    // get the right vtable entries
    if (osg::Drawable *d = dynamic_cast<osg::Drawable*>(object)) {
        osg::Node *parentNode = dynamic_cast<osg::Node*>(parentObject);
        for (unsigned i=0 ; i < d->getParents().size() ; i++) {
            if (d->getParent(i) == parentNode)
                return i;
        }
        return (unsigned)-1;
    }

    if (osg::StateSet *ss = dynamic_cast<osg::StateSet*>(object)) {
        for (unsigned i=0 ; i < ss->getParents().size() ; i++) {
            if (ss->getParent(i) == parentObject)
                return i;
        }
        return (unsigned)-1;
    }

    return (unsigned)-1;
}
#if 0
int OsgTreeWidget::getIndexOfChildInParent(osg::Object *object,
                                           osg::Object *parentObject)
{
    if (osg::Group *g = dynamic_cast<osg::Group*>(parentObject))
        if (osg::Node *n = dynamic_cast<osg::Node*>(object))
            return g->getChildIndex(n);

    if (osg::Geode *geode = dynamic_cast<osg::Geode*>(parentObject))
            if (osg::Drawable *drawable = dynamic_cast<osg::Drawable*>(object))
                geode->getDrawableIndex(drawable);

    return -1;
}
#endif
bool OsgTreeWidget::addChildrenOfGroup(osg::Object *object,
                                       QTreeWidgetItem *groupItem)
{
    osg::Group *group = dynamic_cast<osg::Group*>(object);
    if (!group) return false;

    for (unsigned i = 0 ; i < group->getNumChildren() ; i++) {
        QTreeWidgetItem *childItem = addObjectItem(group->getChild(i),object);
        groupItem->addChild(childItem);
    }
    return true;
}

bool OsgTreeWidget::addChildrenOfGeode(osg::Object *object,
                                       QTreeWidgetItem *geodeItem)
{
    osg::Geode *geode = dynamic_cast<osg::Geode*>(object);
    if (!geode) return false;

    for (unsigned i = 0 ; i < geode->getNumDrawables() ; i++ ) {

        QTreeWidgetItem *childItem = addObjectItem(geode->getDrawable(i), object);
        geodeItem->addChild(childItem);
    }
    return true;
}


QTreeWidgetItem * OsgTreeWidget::addObjectItem(osg::Object *object,
                                               osg::Object *parentObject) // may be null
{
    if (parentObject)
        ensureObjectHasName(object, parentObject->getName());

    QTreeWidgetItem *newItem =
            createItemForObject(object,
                                getIndexOfParentInChild(object, parentObject));

    if (addChildrenOfGroup(object, newItem)) return newItem;
    if (addChildrenOfGeode(object, newItem)) return newItem;
#if 0
    if (osg::StateSet *ss = dynamic_cast<osg::StateSet *>(object)) {
        osg::StateAttribute *sa = ss->getAttribute(osg::StateAttribute::MATERIAL);
        if (!sa) return newItem;
        osg::Material *m = dynamic_cast<osg::Material *>(sa);
        if (!m) return newItem;
        newItem->addChild(addObjectItem(m, newItem->text(0)));
    }

    if (osg::Geometry *geom = dynamic_cast<osg::Geometry *>(object)) {
        if (osg::StateSet *ss = geom->getStateSet()) {
            newItem->addChild(addObjectItem(ss, newItem->text(0)));
        }
    }

    if (osg::Node *n = dynamic_cast<osg::Node *>(object)) {
        newItem->setText(2, QString("").setNum(n->getNodeMask(), 16));

        if (osg::StateSet *ss = n->getStateSet()) {
            newItem->addChild(addObjectItem(ss, newItem->text(0)));
        }


        if (osg::Group *group = n->asGroup()) {
            for (unsigned i = 0 ; i < group->getNumChildren() ; i++) {
                newItem->addChild(addObjectItem(group->getChild(i), newItem->text(0)));
            }
        }
        if (osg::Geode *geode = n->asGeode()) {
            for (unsigned i = 0 ; i < geode->getNumDrawables() ; i++) {
                osg::Object *drawableObject = geode->getDrawable(i);
                newItem->addChild(addObjectItem(drawableObject, newItem->text(0)));
            }
        }
    }
#endif
    return newItem;
}

void OsgTreeWidget::addObjectFinished()
{
    QTreeWidgetItem *newItem = m_watcher.result();
    this->addTopLevelItem(newItem);
    resizeAllColumns();
    this->setCursor(m_stashedCursor);
}

void OsgTreeWidget::insertMatrixAboveCurrentItem()
{
    QTreeWidgetItem *currentItem = this->currentItem();
    if (!currentItem) return;

    QVariant v = currentItem->data(0, Qt::UserRole);
    if (v.isNull() || ! v.isValid()) return;

    osg::Object *obj = VariantPtr<osg::Object>::asPtr(v);

    if (osg::Node *n = dynamic_cast<osg::Node *>(obj)) {

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        QString name = QString("%1_xform").arg(obj->getName().c_str());
        mt->setName(name.toStdString());
        bool ok = false;
        unsigned parentNumber = currentItem->data(1, Qt::UserRole).toUInt(&ok);
        if (!ok || parentNumber == (unsigned)-1) {
            return;
        }

        // substitute MatrixTransform for node in OSG parent
        osg::Group * currentParent = n->getParent(parentNumber);
        unsigned positionInParent = currentParent->getChildIndex(n);
        currentParent->insertChild(positionInParent, mt);
        currentParent->removeChild(n);

        // substitute matrixItem for current item in TreeWidgetItem parent
        QTreeWidgetItem *matrixItem = createItemForObject(mt, 0);
        QTreeWidgetItem *parentItem = currentItem->parent();
        int idx = parentItem->indexOfChild(currentItem);

        currentItem = parentItem->takeChild(idx);
        matrixItem->addChild(currentItem);

        parentItem->insertChild(idx, matrixItem);
    }
}

void OsgTreeWidget::buildPopupMenu()
{


    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));

    m_popupMenu.addAction("InsertMatrix", this, SLOT(insertMatrixAboveCurrentItem()), QKeySequence("i"));

#if 0 // example code
    QMenu *sub = m_popupMenu.addMenu("MouseMode...");
    QAction *a;
    sub = m_popupMenu.addMenu("Std View...");
    a = sub->addAction("Top", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_T));
    a->setData(V_TOP);
    a = sub->addAction("Bottom", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_U));
    a->setData(V_BOTTOM);
    a = sub->addAction("Front", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_F));
    a->setData(V_FRONT);
    a = sub->addAction("Back", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_B));
    a->setData(V_BACK);
    a = sub->addAction("Right", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_R));
    a->setData(V_RIGHT);
    a = sub->addAction("Left", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_L));
    a->setData(V_LEFT);
#endif
}

void OsgTreeWidget::addObject(osg::Object *object)
{
    if (m_watcher.isRunning()) {
        QMessageBox::warning(this, "Loading in progress", "Please wait until the previous load has completed");
        return;
    }
    connect(&m_watcher, SIGNAL(finished()),
            this, SLOT(addObjectFinished()));

    // Start the computation.
    QFuture< QTreeWidgetItem * > future =
            QtConcurrent::run(this, &OsgTreeWidget::addObjectItem, object,
                              (osg::Object *)0);
    m_watcher.setFuture(future);

    m_stashedCursor = this->cursor();
    this->setCursor(Qt::WaitCursor);
}

bool OsgTreeWidget::itemMatchesObject(QTreeWidgetItem *i, osg::ref_ptr<osg::Object> obj)
{
    if (!i ||!obj.valid()) return false;

    QVariant v = i->data(0, Qt::UserRole);
    if (v.isNull() || !v.isValid()) return false;

    osg::Object *object = VariantPtr<osg::Object>::asPtr(v);
    if (!object) return false;

    return (object == obj.get());
}

void OsgTreeWidget::lookForMatch(std::vector<QTreeWidgetItem *> &matchingItems,
                                 osg::ref_ptr<osg::Object> obj,
                                 QTreeWidgetItem *item)
{
    if (item) {
        if (itemMatchesObject(item, obj)) {
            matchingItems.push_back(item);
            return;
        }
    } else {
        item = invisibleRootItem();
        matchingItems.clear();
    }

    for (int i=0 ; i < item->childCount() ; i++) {
        lookForMatch(matchingItems, obj, item->child(i));
    }
}

QTreeWidgetItem *OsgTreeWidget::childThatMatches(QTreeWidgetItem *top,
                                                 osg::ref_ptr<osg::Object> obj)
{
    for (int i=0 ; i < top->childCount() ; i++) {
        if (itemMatchesObject(top->child(i), obj))
            return top->child(i);
    }
    return (QTreeWidgetItem *)0;
}

void OsgTreeWidget::resizeAllColumns()
{
    twDebug("resizeColumns %d", columnCount());
    for (int i=0 ; i < this->columnCount() ; i++) {
        resizeColumnToContents(i);
    }
    header()->setStretchLastSection(true);
}

void OsgTreeWidget::customMenuRequested(const QPoint &pos)
{
    // Should disable/enable menu entries
    // as appropriate for current selected item.
    // For instance, cannot parent a Geometry to a MatrixTransform
    // Geometry nodes when a Geometry node is current
    m_popupMenu.popup(this->mapToGlobal(pos));
}

void OsgTreeWidget::currentItemWasChanged(QTreeWidgetItem *current,
                                          QTreeWidgetItem *)
{
    if (!current)
        return;
    QVariant v = current->data(0, Qt::UserRole);
    osg::ref_ptr<osg::Object> object = VariantPtr<osg::Object>::asPtr(v);

    twDebug("CurrentObject %s", object->getName().c_str());
    emit currentObject(object);
}

