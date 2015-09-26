#include "GDALloader.h"

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()

#include <QTextStream>

#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/SmoothingVisitor>

GDALloader::GDALloader(QObject *parent)
    : QObject(parent)
    , poDataset(nullptr)
{
    GDALAllRegister();
}



int GDALloader::getBandCount()
{
    if (poDataset == nullptr) return 0;

    return poDataset->GetRasterCount();
}

void GDALloader::getSizes(int &x, int &y, int bandNumber) const
{
    if (poDataset == nullptr) {
        x = y = 0;
        return;
    }
    GDALRasterBand  *poBand = poDataset->GetRasterBand( bandNumber );
    poBand->GetBlockSize( &x, &y );
}

void GDALloader::loadData(std::vector<float> &buffer, const int bandNumber, int x, int y)
{
    GDALRasterBand  *poBand = poDataset->GetRasterBand( bandNumber );
    int             nBlockXSize, nBlockYSize;

    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );

    buffer.resize(nBlockXSize * nBlockYSize);

    CPLErr status =
            poBand->RasterIO( GF_Read,
                              0, 0, // offset in X,Y
                              nBlockXSize, nBlockYSize, // size of tile in X,Y
                              buffer.data(), // where to put it
                              nBlockXSize, nBlockYSize, // size of pafScanLine in x,y
                              GDT_Float32, // size of 1 element in pafScanLine
                              0, 0 // stride values in X,Y for entries in pafScanLine
                              //, nullptr
                              ); // extra args about resampling, progress callback
}

osg::ref_ptr<osg::Geode> GDALloader::buildGeometry(int bandNumber)
{
    if (poDataset == nullptr) {
        osg::ref_ptr<osg::Geode> bogus = new osg::Geode;
        bogus->setName("dataset Not Loaded");
        return bogus;
    }

    // get dimensions
    int xDim = poDataset->GetRasterXSize();
    int yDim = poDataset->GetRasterYSize();

    std::vector<float> elevBuffer;
    elevBuffer.resize(xDim * yDim);

    GDALRasterBand  *poBand = poDataset->GetRasterBand( bandNumber );

    qDebug("xDim:%d yDim:%d", xDim, yDim);

    int blockX, blockY;
    poBand->GetBlockSize(&blockX, &blockY);
    qDebug("blockX:%d blockY:%d", blockX, blockY);

    qDebug("Xsize:%d Ysize:%d", poBand->GetXSize(), poBand->GetYSize());


    CPLErr status =
    poBand->RasterIO(GF_Read, 0, 0, xDim, yDim,
                     (void *)elevBuffer.data(), // where to put it
                     xDim, yDim,
                     GDT_Float32, // dataType name
                     0, 0);

    // build the vertex array
    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;
    verts->resize(xDim*yDim);
    for (int j=0 ; j < yDim ; j++)
        for (int i=0 ; i < xDim ; i++)
            (*verts)[i+j*xDim].set(
                    osg::Vec3f(i*adfGeoTransform[1],
                               j*adfGeoTransform[5],
                               elevBuffer[i+j*xDim]) );

    osg::Geometry *geom = new osg::Geometry;
    geom->setName( basename(qPrintable(m_fileName)));
    geom->setVertexArray(verts.get());

    // create primitive sets
    for (int y=0 ; y < yDim-1 ; y++) {
        osg::ref_ptr<osg::DrawElementsUInt> indicies =
                new osg::DrawElementsUInt(GL_TRIANGLE_STRIP, xDim*2);
        int currentIndex = 0;
        for (int x=0 ; x < xDim ; x++) {
            (*indicies)[currentIndex++] = x + y*xDim;
            (*indicies)[currentIndex++] = x + (y+1)*xDim;
        }
        geom->addPrimitiveSet( indicies );
    }

    // build the normal array (compute normals per-triangle)
    osgUtil::SmoothingVisitor::smooth(*geom,osg::PI_4);


    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setName(basename(qPrintable(m_fileName)));
    geode->addDrawable(geom);

    return geode;
}

void GDALloader::loadTerrain(const QString filename)
{
    if (poDataset != nullptr)
        GDALClose(poDataset);

    poDataset = (GDALDataset *) GDALOpen( qPrintable(filename), GA_ReadOnly );
    if( poDataset == NULL ) {
        m_fileName.clear();
    } else {
        m_fileName = filename;
    }
}

QString GDALloader::getInfo()
{
    QString infoParagraph;
    QTextStream stream(&infoParagraph);

    stream << "Driver: " <<
              poDataset->GetDriver()->GetDescription() << "/" <<
              poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) << endl;

    stream << "Size is RasterXSize:" <<
              poDataset->GetRasterXSize() << " x RasterYSize:" << poDataset->GetRasterYSize()
           << " x RasterCount:" <<
              poDataset->GetRasterCount() <<endl;
#if 0
    if( poDataset->GetProjectionRef()  != NULL ) {
        stream << "Projection is `" << endl;
        QString projRef = poDataset->GetProjectionRef();
        projRef = projRef.replace("[", "[\n");
        projRef = projRef.replace("]", "]\n");
        projRef = projRef.replace(",", ",\n");

        QStringList openBraces = projRef.split("\n");
        int indent = 0;
        foreach (QString s, openBraces) {
            for (int i=0 ; i < indent ; i++)
                stream << "    ";
            stream << s << endl;
            indent += s.count(QChar('['));
            indent -= s.count(QChar(']'));
        }
    }
#endif
    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
        stream << "Origin = (" <<
                  adfGeoTransform[0] << "," << adfGeoTransform[3] << ")" << endl;

        stream << "Pixel Size = (" <<
                  adfGeoTransform[1] << "," << adfGeoTransform[5] << ")" << endl;

    }

    for (int bandNumber=1 ; bandNumber <= poDataset->GetRasterCount() ; bandNumber++) {
        int             nBlockXSize, nBlockYSize;
        GDALRasterBand  *poBand = poDataset->GetRasterBand( bandNumber );


        stream << "Band " << bandNumber << endl;
        poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
        stream << "\tblockSize=" << nBlockXSize << "x" << nBlockYSize
               << " Type=" << GDALGetDataTypeName(poBand->GetRasterDataType())
               << " ColorInterp=" << GDALGetColorInterpretationName(
                      poBand->GetColorInterpretation()) << endl;

        int             bGotMin, bGotMax;
        double          adfMinMax[2];
        adfMinMax[0] = poBand->GetMinimum( &bGotMin );
        adfMinMax[1] = poBand->GetMaximum( &bGotMax );
        if( ! (bGotMin && bGotMax) )
            GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);

        stream << "\tMin=" << adfMinMax[0] << " Max=" << adfMinMax[1] << endl;
        stream << "\toverviews=" << poBand->GetOverviewCount() << endl;
        if( poBand->GetColorTable() != NULL )
            stream << "\tcolor table with " <<
                      poBand->GetColorTable()->GetColorEntryCount()
                   << " entries" << endl;

    }

    return infoParagraph;
}
