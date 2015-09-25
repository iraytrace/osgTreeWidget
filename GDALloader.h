#ifndef GDALLOADER_H
#define GDALLOADER_H

#include <QObject>
#include <vector>

#include <osg/Geode>

class GDALDataset;

class GDALloader : public QObject
{
    Q_OBJECT
public:
    explicit GDALloader(QObject *parent = 0);

    void loadTerrain(const QString filename);
    QString getInfo();
    int getBandCount();
    void getSizes(int &x, int &y, int bandNumber) const;
    void loadData(std::vector<float> &buffer, int bandNumber, int x, int y);
    osg::ref_ptr<osg::Geode> buildGeometry(int bandNumber);
signals:

public slots:

private: // data
    QString m_fileName;
    // Please keep in mind that GDALRasterBand objects are owned
    // by their dataset, and they should never be destroyed with
    // the C++ delete operator.
    GDALDataset  *poDataset;
    double        adfGeoTransform[6];
};

#endif // GDALLOADER_H
