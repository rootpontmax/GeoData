#include "TerraData.h"

#include <fstream>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////
STerraData::STerraData() :
    height( 0.0f ),
    population( 0.0f ),
    bIsLand( false ),
    bIsWater( false ),
    bIsInit( false )
{
    for( int i = 0; i < 12; ++i )
    {
        landTempDay[i] = 0.0f;
        landTempNight[i] = 0.0f;
        seaTemp[i] = 0.0f;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CTerraData::CTerraData( const int count ) :
    m_count( count )
{
    assert( m_count > 0 );
    m_data.resize( m_count );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTerraData::Check()
{
    const int cellCount = static_cast< int >( m_data.size() );
    for( int i = 0; i < cellCount; ++i )
    {
        const STerraData& data = m_data[i];
        const bool bIsLandAndSea = data.bIsLand && data.bIsWater;
        assert( data.bIsInit );
        assert( bIsLandAndSea ); // Cool!
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTerraData::CreateSnapShot( const int id )
{
    
    //bool compress_image_to_jpeg_file(const char *pFilename, int width, int height, int num_channels, const uint8 *pImage_data, const params &comp_params = params());

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTerraData::Load( const char *pFilename  )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTerraData::Save( const char *pFilename  )
{
    printf( "\nSaving geoid data to %s...\n", pFilename );
    
    const int cellCount = static_cast< int >( m_data.size() );
    printf( "\tCell: %d\n", cellCount );
        
    std::ofstream file;
    file.open( pFilename, std::ios::out | std::ios::binary );
    
    // Write header
    file.write( (char*)&cellCount, sizeof( int ) );

    // Write cell data
    for( int i = 0; i < cellCount; ++i )
    {
        const STerraData& data = m_data[i];
        
        file.write( (char*)&data.angleLat, sizeof( float ) );
        file.write( (char*)&data.angleLon, sizeof( float ) );
        
        file.write( (char*)&data.height, sizeof( float ) );
        file.write( (char*)&data.population, sizeof( float ) );
        
        for( int j = 0; j < 12; ++j )
        {
            file.write( (char*)&data.landTempDay[j], sizeof( float ) );
            file.write( (char*)&data.landTempNight[j], sizeof( float ) );
            file.write( (char*)&data.seaTemp[j], sizeof( float ) );
        }
    }
        
    file.close();
    
    printf( "\tSaving geoid data completed.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int CTerraData::GetCount() const
{
    assert( m_count == static_cast< int >( m_data.size() ) );
    return m_count;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
STerraData& CTerraData::GetData( const int id )
{
    assert( m_count == static_cast< int >( m_data.size() ) );
    assert( id >= 0 && id < m_count );
    return m_data[id];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
