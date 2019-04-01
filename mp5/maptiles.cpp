/**
 * @file maptiles.cpp
 * Code for the maptiles function.
 */

#include <iostream>
#include <map>
#include "maptiles.h"
//#include "cs225/RGB_HSL.h"

using namespace std;


Point<3> convertToXYZ(LUVAPixel pixel) {
    return Point<3>( pixel.l, pixel.u, pixel.v );
}

MosaicCanvas* mapTiles(SourceImage const& theSource,
                       vector<TileImage>& theTiles)
{
    if(theSource.getRows()==0 || theSource.getColumns()==0 || theSource.getRows()==-1 || theSource.getColumns()==-1 || theTiles.empty())
       return NULL;

    MosaicCanvas* canvas = new MosaicCanvas(theSource.getRows(), theSource.getColumns());
    vector<Point<3>> v;
    map<Point<3>, int> map;
    int i=0;
    for(vector<TileImage>::iterator it = theTiles.begin(); it!=theTiles.end(); ++it){
      v.push_back(convertToXYZ(it->getAverageColor()));
      map.emplace(convertToXYZ(it->getAverageColor()), i);
      i++;
    }

    KDTree<3> tree(v);

    for(int r=0; r<theSource.getRows(); r++){
      for(int c=0; c<theSource.getColumns(); c++){
         Point <3> neighbor = tree.findNearestNeighbor(convertToXYZ(theSource.getRegionColor(r, c)));

         canvas->setTile(r, c, &theTiles[map[neighbor]]);
      }
    }

    return canvas;
}
