#include "OriginAxis.h"
#include <osg/Matrix>
#include <osg/BoundingSphere>

OriginAxis::OriginAxis()
{
    m_axisGeom = new osg::Geometry();

    // allocate verticies
    m_axisVerts = new osg::Vec3Array;
    m_axisVerts->push_back(osg::Vec3(-1.0, 0.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 0.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(1.0, 0.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, -1.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 0.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 1.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 0.0, -1.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 0.0, 0.0));
    m_axisVerts->push_back(osg::Vec3(0.0, 0.0, 1.0));

    m_axisGeom->setVertexArray(m_axisVerts);
    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));
    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 1, 2));

    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 3, 2));
    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 4, 2));

    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 6, 2));
    m_axisGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 7, 2));

    // allocate colors
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(.25f,0.0f,0.0f,1.0f));
    colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    colors->push_back(osg::Vec4(0.0f,.25f,0.0f,1.0f));
    colors->push_back(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
    colors->push_back(osg::Vec4(0.0f,0.0f,.25f,1.0f));
    colors->push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));

    m_axisGeom->setColorArray(colors);
    m_axisGeom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
    m_axisGeom->setDataVariance(osg::Object::DYNAMIC);

    // generate the Geode
    m_axisGeode = new osg::Geode();
    m_axisGeode->addDrawable(m_axisGeom);
    m_axisGeode->setNodeMask( ~0 );
    m_axisGeode->setName("Axis");

    //turn off lighting so we always see the line color
    osg::StateSet *ss = m_axisGeode->getOrCreateStateSet();
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );


    m_transform = new osg::MatrixTransform();
    osg::Matrixd m;
    m.makeIdentity();
    m_transform->setMatrix(m);

    m_transform->addChild(m_axisGeode);
    osg::BoundingSphere bs(osg::Vec3d(0.0, 0.0, 0.0), 1.0);
    setScale(bs);
}

void OriginAxis::setScale(const osg::BoundingSphere &sph)
{


    osg::Vec3d center = sph.center();
    double radius = sph.radius();
    double x, y, z;
    double a, b;

    a = fabs(center.x()+radius);
    b = fabs(center.x()-radius);
    if (a > b) x = a;
    else x = b;

    a = fabs(center.y()+radius);
    b = fabs(center.y()-radius);
    if (a > b) y = a;
    else y = b;

    a = fabs(center.z()+radius);
    b = fabs(center.z()-radius);
    if (a > b) z = a;
    else z = b;

    osg::Matrixd m;
    m.makeScale(x, y, z);

    m_transform->setMatrix(m);
}
