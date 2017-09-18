////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  main.cpp
//  Matrix
//
//  Created by Mikhail Scherbakov on 08/09/16.
//  Copyright © 2016 valve. All rights reserved.
//
//  Class CDataCollector: collects data from JPEG
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <vector>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////////////////////////
class TiXmlNode;
class CTerraData;
struct STerraData;
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDataCollector
{
public:
    CDataCollector( CTerraData *pData, const int coreCount );
    void    Collect( const char *pFilenameXML );

private:
    
    enum EImageType
    {
        IMAGE_TYPE_GRAY_SCALE,
        IMAGE_TYPE_COLOR,
        IMAGE_TYPE_COUNT
    };
    
    enum EDataType
    {
        DATA_TYPE_TOPOGRAPHY,           // Meters. Zero - sea level
        DATA_TYPE_OCEAN_DEPTH,          //
        DATA_TYPE_POPULATION,           // Person(s) per square kilometer
        DATA_TYPE_TEMPERATURE_DAY,      // Centigrade
        DATA_TYPE_TEMPERATURE_NIGHT,    //
        DATA_TYPE_TEMPERATURE_SEA,      //
        DATA_TYPE_COUNT
    };
    
    // Internal structure to represent inut data
    struct SImageData
    {
        SImageData();
        
        std::string filename;
        EImageType  imageType;
        EDataType   dataType;
        float       rangeMin;
        float       rangeMax;
        int         month;
        
        // Flags
        bool        bWasStarted;
    };
    
    // Typedefs
    typedef std::vector< SImageData > TImageVec;
    //typedef void (*TDataFunc)( const int x, const int y, const uint8_t colR, const uint8_t colG, const uint8_t colB );
    
    // String constants for various types
    static const char *m_pImageTypeStr[IMAGE_TYPE_COUNT];
    static const char *m_pDataTypeStr[DATA_TYPE_COUNT];
    
    // Main working steps
    void        CollectImageData( const char *pFilenameXML );
    void        Process();
    
    // Aux methods
    int         GetNodeChildCount( const TiXmlNode *pRoot, const char *pChildName );
    
    // Methods for parsing input data and get string name by its type
    EImageType  ParseImageType( const char *pImageTypeStr );
    EDataType   ParseDataType( const char *pDataTypeStr );
    const char *GetStringForImageType( const EImageType type );
    const char *GetStringForDataType( const EDataType type );
    
    // Thread functions and its parts
    static void ThreadProcessImage( const int threadID, CDataCollector *pThis,
                                    std::mutex& jobMutex, std::mutex& dataMutex );
    static void ProcessImage( CDataCollector *pThis, const SImageData& imageData, std::mutex& mtx );
    static void ProcessPixel( CDataCollector *pThis,
                              STerraData& terraData,
                              const SImageData& imageData, std::mutex& mtx,
                              const uint8_t colR, const uint8_t colG, const uint8_t colB );
    
    // Report functions
    void        ReportInputDataQueue();
    
    CTerraData *m_pData;
    
    // Data
    TImageVec   m_imageData;
    const int   m_coreCount;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
