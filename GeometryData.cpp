#include "GeometryData.h"

#include <map>
#include <cmath>
#include <fstream>
#include <cassert>

#include "Utils.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert::SVert() :
    x( 0.0f ),
    y( 0.0f ),
    z( 0.0f )
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert::SVert( const float _x, const float _y, const float _z ) :
    x( _x ),
    y( _y ),
    z( _z )
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert& SVert::operator+( const SVert& rhs )
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert& SVert::operator*( float coef )
{
    x *= coef;
    y *= coef;
    z *= coef;
    return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert SVert::GetNormalazed() const
{
    SVert res( x, y, z );
    res.Normalize();
    return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SVert::Normalize()
{
    const float length = sqrt( x * x + y * y + z * z );
    x /= length;
    y /= length;
    z /= length;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SEdge::SEdge() :
    idA( INVALID_ID ),
    idB( INVALID_ID ),
    faceID{ INVALID_ID, INVALID_ID }
{}
////////////////////////////////////////////////////////////////////////////////////////////////////    
SEdge::SEdge( const int _idA, const int _idB ) :
    faceID{ INVALID_ID, INVALID_ID }
{
    const bool bIsABigger = ( _idA > _idB ); 
    idA = bIsABigger ? _idB : _idA; 
    idB = bIsABigger ? _idA : _idB;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SEdge::RegisterFace( const int id )
{
    for( int i = 0; i < 2; ++i )
        if( INVALID_ID == faceID[i] )
        {
            faceID[i] = id;
            return;
        }
    abort();
}
/*
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SEdge::operator==( const SEdge& rhs ) const
{
    return ( idA == rhs.idA && idB == rhs.idB );
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
SFace::SFace( const int _regionID, const int idA, const int idB, const int idC ) :
    regionID( _regionID ),
    pointID{ idA, idB, idC }
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
SIcosahedron::SIcosahedron() :
    level( 0 )
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator<( const SEdge& lhs, const SEdge& rhs )
{
    if( lhs.idA < rhs.idA )
        return true;
    else if( lhs.idA == rhs.idA )
        return ( lhs.idB < rhs.idB );
    else
        return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SVert operator+( const SVert& lhs, const SVert& rhs )
{
    SVert res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;
    return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ReportIcosahedron( const SIcosahedron& ico )
{
    const int vertCount = static_cast< int >( ico.vert.size() );
    const int edgeCount = static_cast< int >( ico.edge.size() );
    const int faceCount = static_cast< int >( ico.face.size() );
    
    printf( "\nIcosahedron report:\n" );
    printf( "\tLevel:      %d\n", ico.level );
    printf( "\tVert count: %d\n", vertCount );
    printf( "\tEdge count: %d\n", edgeCount );
    printf( "\tFace count: %d\n", faceCount );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheckIcosahedron( const SIcosahedron& ico )
{
    // Check the edges
    for( size_t i = 0; i < ico.edge.size(); ++i ) 
    {
        const SEdge& edge = ico.edge[i];
        assert( INVALID_ID != edge.faceID[0] && INVALID_ID != edge.faceID[1] );
    }
    
    // Check the triangles
    for( size_t i = 0; i < ico.face.size(); ++i )
    {
        const SFace& face = ico.face[i];
        assert( INVALID_ID != face.edgeID[0] && INVALID_ID != face.edgeID[1] && INVALID_ID != face.edgeID[2] );
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static int FindEdgeID( const SIcosahedron *pIco, const std::map< SEdge, int >& dict, const SEdge& edge )
{
    assert( pIco );
    
    std::map< SEdge, int >::const_iterator it = dict.find( edge );
    if( it == dict.end() )
    {
        assert( false );
        return INVALID_ID;
    }
    
    return it->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void EstablishConnectivity( SIcosahedron *pIco )
{
    assert( pIco );
    const int vertCount = static_cast< int >( pIco->vert.size() );
    const int faceCount = static_cast< int >( pIco->face.size() );
    const int edgeCount = static_cast< int >( pIco->edge.size() );
    
    // Create dictionary edge-id
    std::map< SEdge, int > dict;
    for( int i = 0; i < edgeCount; ++i )
        dict.insert( std::pair< SEdge, int >( pIco->edge[i], i ) );
    
    // Create connectivity
    for( int faceID = 0; faceID < faceCount; ++faceID )
        for( int i = 0; i < 3; ++i )
        {
            const int j = ( i + 1 ) % 3;
            const int idA = pIco->face[faceID].pointID[i];
            const int idB = pIco->face[faceID].pointID[j];
            assert( idA >= 0 && idA < vertCount );
            assert( idB >= 0 && idB < vertCount );
            const SEdge edge( idA, idB );
            const int edgeID = FindEdgeID( pIco, dict, edge );
            assert( edgeID >= 0 && edgeID < edgeCount );
            pIco->face[faceID].edgeID[i] = edgeID;
            pIco->edge[edgeID].RegisterFace( faceID );
        }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NormalizeIcosahedron( SIcosahedron *pIco )
{
    assert( pIco );
    for( size_t i = 0; i < pIco->vert.size(); ++i )
        pIco->vert[i].Normalize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CalcCoordinates( SIcosahedron *pIco )
{
    assert( pIco );
    
    const int vertCount = static_cast< int >( pIco->vert.size() );
    const int faceCount = static_cast< int >( pIco->face.size() );
    
    // Iterate through all faces, find its middle point and calc its UV coordinate
    for( int i = 0; i < faceCount; ++i )
    {
        SFace& face = pIco->face[i];
        
        const int idA = face.pointID[0];
        const int idB = face.pointID[1];
        const int idC = face.pointID[2];
        
        assert( idA >= 0 && idA < vertCount );
        assert( idB >= 0 && idB < vertCount );
        assert( idC >= 0 && idC < vertCount );

        const SVert& vertA = pIco->vert[idA];
        const SVert& vertB = pIco->vert[idB];
        const SVert& vertC = pIco->vert[idC];
        
        const SVert middleVert = ( vertA + vertB + vertC );
        const SVert horizontal( middleVert.x, 0.0f, middleVert.z );
        const SVert normal = middleVert.GetNormalazed();
        const SVert equator = horizontal.GetNormalazed();

        const float radToDegCoef = 180.0f / 3.1415926f;
        const float angleV = 90.0f - acos( normal.y ) * radToDegCoef;
        const float angleH = acos( equator.x ) * radToDegCoef;
        
        face.angleLat = ( normal.y >= 0.0f ) ? angleV : -angleV;
        face.angleLon = ( equator.z >= 0.0f ) ? ( 180.0f - angleH ) : ( 180.0f + angleH );
        
        // Correct longitude angle
        if( face.angleLon < 0.0f )
            face.angleLon += 360.0f;
        
        assert( face.angleLat >= -90.0f & face.angleLat <= 90.0f );
        assert( face.angleLon >= 0.0f & face.angleLon <= 360.0f );
        
        
        //printf( "%f\n", face.angleLat );
        //printf( "%f : %f\n", face.angleLat, face.angleLon );
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveIcosahedronGeom( const SIcosahedron& ico, const char *pFilename )
{
    printf( "\nSaving geometry to %s...\n", pFilename );
    
    const int vertCount = static_cast< int >( ico.vert.size() );
    const int edgeCount = static_cast< int >( ico.edge.size() );
    const int faceCount = static_cast< int >( ico.face.size() );
    
    printf( "\tVert: %d\n", vertCount );
    printf( "\tEdge: %d\n", edgeCount );
    printf( "\tFace: %d\n", faceCount );
    
    std::ofstream file;
    file.open( pFilename, std::ios::out | std::ios::binary );
    
    // Write header
    file.write( (char*)&vertCount, sizeof( int ) );
    file.write( (char*)&edgeCount, sizeof( int ) );
    file.write( (char*)&faceCount, sizeof( int ) );
    
    // Write point positions
    for( int i = 0; i < vertCount; ++i )
    {
        const SVert& vert = ico.vert[i];
        file.write( (char*)&vert.x, sizeof( float ) );
        file.write( (char*)&vert.y, sizeof( float ) );
        file.write( (char*)&vert.z, sizeof( float ) );
    }
    
    // Write edge data
    for( int i = 0; i < edgeCount; ++i )
    {
        const SEdge& edge = ico.edge[i];
        SaveInt24( file, edge.idA );
        SaveInt24( file, edge.idB );
    }
    
    // Write face data
    for( int i = 0; i < faceCount; ++i )
    {
        const SFace& face = ico.face[i];
        for( int j = 0; j < 3; ++j )
            SaveInt24( file, face.edgeID[j] );
    }
        
    file.close();
    
    printf( "\tSaving geom completed.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveIcosahedronData( const SIcosahedron& ico, const char *pFilename )
{
    printf( "\nSaving geoid data to %s...\n", pFilename );
    
    const int faceCount = static_cast< int >( ico.face.size() );
    
    printf( "\tFace: %d\n", faceCount );
    
    std::ofstream file;
    file.open( pFilename, std::ios::out | std::ios::binary );
    
    // Write header
    file.write( (char*)&faceCount, sizeof( int ) );

    // Write face data
    for( int i = 0; i < faceCount; ++i )
    {
        const SFace& face = ico.face[i];
        file.write( (char*)&face.angleLat, sizeof( float ) );
        file.write( (char*)&face.angleLon, sizeof( float ) );
    }
        
    file.close();
    
    printf( "\tSaving data completed.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SIcosahedron CreateIcosahedron()
{
    const float A = 0.26286500f;
    const float B = 0.42532500f;
    const float C = 0.0f;
    
    SIcosahedron ico;
    ico.level = 0;
    ico.vert.reserve( 12 );
    ico.face.reserve( 20 );
    ico.edge.reserve( 30 );

    // Vertexes
    ico.vert.push_back( SVert( -A,  C,  B ) ); //  0
    ico.vert.push_back( SVert(  A,  C,  B ) ); //  1
    ico.vert.push_back( SVert( -A,  C, -B ) ); //  2
    ico.vert.push_back( SVert(  A,  C, -B ) ); //  3
    ico.vert.push_back( SVert(  C,  B,  A ) ); //  4
    ico.vert.push_back( SVert(  C,  B, -A ) ); //  5
    ico.vert.push_back( SVert(  C, -B,  A ) ); //  6
    ico.vert.push_back( SVert(  C, -B, -A ) ); //  7
    ico.vert.push_back( SVert(  B,  A,  C ) ); //  8
    ico.vert.push_back( SVert( -B,  A,  C ) ); //  9
    ico.vert.push_back( SVert(  B, -A,  C ) ); // 10
    ico.vert.push_back( SVert( -B, -A,  C ) ); // 12
    
    // Trianlges. Create and assign regionID
    int regionID = 0;
    ico.face.push_back( SFace( regionID++, 0,  6, 1 ) );
    ico.face.push_back( SFace( regionID++, 0, 11, 6 ) );
    ico.face.push_back( SFace( regionID++, 1,  4, 0 ) );
    ico.face.push_back( SFace( regionID++, 1,  8, 4 ) );
    ico.face.push_back( SFace( regionID++, 1, 10, 8 ) );
    ico.face.push_back( SFace( regionID++, 2,  5, 3 ) );
    ico.face.push_back( SFace( regionID++, 2,  9, 5 ) );
    ico.face.push_back( SFace( regionID++, 2, 11, 9 ) );
    ico.face.push_back( SFace( regionID++, 3,  7, 2 ) );
    ico.face.push_back( SFace( regionID++, 3, 10, 7 ) );
    ico.face.push_back( SFace( regionID++, 4,  8, 5 ) );
    ico.face.push_back( SFace( regionID++, 4,  9, 0 ) );
    ico.face.push_back( SFace( regionID++, 5,  8, 3 ) );
    ico.face.push_back( SFace( regionID++, 5,  9, 4 ) );
    ico.face.push_back( SFace( regionID++, 6, 10, 1 ) );
    ico.face.push_back( SFace( regionID++, 6, 11, 7 ) );
    ico.face.push_back( SFace( regionID++, 7, 10, 6 ) );
    ico.face.push_back( SFace( regionID++, 7, 11, 2 ) );
    ico.face.push_back( SFace( regionID++, 8, 10, 3 ) );
    ico.face.push_back( SFace( regionID++, 9, 11, 0 ) );
    
    // Assign regionID and create edges
    std::set< SEdge > edgeDict;
    for( size_t i = 0; i < ico.face.size(); ++i )
    {
        SFace& face = ico.face[i]; 
        for( int i = 0; i < 3; ++i )
        {
            const int j = ( i + 1 ) % 3;
            const int idA = face.pointID[i];
            const int idB = face.pointID[j];
            const SEdge edge( idA, idB );
            edgeDict.insert( edge );
        }
    }
    
    // Copy all edges to vector
    for( std::set< SEdge >::iterator it = edgeDict.begin(); it != edgeDict.end(); ++it )
    {
        const SEdge& edge = *it;
        ico.edge.push_back( edge );
    }
    assert( ico.edge.size() == 30 );
    
    EstablishConnectivity( &ico );
    return ico;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SIcosahedron SplitIcosahedron( SIcosahedron& oldIco )
{
    const int oldVertCount = static_cast< int >( oldIco.vert.size() );
    const int oldEdgeCount = static_cast< int >( oldIco.edge.size() );
    const int oldFaceCount = static_cast< int >( oldIco.face.size() );
    
    const int newEdgeCount = oldEdgeCount * 2 + oldFaceCount * 3;
    const int newFaceCount = oldFaceCount * 4;
    const int newVertCount = oldVertCount + oldEdgeCount;
    
    SIcosahedron newIco;
    newIco.vert.reserve( newVertCount );
    newIco.face.reserve( newFaceCount );
    newIco.edge.reserve( newEdgeCount );
    newIco.level = oldIco.level + 1;
    
    // Copy old vertexes
    for( int i = 0; i < oldVertCount; ++i )
        newIco.vert.push_back( oldIco.vert[i] );
        
    // Calculate middle points of edges
    for( int i = 0; i < oldEdgeCount; ++i )
    {
        SEdge& edge = oldIco.edge[i];
        edge.idC = static_cast< int >( newIco.vert.size() );
        const SVert middle = ( oldIco.vert[edge.idA] + oldIco.vert[edge.idB] ) * 0.5f;
        newIco.vert.push_back( middle );
    }
    
    // Split old edges: A,B,... -> A1,A2,B1,B2,....
    for( int i = 0; i < oldEdgeCount; ++i )
    {
        const SEdge& edge = oldIco.edge[i];
        const SEdge edgeA( edge.idA, edge.idC );
        const SEdge edgeB( edge.idB, edge.idC );
        newIco.edge.push_back( edgeA );
        newIco.edge.push_back( edgeB );
    }
    
    // Add new edges and add new faces
    for( int i = 0; i < oldFaceCount; ++i )
    {
        SFace& oldFace = oldIco.face[i]; 
        const int idEdgeA = oldFace.edgeID[0];
        const int idEdgeB = oldFace.edgeID[1];
        const int idEdgeC = oldFace.edgeID[2];
        assert( idEdgeA >= 0 && idEdgeA < oldEdgeCount );
        assert( idEdgeB >= 0 && idEdgeB < oldEdgeCount );
        assert( idEdgeC >= 0 && idEdgeC < oldEdgeCount );
        const SEdge& edgeA = oldIco.edge[idEdgeA];
        const SEdge& edgeB = oldIco.edge[idEdgeB];
        const SEdge& edgeC = oldIco.edge[idEdgeC];
        
        const SFace newFaceA( oldFace.regionID, oldFace.pointID[0], edgeA.idC, edgeC.idC );
        const SFace newFaceB( oldFace.regionID, oldFace.pointID[1], edgeA.idC, edgeB.idC );
        const SFace newFaceC( oldFace.regionID, oldFace.pointID[2], edgeB.idC, edgeC.idC );
        const SFace newFaceD( oldFace.regionID, edgeA.idC, edgeB.idC, edgeC.idC );
        
        const SEdge newEdgeA( edgeA.idC, edgeC.idC );
        const SEdge newEdgeB( edgeA.idC, edgeB.idC );
        const SEdge newEdgeC( edgeB.idC, edgeC.idC );
        
        newIco.face.push_back( newFaceA );
        newIco.face.push_back( newFaceB );
        newIco.face.push_back( newFaceC );
        newIco.face.push_back( newFaceD );
        
        newIco.edge.push_back( newEdgeA );
        newIco.edge.push_back( newEdgeB );
        newIco.edge.push_back( newEdgeC );
    }
    
    EstablishConnectivity( &newIco );

    return newIco;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
