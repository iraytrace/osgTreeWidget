#include "OsgTreeWidget.h"
#include <QHeaderView>

#include <osg/Group>
#include <osg/Geode>

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


void OsgTreeWidget::addObject(osg::Object *object, QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem;


    if (object->getName().size() == 0) {
        QString name;
        if (parentItem) {
            name = QString("%1_%2").arg(parentItem->text(0)).arg(object->className());
        } else {
            name = object->className();
        }
        object->setName(qPrintable(name));
    }

    twDebug("adding <%s>", object->getName().c_str());
    newItem->setText(0, QString::fromStdString(object->getName()));
    newItem->setText(1, object->className());
    newItem->setData(0, Qt::UserRole, VariantPtr<osg::Object>::asQVariant(object));
    newItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);

    if (osg::Node *n = dynamic_cast<osg::Node *>(object)) {
        newItem->setText(2, QString("").setNum(n->getNodeMask(), 16));

        if (osg::Group *group = n->asGroup()) {
            for (unsigned i = 0 ; i < group->getNumChildren() ; i++) {
                addObject(group->getChild(i), newItem);
            }
        }
        if (osg::Geode *geode = n->asGeode()) {
            for (unsigned i = 0 ; i < geode->getNumDrawables() ; i++) {
                osg::Object *drawableObject = geode->getDrawable(i);
                addObject(drawableObject, newItem);
            }
        }
    }

    if (!parentItem) {
        this->addTopLevelItem(newItem);
    } else {
        parentItem->addChild(newItem);
    }
    resizeAllColumns();
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
    QVariant v = current->data(0, Qt::UserRole);
    osg::ref_ptr<osg::Object> object = VariantPtr<osg::Object>::asPtr(v);

    twDebug("CurrentObject %s", object->getName().c_str());
    emit currentObject(object);
}

