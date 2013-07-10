#ifndef SHAPEFILE_H
#define SHAPEFILE_H

#include <ogrsf_frmts.h>
#include <string>

class ShapeFile
{
  private:
    OGRDataSource* _poDS;
    OGRLayer* _layer;
    OGRSpatialReference* _coordinateSystem;
  public:
    ShapeFile (const std::string);
    ~ShapeFile ();
    const int getLayerCount () const;
    const int getFeatureCount () const;
    void resetFeatures ();
    OGRFeature* getNextFeature ();
    const double getAreaRatio (OGRGeometry*, OGRSpatialReference*);
    void setSpatialFilter (OGRGeometry*);
};

class ShapeFileOpenFileException {};
class ShapeFileGetLayerException {};
class ShapeFileGetSpatialRefException {};

#endif
