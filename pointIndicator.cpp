#include "pointIndicator.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PrimitiveSet>

#include <osg/Matrixd>
#include <osg/Matrix>
#include <osgUtil/SmoothingVisitor>

osg::ref_ptr<osg::Geode> makeGeode(osg::Matrixd m,
                                   osg::Vec4f color,
                                   const char *name)
{
    osg::Vec4Array *colors = new osg::Vec4Array;
    colors->push_back(color);

    osg::Vec3Array *verts = new osg::Vec3Array;
    verts->push_back( m * osg::Vec3f( 0.0, -0.25, 1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f( 0.25,-0.0,  1.0));
    verts->push_back( m * osg::Vec3f(-0.25,-0.0,  1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.25, 1.0));
    verts->push_back( m * osg::Vec3f( 0.0,  0.25, 1.0));
    verts->push_back( m * osg::Vec3f( 0.25,-0.0,  1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f(-0.25,-0.0,  1.0));
    verts->push_back( m * osg::Vec3f( 0.0,  0.25, 1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.25,-1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f(-0.25, 0.0, -1.0));
    verts->push_back( m * osg::Vec3f( 0.25, 0.0, -1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.25,-1.0));
    verts->push_back( m * osg::Vec3f( 0.0,  0.25,-1.0));
    verts->push_back( m * osg::Vec3f(-0.25, 0.0, -1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));
    verts->push_back( m * osg::Vec3f( 0.25, 0.0, -1.0));
    verts->push_back( m * osg::Vec3f( 0.0,  0.25,-1.0));
    verts->push_back( m * osg::Vec3f( 0.0, -0.0,  0.0));

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setName(name);
    geom->setVertexArray(verts);
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 24));

    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osgUtil::SmoothingVisitor::smooth(*geom, 0.0);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setName(name);
    geode->addDrawable(geom);

    return geode;
}


PointIndicator::PointIndicator(QObject *parent)
    : QObject(parent)
    , m_xform(new osg::MatrixTransform)
{
    m_xform->addChild( makeGeode(
                           osg::Matrix::identity(),
                           osg::Vec4f(0.0, 0.3, 1.0, 1.0),
                           "Zaxis"
                           ));

    m_xform->addChild( makeGeode(
                           osg::Matrix::rotate(osg::Vec3f(0.0, 0.0, 1.0),
                                               osg::Vec3f(1.0, 0.0, 0.0)),
                           osg::Vec4f(1.0, 0.0, 0.0, 1.0),
                           "Xaxis"
                           ));
    m_xform->addChild( makeGeode(
                           osg::Matrix::rotate(osg::Vec3f(0.0, 0.0, 1.0),
                                               osg::Vec3f(0.0, 1.0, 0.0)),
                           osg::Vec4f(0.0, 1.0, 0.0, 1.0),
                           "Yaxis"
                           ));

    m_xform->setNodeMask(0);
}

osg::Vec3 PointIndicator::getPosition() const
{
    osg::Matrix m = m_xform->getMatrix();
    return m.getTrans();
}

void PointIndicator::setPosition(osg::Vec3d pt)
{
    setPosition(pt.x(), pt.y(), pt.z());
}

void PointIndicator::setPosition(float x, float y, float z)
{
    osg::Matrix m = m_xform->getMatrix();

    m.makeTranslate(x, y, z);
    m_xform->setMatrix(m);

    osg::Vec3 pt = m.getTrans();
    emit positionChanged(pt.x(), pt.y(), pt.z());
}

void PointIndicator::setScale(float s)
{
    if (s == 0.0)
        return;

    osg::Matrix m = m_xform->getMatrix();
    m.makeScale(s, s, s);
    m.scale(s, s, s);
    m_xform->setMatrix(m);
}

void PointIndicator::setRelativeScale(float s)
{
    if (s == 0.0)
        return;
    osg::Matrix m = m_xform->getMatrix();
    osg::Vec3d sv = m.getScale() * s;

    m.scale(sv);
    m_xform->setMatrix(m);
}

