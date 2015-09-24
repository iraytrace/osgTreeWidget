#include "GDAL_Wrapper.h"

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include "ogrsf_frmts.h"

#include <memory>

GDAL_Wrapper::GDAL_Wrapper(QObject *parent) : QObject(parent)
{

}
bool GDAL_Wrapper::loadTerrain(const QString fileName)
{
    GDALDataset *poDataset = (GDALDataset *)
            GDALOpen(fileName.toStdString().c_str(), GA_ReadOnly);
    if (!poDataset) {
        emit showMessage("GDALOpen() Failed");
        return false;
    }

    double adfGeoTransform[6];


    emit showMessage( QString::asprintf( "Driver: %s/%s\n",
            poDataset->GetDriver()->GetDescription(),
            poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) ));

    emit showMessage( QString::asprintf( "Size is %dx%dx%d\n",
            poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
            poDataset->GetRasterCount() ));

    if( poDataset->GetProjectionRef()  != NULL )
        emit showMessage( QString::asprintf( "Projection is `%s'\n",
                                              poDataset->GetProjectionRef() ));

    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
        emit showMessage( QString::asprintf( "Origin = (%.6f,%.6f)\n",
                adfGeoTransform[0], adfGeoTransform[3] ));

        emit showMessage( QString::asprintf( "Pixel Size = (%.6f,%.6f)\n",
                adfGeoTransform[1], adfGeoTransform[5] ));
    } else {
        emit showMessage( "We have a GeoTransform");
    }

    int maxRaster =  poDataset->GetRasterCount();
    for (int i=1 ; i <= maxRaster ; i++) {
        fetchRasterBand(poDataset, i);
    }

    GDALClose(poDataset);
    return true;
}
bool GDAL_Wrapper::loadShape(const QString fileName)
{
    GDALDataset       *poDS;
#if 0
    poDS = (GDALDataset*) GDALOpenEx( "point.shp", GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( poDS == NULL ) {
        emit showMessage( QString::asprintf("Open %s failed.", qPrintable(fileName)) );
        return false;
    }
#endif
    return true;
}

bool GDAL_Wrapper::loadDTED(const QString fileName)
{
    GDALAllRegister();
    if (fileName.endsWith("tiff") || fileName.endsWith("tif"))
        return loadTerrain(fileName);
    else if (fileName.endsWith(".shp"))
        return loadShape(fileName);
    return false;
}

void GDAL_Wrapper::fetchRasterBand(GDALDataset *poDataset, int i)
{

    int             nBlockXSize, nBlockYSize;
    int             bGotMin, bGotMax;
    double          adfMinMax[2];

    if (i < 1 || i > poDataset->GetRasterCount())
        return;

    QString message = QString("Band %1\n").arg(i);
    GDALRasterBand  *poBand = poDataset->GetRasterBand( i );
    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );

    message.append( QString::asprintf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
            nBlockXSize, nBlockYSize,
            GDALGetDataTypeName(poBand->GetRasterDataType()),
            GDALGetColorInterpretationName(
                poBand->GetColorInterpretation()) ));

    adfMinMax[0] = poBand->GetMinimum( &bGotMin );
    adfMinMax[1] = poBand->GetMaximum( &bGotMax );
    if( ! (bGotMin && bGotMax) )
        GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);

    message.append( QString::asprintf( "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] ));

    if( poBand->GetOverviewCount() > 0 )
        message.append( QString::asprintf( "Band has %d overviews.\n",
                                              poBand->GetOverviewCount() ));

    if( poBand->GetColorTable() != NULL )
        message.append( QString::asprintf( "Band has a color table with %d entries.\n",
                 poBand->GetColorTable()->GetColorEntryCount() ));

    emit showMessage(message);
}

void GDAL_Wrapper::fetchRasterData(GDALDataset *poDataset, int i)
{
    GDALRasterBand  *poBand = poDataset->GetRasterBand( i );
    int   nXSize = poBand->GetXSize();
    int   nYSize = poBand->GetYSize();

    float *pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize*nYSize);
    poBand->RasterIO( GF_Read, 0, 0, nXSize, nYSize,
                      pafScanline, nXSize, nYSize, GDT_Float32,
                      0, 0 );

    CPLFree(pafScanline);
}

