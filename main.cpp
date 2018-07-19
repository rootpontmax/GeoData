////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  main.cpp
//  Matrix
//
//  Created by Mikhail Scherbakov on 08/09/16.
//  Copyright © 2016 valve. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cassert>
#include <thread>
#include <cmath>

#include <set>

#include "TerraData.h"
#include "DataCollector.h"
#include "GeometryData.h"
#include "Utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static const size_t g_memorySize = 256 << 20;
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CalculateIcosahedron()
{
    int countV = 3;
    int countT = 1;
    int countE = 3;
    int n = 0;
    int patchHierarchyTrianglesCount = 0;
    while( countV <= 65536 )
    {
        patchHierarchyTrianglesCount += countT;
        const int totalTriaCount = countT;
        const int textureSizeA = static_cast< int >( sqrt( (double)totalTriaCount ) );
        assert( textureSizeA * textureSizeA >= totalTriaCount );
         
        printf( "[%d] Vertex: %5d. Triangles: %5d/%7d. Edges: %5d. Texture: %d. \n",
            n,
            countV,
            countT,
            totalTriaCount * 20,
            countE,
            textureSizeA );
            
        countV = countV + countE;
        countE = countE * 2 + countT * 3;
        countT *= 4;
        
        ++n;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CreateGeometryData()
{
    std::cout << "Create geometry data..." << std::endl;
    
    CalculateIcosahedron();
        
    SIcosahedron ico = CreateIcosahedron();
    CheckIcosahedron( ico );
    ReportIcosahedron( ico );
    
    for( int i = 0; i < 8; ++i )
    {
        const uint64_t timeA = GetProcessTime();
        //ico = std::move( SplitIcosahedron( ico ) );
        ico = SplitIcosahedron( ico );
        CheckIcosahedron( ico );
        ReportIcosahedron( ico );
        const uint64_t timeB = GetProcessTime();
        const uint64_t timeDelta = timeB - timeA;
        const int timeDeltaMS = static_cast< int >( timeDelta );
        printf( "\tSplit time: %d ms\n", timeDeltaMS );
    }
    
    NormalizeIcosahedron( &ico );
    CalcCoordinates( &ico );
    SaveIcosahedronGeom( ico, "GeoidGeom.bin" );
    SaveIcosahedronData( ico, "GeoidFace.bin" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void CreateGeoidData( const int coreCount )
{
    const char *pFaceFilename = "GeoidFace.bin";
    
    // Loading
    std::cout << "Load geometry face data from file: " << pFaceFilename << std::endl;
    std::ifstream file;
    file.open( pFaceFilename, std::ios::in | std::ios::binary );
    if( !file.is_open() )
    {
        std::cout << "Can't open file: " << pFaceFilename << std::endl;
        return;
    }
    const int faceCount = ReadInt( file );
    if( faceCount <= 0 )
        return;
        
    CTerraData terraData( faceCount );
    for( int i = 0; i < faceCount; ++i )
    {
        STerraData& data = terraData.GetData( i );
        data.angleLat = ReadFlt( file );
        data.angleLon = ReadFlt( file );
    }
    file.close();
    std::cout << "Loading completed for " << terraData.GetCount() << " face(s)" << std::endl;
    
    // Load configuration from xml and parse it
    CDataCollector dataCollector( &terraData, coreCount );
    dataCollector.Collect( "config.xml" );
    terraData.Save( "terraData.bin" );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int main( int argc, const char * argv[] )
{
    const char *pCreateGeomCmd = "-createGeom";
    const char *pCreateDataCmd = "-createData";
    
    std::cout << "TerraData" << std::endl;
    
    const int coreNumber = std::thread::hardware_concurrency();
    std::cout << "Use " << coreNumber << " core(s) and " << g_memorySize << " byte(s)"<< std::endl;
    
    if( argc < 2 )
    {
        std::cout << "Wrong arguments count"<< std::endl;
        std::cout << "Usage:"<< std::endl;
        std::cout << "\t[" << pCreateGeomCmd << "] - Create geometry"<< std::endl;
        std::cout << "\t[" << pCreateDataCmd << "] - Create geoid data"<< std::endl;
        return 0;
    }
    
    const char * const pCommand = argv[1];
    if( strcmp( pCommand, pCreateGeomCmd ) == 0 )
        CreateGeometryData();
    else if( strcmp( pCommand, pCreateDataCmd ) == 0 )
        CreateGeoidData( coreNumber );
        
    std::cout << std::endl << "Completed." << std::endl << std::endl;
        
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
