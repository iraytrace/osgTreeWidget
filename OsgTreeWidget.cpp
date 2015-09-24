#include "OsgTreeWidget.h"
#include <QHeaderView>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
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

    resizeAllColumns();

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
#if 0
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

QTreeWidgetItem * OsgTreeWidget::addObjectItem(osg::Object *object,
                                               QString parentName)

{
    QTreeWidgetItem *newItem = new QTreeWidgetItem;

    if (object->getName().size() == 0) {
        QString name;
        if (parentName.size() > 0)
            name = QString("%1_%2").arg(parentName).arg(object->className());
         else
            name = object->className();

        object->setName(qPrintable(name));
    }

    twDebug("adding <%s>", object->getName().c_str());
    newItem->setText(0, QString::fromStdString(object->getName()));
    newItem->setText(1, object->className());
    newItem->setData(0, Qt::UserRole, VariantPtr<osg::Object>::asQVariant(object));
    newItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
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

    return newItem;
}

void OsgTreeWidget::addObjectFinished()
{
    QTreeWidgetItem *newItem = m_watcher.result();
    this->addTopLevelItem(newItem);
    resizeAllColumns();
    this->setCursor(m_stashedCursor);
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
            QtConcurrent::run(this, &OsgTreeWidget::addObjectItem, object, QString());
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

