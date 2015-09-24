#ifndef POINTINDICATOR_H
#define POINTINDICATOR_H

#include <QObject>
#include <osg/MatrixTransform>
class PointIndicator : public QObject
{
    Q_OBJECT
public:
    PointIndicator(QObject *parent = 0);
    osg::ref_ptr<osg::MatrixTransform> getNode() const { return m_xform; }

    osg::Vec3 getPosition() const;
signals:
    void positionChanged(float x, float y, float z);
public slots:
    void setPosition(osg::Vec3d pt);
    void setPosition(float x, float y, float z);
    void setScale(float s);
    void setRelativeScale(float s);
private:
    osg::ref_ptr<osg::MatrixTransform> m_xform;
};

#endif // POINTINDICATOR_H
