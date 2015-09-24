#ifndef GDAL_WRAPPER_H
#define GDAL_WRAPPER_H

#include <QObject>
class GDALDataset;

class GDAL_Wrapper : public QObject
{
    Q_OBJECT
public:
    GDAL_Wrapper(QObject *parent = 0);

    bool loadDTED(const QString fileName);
    void fetchRasterBand(GDALDataset *poDataset, int i);
    void fetchRasterData(GDALDataset *poDataset, int i);
    bool loadShape(const QString fileName);
    bool loadTerrain(const QString fileName);
signals:
    void showMessage(QString);

public slots:
private:
};

#endif // GDAL_WRAPPER_H
