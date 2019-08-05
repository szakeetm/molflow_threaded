//
// Created by pascal on 8/2/19.
//

#ifndef MOLFLOW_SUPERFACET_H
#define MOLFLOW_SUPERFACET_H


#include <vector>
#include "Vector.h"
#include "Buffer_shared.h" //DirectionCell

class CellProperties {
public:
    //Old C-style array to save memory
    Vector2d* points;
    size_t nbPoints;
    double   area;     // Area of element
    float   uCenter;  // Center coordinates
    float   vCenter;  // Center coordinates
    //int     elemId;   // Element index (MESH array)
    //int full;
};

class SuperFacet {

public:
    void  FillVertexArray(InterfaceVertex *v);

    size_t GetNbCell();

    size_t GetNbCellForRatio(double ratio);

    void  InitVisibleEdge();

    size_t   GetIndex(int idx);

    size_t   GetIndex(size_t idx);

    double GetMeshArea(size_t index, bool correct2sides = false);

    size_t GetMeshNbPoint(size_t index);

    Vector2d GetMeshPoint(size_t index, size_t pointId);

    Vector2d GetMeshCenter(size_t index);

    double GetArea();

//Different implementation within Molflow/Synrad
size_t GetGeometrySize();

    size_t GetHitsSize(size_t nbMoments);

    std::string GetAngleMap(size_t formatId);

//C-style arrays to save memory (textures can be huge):
int      *cellPropertiesIds;
    FacetProperties sh;
// GUI stuff
std::vector<bool>  visible;
    std::vector<size_t>   indices;
// -1 if full element, -2 if outside polygon, otherwise index in meshvector
CellProperties* meshvector;
    size_t meshvectorsize;
    double tRatio;

#ifdef MOLFLOW
    std::vector<size_t> angleMapCache;
#endif
};




#endif //MOLFLOW_SUPERFACET_H
