////////////////////////////////////////////////////////////////////////////////////////////////////
// Utils for make and check huge files.                                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <cstddef>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t    GetProcessTime();
size_t      GetFileSize( std::fstream& file );
void        SaveFlt24( std::ofstream& file, const float val );
void        SaveInt24( std::ofstream& file, const int val );
int         ReadInt( std::ifstream& file );
float       ReadFlt( std::ifstream& file );
int         ReadInt24( std::ifstream& file );
////////////////////////////////////////////////////////////////////////////////////////////////////