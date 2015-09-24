#ifndef ORIGINAXIS_H
#define ORIGINAXIS_H
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/ClipNode>
#include <osg/StateSet>
#include <osg/MatrixTransform>
#include <osg/BoundingSphere>
class OriginAxis
{
public:
    OriginAxis();
    osg::ref_ptr <osg::Node>           getNode() const { return m_transform; }
    void setScale(const osg::BoundingSphere &s);
private:
    osg::ref_ptr<osg::Vec3Array> m_axisVerts;
    osg::ref_ptr<osg::Geometry> m_axisGeom;
    osg::ref_ptr<osg::Geode> m_axisGeode;
    osg::ref_ptr<osg::MatrixTransform> m_transform;
};

#endif // ORIGINAXIS_H
