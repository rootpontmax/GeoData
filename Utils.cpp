#include "Utils.h"

#include <fstream>
#include <iostream>
#include <cassert>
#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////////
struct SFloat24
{
    SFloat24( const float _v ) :
        val( _v )
    {}
    union
    {
        const float     val;
        const uint8_t   byte[4];
    };
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SInt24
{
    SInt24( const uint8_t a, const uint8_t b, const uint8_t c ) :
        byte{ a, b, c, 0 }
    {}
    SInt24( const int _v ) :
        val( _v )
    {}
    union
    {
        const int       val;
        const uint8_t   byte[4];
    };
};
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t GetProcessTime()
{
    rusage ru;
    if( getrusage( RUSAGE_SELF, &ru ) != -1 )
    {
        const uint64_t ms = ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000;
        return ms;
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t GetFileSize( std::fstream& file )
{
    const size_t cachedPos = file.tellg();
    
    file.seekg( 0, file.end );
    const size_t fileSize = file.tellg();
    file.seekg( cachedPos, file.beg );
    
    return fileSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveFlt24( std::ofstream& file, const float val )
{
    const SFloat24 value( val );
    
    /*
    const int idA = static_cast< int >( value.byte[0] );
    const int idB = static_cast< int >( value.byte[1] );
    const int idC = static_cast< int >( value.byte[2] );
    const int idD = static_cast< int >( value.byte[3] );
    printf( "%02x %02x %02x %02x\n", idA, idB, idC, idD );
    */
    
    file.write( (char*)&value.byte[0], sizeof( uint8_t ) );
    file.write( (char*)&value.byte[1], sizeof( uint8_t ) );
    file.write( (char*)&value.byte[2], sizeof( uint8_t ) );    
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveInt24( std::ofstream& file, const int val )
{
    const SInt24 value( val );
    assert( 0 == value.byte[3] );
    file.write( (char*)&value.byte[0], sizeof( uint8_t ) );
    file.write( (char*)&value.byte[1], sizeof( uint8_t ) );
    file.write( (char*)&value.byte[2], sizeof( uint8_t ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int ReadInt( std::ifstream& file )
{
    int val;
    file.read( (char*)&val, sizeof( int ) );
    return val;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float ReadFlt( std::ifstream& file )
{
    float val;
    file.read( (char*)&val, sizeof( float ) );
    return val;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int ReadInt24( std::ifstream& file )
{
    uint8_t symbol[3];
    file.read( (char*)&symbol[0], sizeof( uint8_t ) );
    file.read( (char*)&symbol[1], sizeof( uint8_t ) );
    file.read( (char*)&symbol[2], sizeof( uint8_t ) );
    
    const SInt24 value( symbol[0], symbol[1], symbol[2] );
    return value.val;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
