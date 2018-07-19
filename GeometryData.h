////////////////////////////////////////////////////////////////////////////////////////////////////
// Types of geometry representation
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <set>

////////////////////////////////////////////////////////////////////////////////////////////////////
static const int INVALID_ID = -1;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFace;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SVert
{
    SVert();
    SVert( const float _x, const float _y, const float _z );
    SVert& operator+( const SVert& rhs );
    SVert& operator*( float coef );
    
    SVert   GetNormalazed() const;
    void    Normalize();
    
    float       x;
    float       y;
    float       z;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SEdge
{
    SEdge();
    SEdge( const int _idA, const int _idB );
    
    void    RegisterFace( const int id );
    //bool    operator==( const SEdge& rhs ) const;
    
    int         idA;        // Original point
    int         idB;        // Original point
    int         idC;        // Middle point
    int         faceID[2];  // Face
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFace
{
    SFace( const int _regionID, const int idA, const int idB, const int idC );
    
    const int   regionID;
    int         pointID[3];
    int         edgeID[3];
    
    float       angleLat;
    float       angleLon;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SIcosahedron
{
    SIcosahedron();
    std::vector< SVert >    vert;
    std::vector< SEdge >    edge;
    std::vector< SFace >    face;
    int                     level;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool            operator<( const SEdge& lhs, const SEdge& rhs );
SVert           operator+( const SVert& lhs, const SVert& rhs );
void            ReportIcosahedron( const SIcosahedron& ico );
void            CheckIcosahedron( const SIcosahedron& ico );
void            NormalizeIcosahedron( SIcosahedron *pIco );
void            CalcCoordinates( SIcosahedron *pIco );
void            SaveIcosahedronGeom( const SIcosahedron& ico, const char *pFilename );
void            SaveIcosahedronData( const SIcosahedron& ico, const char *pFilename );
SIcosahedron    CreateIcosahedron();
SIcosahedron    SplitIcosahedron( SIcosahedron& oldIco );
////////////////////////////////////////////////////////////////////////////////////////////////////
