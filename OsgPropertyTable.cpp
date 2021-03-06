#include "OsgPropertyTable.h"

#include <QString>
#include <QHeaderView>

#include <osg/Geometry>
#include <osg/Drawable>
#include <osg/Group>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Material>

#include "VariantPtr.h"
static const bool thisDebug = false;
#define ptDebug if (thisDebug) qDebug

OsgPropertyTable::OsgPropertyTable(QWidget *parent) :
    QTableWidget(parent)
{
    resizeColumnsToContents();
    horizontalHeader()->setStretchLastSection(true);

    connect(this, SIGNAL(itemClicked(QTableWidgetItem*)),
            this, SLOT(itemWasClicked(QTableWidgetItem*)));

    setColumnCount(2);

    hideAllRows();
}


void OsgPropertyTable::displayObject(osg::ref_ptr<osg::Object> object)
{
    ptDebug("displayObject(%s)", object->getName().c_str());

    hideAllRows();

    if (object.get()) {
        setTableValuesForObject(object);
        setTableValuesForNode(dynamic_cast<osg::Node *>(object.get()));
        setTableValuesForDrawable(dynamic_cast<osg::Drawable *>(object.get()));
        setTableValuesForStateSet(dynamic_cast<osg::StateSet *>(object.get()));
        setTableValuesForMaterial(dynamic_cast<osg::Material *>(object.get()));
    }

    resizeColumnsToContents();
    horizontalHeader()->setStretchLastSection(true);
}

void OsgPropertyTable::hideAllRows()
{
    for (int i=0 ; i < rowCount() ; i++)
        hideRow(i);
}

void OsgPropertyTable::itemWasClicked(QTableWidgetItem *item)
{
    if (!item) return;
    if ( ! (item->flags() & Qt::ItemIsUserCheckable) )
        return;

    QVariant v = item->data(Qt::UserRole);

    ptDebug("clicked %d %d %s", item->row(), item->column(), qPrintable(item->text()));
    if (v.isValid() && !v.isNull()) {
        osg::Object *obj = VariantPtr<osg::Object>::asPtr(v);

        if (osg::Node *n = dynamic_cast<osg::Node *>(obj)) {
            ptDebug("Node");
        }
        if (osg::Drawable *d = dynamic_cast<osg::Drawable *>(obj)) {
            ptDebug("Drawable");
        }
    }
}

QTableWidgetItem * OsgPropertyTable::getOrCreateWidgetItem(QTableWidget *tw, int row, int col)
{
    QTableWidgetItem *twi = tw->item(row, col);
    if (!twi) {
        twi = new QTableWidgetItem;
        twi->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
        tw->setItem(row, col, twi);
    }
    return twi;
}
void OsgPropertyTable::setTextForKey(const QString key, const QString value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText(value);

    twi->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

    showRow(twi->row());
}
QTableWidgetItem * OsgPropertyTable::setKeyChecked(const QString key, const bool value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText("");

    if (value)
        twi->setCheckState(Qt::Checked);
    else
        twi->setCheckState(Qt::Unchecked);

    twi->setFlags(Qt::ItemIsEnabled);
    showRow(twi->row());

    return twi;
}
QTableWidgetItem * OsgPropertyTable::itemForKey(const QString key)
{
    QList<QTableWidgetItem *> twilist = findItems(key, Qt::MatchFixedString);
    QTableWidgetItem *twi;

    if (twilist.size() == 0) {
        int currentRowCount = rowCount();
        setRowCount(currentRowCount+1);
        twi = getOrCreateWidgetItem(this, currentRowCount, 0);
        twi->setText(key);
        twi->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    } else
        twi = twilist.at(0);

     return getOrCreateWidgetItem(this, twi->row(), twi->column()+1);
}

void OsgPropertyTable::setTableValuesForObject(osg::ref_ptr<osg::Object> object)
{
    if (!object.valid()) return;
    setTextForKey("Name", QString::fromStdString( object->getName() ));

    osg::Object::DataVariance dataVariance = object->getDataVariance();
    QString key = "DataVariance";
    switch (dataVariance) {
    case osg::Object::DYNAMIC:
        setTextForKey(key, "DYNAMIC");
        break;
    case osg::Object::STATIC:
        setTextForKey(key, "STATIC");
        break;
    case osg::Object::UNSPECIFIED:
        setTextForKey(key, "UNSPECIFIED");
        break;
    }
}
void OsgPropertyTable::setTableValuesForNode(osg::Node *node)
{
    if (!node)
        return;

    setTextForKey("NodeMask", QString("").setNum((unsigned)node->getNodeMask(), 16));

    setKeyChecked("CullingActive", node->isCullingActive());

    setTextForKey("Descriptions", QString("%1").arg(node->getNumDescriptions()));

    setTableValuesForGroup(node->asGroup());
    setTableValuesForGeode(node->asGeode());
}
#include <osg/MatrixTransform>
void OsgPropertyTable::setTableValuesForTransform(osg::MatrixTransform *xform)
{
    if (!xform) return;

    osg::Matrix m = xform->getMatrix();

    osg::Vec3d d = m.getTrans();
    setTextForKey("Xlate", QString("%1 %2 %3").arg(d.x()).arg(d.y()).arg(d.z()));

    d = m.getScale();
    setTextForKey("Scale", QString("%1 %2 %3").arg(d.x()).arg(d.y()).arg(d.z()));
}
void OsgPropertyTable::setTableValuesForGroup(osg::Group *group)
{
    if(!group) return;
    setTextForKey("NumChildren", QString("%1").arg(group->getNumChildren()));

    setTableValuesForTransform(dynamic_cast<osg::MatrixTransform*>(group));

    osg::StateSet * ss = group->getStateSet();
    setTableValuesForStateSet(ss);

}
void OsgPropertyTable::setTableValuesForGeode(osg::Geode *geode)
{
    if(!geode) return;

    setTextForKey("NumDrawables", QString("%1").arg(geode->getNumDrawables()));

    osg::BoundingSpheref bs = geode->computeBound();
    setTextForKey("Bounds", QString("radius:%1 @(%2 %3 %4)")
                  .arg(bs.radius())
                  .arg(bs.center().x())
                  .arg(bs.center().y())
                  .arg(bs.center().z()));

    osg::StateSet * ss = geode->getStateSet();
    setTableValuesForStateSet(ss);

}
void OsgPropertyTable::setTableValuesForDrawable(osg::Drawable *drawable)
{
    if (!drawable) return;

    setKeyChecked("UseVertexBuffer", drawable->getUseVertexBufferObjects());
    setKeyChecked("UseDisplayList", drawable->getUseDisplayList());

    setTableValuesForGeometry(drawable->asGeometry());
}
#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/Material>
void OsgPropertyTable::setTableValuesForGeometry(osg::Geometry *geometry)
{
    if (!geometry) return;

    osg::Array *array = geometry->getVertexArray();
    setTextForKey("PrimitiveSets",
                  QString("%1").arg(geometry->getNumPrimitiveSets()));

    if (array)
        setTextForKey("VertexCount", QString("%1").arg(array->getNumElements()));


    array = geometry->getNormalArray();
    if (array)
        setTextForKey("NormalCount", QString("%1").arg( array->getNumElements()));


    array = geometry->getColorArray();
    if (array)
        setTextForKey("ColorCount", QString("%1").arg(array->getNumElements()));

    setTextForKey("TextCoordArrayCount",
                  QString("%1").arg(geometry->getNumTexCoordArrays()));

    osg::StateSet * ss = geometry->getStateSet();
    setTableValuesForStateSet(ss);
}
static QString formatVec4(const osg::Vec4 colorQuad)
{
    return QString("%1 %2 %3 %4").arg(colorQuad[0]).arg(colorQuad[1]).arg(colorQuad[2]).arg(colorQuad[3]);
}

void OsgPropertyTable::setTableValuesForStateSet(osg::StateSet *ss)
{
    if (!ss) return;
    osg::StateAttribute *sa = ss->getAttribute(osg::StateAttribute::MATERIAL);
    if (!sa) return;

    osg::Material *m = dynamic_cast<osg::Material *>(sa);
    setTableValuesForMaterial(m);
}

void OsgPropertyTable::setTableValuesForMaterial(osg::Material *m)
{
    if (!m) return;
    const osg::Vec4 diffuse = m->getDiffuse(osg::Material::FRONT);
    setTextForKey("DiffuseColor", formatVec4(diffuse));

    const osg::Vec4 specular = m->getSpecular(osg::Material::FRONT);
    setTextForKey("SpecularColor", formatVec4(specular));

    float shine = m->getShininess(osg::Material::FRONT);
    setTextForKey("shininess", QString("%2").arg(shine));

}
