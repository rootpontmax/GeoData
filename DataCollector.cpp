#include "DataCollector.h"

#include <cfloat>
#include <iostream>
#include <thread>
#include <mutex>

#include "TerraData.h"
#include "tinyXML/tinyXML.h"
#include "jpeg/jpgd.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char  *g_pCollectRoot = "collect";
static const char  *g_pCollectItem = "item";
static const char  *g_pAttrFilename = "filename";
static const char  *g_pAttrImageType = "imageType";
static const char  *g_pAttrDataType = "dataType";
////////////////////////////////////////////////////////////////////////////////////////////////////
static const char  *g_pAttrRangeMin = "rangeMin";
static const char  *g_pAttrRangeMax = "rangeMax";
static const char  *g_pAttrMonth = "month";
////////////////////////////////////////////////////////////////////////////////////////////////////
CDataCollector::SImageData::SImageData() :
    rangeMin( FLT_MAX ),
    rangeMax( -FLT_MAX ),
    month( -1 ),
    bWasStarted( false )
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *CDataCollector::m_pImageTypeStr[IMAGE_TYPE_COUNT] =
{
    "grayScale",
    "color"
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *CDataCollector::m_pDataTypeStr[DATA_TYPE_COUNT] =
{
    "topography",
    "oceanDepth",
    "population",
    "temperatureDay",
    "temperatureNight",
    "temperatureSea"
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CDataCollector::CDataCollector( CTerraData *pData, const int coreCount ) :
    m_pData( pData ),
    m_coreCount( coreCount )
{
    assert( m_pData );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::Collect( const char *pFilenameXML )
{
    CollectImageData( pFilenameXML );
    //ReportInputDataQueue();
    Process();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::CollectImageData( const char *pFilenameXML )
{
    // Open the XML-file and read
    TiXmlDocument xmlDoc( pFilenameXML );
    const bool bIsLoadingOK = xmlDoc.LoadFile();
    
    if( !bIsLoadingOK )
    {
        std::cout << "Error loading XML-file: " << pFilenameXML << std::endl;
        return;
    }
    
    // Find node "collect"
    const TiXmlNode *pRoot = xmlDoc.FirstChild( g_pCollectRoot );
    if( !pRoot )
    {
        std::cout << "There is nothing to collect in XML-file" << std::endl;
        return;
    }
    
    const int itemCount = GetNodeChildCount( pRoot, g_pCollectItem );
    m_imageData.reserve( itemCount );
    
    // Parse and process all collect nodes
    const TiXmlNode *pNode = pRoot->FirstChild( g_pCollectItem );
    while( pNode )
    {
        // Read element's data
        const TiXmlElement *pElement = pNode->ToElement();
        const char *pAttrFilename = pElement->Attribute( g_pAttrFilename );
        const char *pAttrImageType = pElement->Attribute( g_pAttrImageType );
        const char *pAttrDataType = pElement->Attribute( g_pAttrDataType );
        const char *pAttrRangeMin = pElement->Attribute( g_pAttrRangeMin );
        const char *pAttrRangeMax = pElement->Attribute( g_pAttrRangeMax );
        const char *pAttrMonth = pElement->Attribute( g_pAttrMonth );
        
        // Parse this data into internal formats
        const EImageType imageType = ParseImageType( pAttrImageType );
        const EDataType dataType = ParseDataType( pAttrDataType );
        const float rangeMin = atof( pAttrRangeMin );
        const float rangeMax = atof( pAttrRangeMax );
        const int month = pAttrMonth ? atoi( pAttrMonth ) : -1;
        
        // Create element
        SImageData data;
        data.filename = pAttrFilename;
        data.imageType = imageType;
        data.dataType = dataType;
        data.rangeMin = rangeMin;
        data.rangeMax = rangeMax;
        data.month = month;
        
        m_imageData.push_back( data );
        
        // Get next element
        pNode = pNode->NextSiblingElement();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::Process()
{
    assert( m_pData );
    
    // Start thread pool
    std::mutex jobMutex;
    std::mutex dataMutex;
    
    std::vector< std::thread > threadPool;
    threadPool.reserve( m_coreCount );
    for( int i = 0; i < m_coreCount; ++i )
        threadPool.push_back( std::thread( ThreadProcessImage, i, this, std::ref( jobMutex ), std::ref( dataMutex ) ) );
     
    // Waiting until all process finished
    for( int i = 0; i < m_coreCount; ++i )
        threadPool[i].join();
        
    // Create terra data
    m_pData->Check();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CDataCollector::GetNodeChildCount( const TiXmlNode *pRoot, const char *pChildName )
{
    assert( pRoot && pChildName );
    
    int retCount = 0;
    const TiXmlNode *pNode = pRoot->FirstChild( g_pCollectItem );
    while( pNode )
    {
        ++retCount;
        pNode = pNode->NextSiblingElement();
    }
    return retCount;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CDataCollector::EImageType CDataCollector::ParseImageType( const char *pImageTypeStr )
{
    for( int i = 0; i < IMAGE_TYPE_COUNT; ++i )
        if( strncmp( pImageTypeStr, m_pImageTypeStr[i], strlen( m_pImageTypeStr[i] ) ) == 0 )
        {
            const EImageType retType = static_cast< EImageType >( i );
            return retType;
        }
    
    std::cout << "Unknown EImageType type" << std::endl;
    return IMAGE_TYPE_COUNT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CDataCollector::EDataType CDataCollector::ParseDataType( const char *pDataTypeStr )
{
    for( int i = 0; i < DATA_TYPE_COUNT; ++i )
        if( strncmp( pDataTypeStr, m_pDataTypeStr[i], strlen( m_pDataTypeStr[i] ) ) == 0 )
        {
            const EDataType retType = static_cast< EDataType >( i );
            return retType;
        }
    
    std::cout << "Unknown EDataType type" << std::endl;
    return DATA_TYPE_COUNT;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *CDataCollector::GetStringForImageType( const EImageType type )
{
    assert( type >= 0 && type < IMAGE_TYPE_COUNT );
    return m_pImageTypeStr[type];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char *CDataCollector::GetStringForDataType( const EDataType type )
{
    assert( type >= 0 && type < DATA_TYPE_COUNT );
    return m_pDataTypeStr[type];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::ThreadProcessImage( const int threadID, CDataCollector *pThis,
                                         std::mutex& jobMutex, std::mutex& dataMutex )
{
    // Infinite loop to process task
    for( ; ; )
    {
        // Find first unprocessed task
        int workID = -1;
        jobMutex.lock();
        for( size_t i = 0; i < pThis->m_imageData.size(); ++i )
            if( !pThis->m_imageData[i].bWasStarted )
            {
                pThis->m_imageData[i].bWasStarted = true;
                workID = static_cast< int >( i );
                break;
            }
        jobMutex.unlock();
        
        // No more unprocessed images
        if( -1 == workID )
            break;
        
        // Do your job
        SImageData& imageData = pThis->m_imageData[workID];
        ProcessImage( pThis, imageData, dataMutex );
        imageData.bWasStarted = true;
    }
    
    printf( "Thread %d finished\n", threadID );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::ProcessImage( CDataCollector *pThis, const SImageData& imageData, std::mutex& mtx )
{
    // Использован код из:
    // http://code.google.com/p/jpeg-compressor/

    // Загружаем файл
    int imageSizeX = 0;
    int imageSizeY = 0;
    int actualComps = 0;
    int reqComps = 4;
    uint8_t *pBuffer = jpgd::decompress_jpeg_image_from_file( imageData.filename.c_str(), &imageSizeX, &imageSizeY, &actualComps, reqComps );
    
    assert( pBuffer );
    
    const float sizeX = static_cast< float >( imageSizeX - 1 ); 
    const float sizeY = static_cast< float >( imageSizeY - 1 );
    const size_t pixelStride = sizeof( uint8_t ) * 4;
    
    // Go through all terraData
    assert( pThis->m_pData );
    const int cellCount = pThis->m_pData->GetCount();
    for( int i = 0; i < cellCount; ++i )
    {
        STerraData& data = pThis->m_pData->GetData( i );
        
        // Recalc angles to pixel coordinates
        const float coefX = data.angleLon / 360.0f;
        const float coefY = 1.0f - ( ( data.angleLat + 90.0f ) / 180.0f );
        assert( coefX >= 0.0f && coefX <= 1.0f );
        assert( coefY >= 0.0f && coefY <= 1.0f );
        const int x = static_cast< int >( sizeX * coefX );
        const int y = static_cast< int >( sizeY * coefY );
        assert( x >= 0 && x < imageSizeX );
        assert( y >= 0 && y < imageSizeY );
        
        const int pos = y * imageSizeX + x;
        const int offset = pos * pixelStride;
            
        const uint8_t colR = pBuffer[offset    ];
        const uint8_t colG = pBuffer[offset + 1];
        const uint8_t colB = pBuffer[offset + 2];
            
        ProcessPixel( pThis, data, imageData, mtx, colR, colG, colB );
        
        //const int x = static_cast< int >( static_cast< float >( imageSizeX ) * data.angleLon / 360.0f );
        //const int y = static_cast< int >( static_cast< float >( imageSizeX ) * data.angleLon / 360.0f );
    }
    
    /*
    const size_t pixelStride = sizeof( uint8_t ) * 4;
    for( int y = 0; y < imageSizeY; ++y )
        for( int x = 0; x < imageSizeX; ++x )
        {
            const int pos = y * imageSizeX + x;
            const int offset = pos * pixelStride;
            
            const uint8_t colR = pBuffer[offset    ];
            const uint8_t colG = pBuffer[offset + 1];
            const uint8_t colB = pBuffer[offset + 2];
            
            ProcessPixel( pThis, imageData, mtx, x, y, colR, colG, colB );
        }
    */
    
    // Освобождаем память
    free( pBuffer );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::ProcessPixel( CDataCollector *pThis,
                                   STerraData& terraData,
                                   const SImageData& imageData, std::mutex& mtx,
                                   const uint8_t colR, const uint8_t colG, const uint8_t colB )
{
    const bool bIsWhite = ( colR == 255 );
    const float coef = static_cast< float >( colR ) / 255.0f;
    switch( imageData.dataType )
    {
        // The same things
        case DATA_TYPE_TOPOGRAPHY:
        case DATA_TYPE_OCEAN_DEPTH:
            {
                if( bIsWhite )
                    return;
                mtx.lock();
                terraData.height = imageData.rangeMin + ( imageData.rangeMax - imageData.rangeMin ) * coef;
                mtx.unlock();
            }
            break;
            
        case DATA_TYPE_POPULATION:
            terraData.population = imageData.rangeMin + ( imageData.rangeMax - imageData.rangeMin ) * coef;
            break;
            
        case DATA_TYPE_TEMPERATURE_DAY:
            assert( imageData.month >= 0 && imageData.month < 12 );
            terraData.landTempDay[imageData.month] = imageData.rangeMin + ( imageData.rangeMax - imageData.rangeMin ) * coef;
            terraData.bIsLand = true;
            terraData.bIsInit = true;
            break;
            
        case DATA_TYPE_TEMPERATURE_NIGHT:
            assert( imageData.month >= 0 && imageData.month < 12 );
            terraData.landTempNight[imageData.month] = imageData.rangeMin + ( imageData.rangeMax - imageData.rangeMin ) * coef;
            terraData.bIsLand = true;
            terraData.bIsInit = true;
            break;
            
        case DATA_TYPE_TEMPERATURE_SEA:
            assert( imageData.month >= 0 && imageData.month < 12 );
            terraData.seaTemp[imageData.month] = imageData.rangeMin + ( imageData.rangeMax - imageData.rangeMin ) * coef;
            terraData.bIsWater = true;
            terraData.bIsInit = true;
            break;
            
        default:
            std::cout << "Unknown EDataType" << std::endl;
            break;
    }
    
    //mtx.unlock();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataCollector::ReportInputDataQueue()
{
    const size_t count = m_imageData.size();
    for( size_t i = 0; i < count; ++i )
    {
        const SImageData& data = m_imageData[i];
        printf( "Image%04d\n", (int)i );
        printf( "    file:      %s\n", data.filename.c_str() );
        printf( "    imageType: %s\n", GetStringForImageType( data.imageType ) );
        printf( "    dataType:  %s\n", GetStringForDataType( data.dataType ) );
        printf( "    rangeMin:  %f\n", data.rangeMin );
        printf( "    rangeMax:  %f\n", data.rangeMax );
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
