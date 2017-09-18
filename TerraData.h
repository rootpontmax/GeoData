////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  main.cpp
//  Matrix
//
//  Created by Mikhail Scherbakov on 08/09/16.
//  Copyright © 2016 valve. All rights reserved.
//
//  Class CTerraData which contains of earth data
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
struct STerraData
{
    STerraData();
    
    // Coordinates
    float   angleLat;
    float   angleLon;
    
    // General data
    float   height; // Height in meters: [-8000.0f .. 6400.0f]
    float   population;
    
    // Temperature
    float   landTempDay[12];
    float   landTempNight[12];
    float   seaTemp[12];
    
    // Flags
    bool    bIsLand;
    bool    bIsWater;
    bool    bIsInit;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CTerraData
{
public:
    CTerraData( const int count );
    void        Check();
    void        CreateSnapShot( const int id );
    void        Load( const char *pFilename  );
    void        Save( const char *pFilename  );
    
    int         GetCount() const;
    STerraData& GetData( const int id );
    
    
private:
        
    // Declate bu never define to preven copy
    CTerraData( const CTerraData& );
    CTerraData& operator=( const CTerraData& );
    
    std::vector< STerraData >   m_data;
    const int                   m_count;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
