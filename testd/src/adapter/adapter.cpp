#include "adapter/adapter.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "support/debug_out.h"

using std::ifstream;
using std::ios;


/*
int className:: 
function_rw( char* _buf,int _size )
{
  int sid = *(_buf+1);
  int br = 0;
  if ( sid == 0x22 ) {

  } else if ( sid == 0x2E ) {

  } else {
    // unknow SID
  }
  return 0;
}
*/

Adapter::
Adapter() 
{}

Adapter::
~Adapter() 
{}

int Adapter::
readStringFromFile( const char* _file,
                    std::string& _str )
{
  std::ifstream fin( _file );
  if (!fin.is_open()){
    return -1;
  }
  fin.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
  try{
    char readStr[100];
    std::memset(readStr,0,100);
    fin.getline(readStr,100);
    fin.close();
    _str = std::string(readStr);
    return 0;
  } catch(std::ifstream::failure &e) {
    LOGW<<" Exception: Failed to read string stream from file";
    fin.close();
    return -1;
  }
  return 0;
}

int Adapter::
writeStringToFile( const char* _file,
                   std::string& _str )
{
  std::ofstream fout( _file );
  if (!fout.is_open()){
    return -1;
  }
  fout.exceptions ( std::ofstream::failbit | std::ofstream::badbit );

  try{
    fout.write(_str.c_str(),_str.size()); 
    fout.flush();
    fout.close();
    return 0;
  } catch(std::ofstream::failure &e) {
    LOGW<<" Exception: Failed to read string stream from file";
    fout.close();
    return -1;
  }
  return 0;
}

int Adapter::
readBinaryFromFile( const char* _file,
                    void* _str,
                    const uint8_t _size)
{
  std::ifstream fin( _file,std::ifstream::binary );
  if (!fin.is_open()){
    return -1;
  }
  fin.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

  try{
    fin.read( reinterpret_cast<char*>(_str) ,_size );
    fin.close();
    return 0;
  } catch(std::ifstream::failure &e) {
    LOGW<<" Exception: Failed to read binary stream from file";
    fin.close();
    return -1;
  }
}

int Adapter::
readIntFromFile( const char*  _file,int& _val )
{
  std::string str;
  if ( readStringFromFile( _file,str ) < 0 ){
    LOGW<<" Failed to read string";
    _val = 0;
    return -1;
  }
  
  int val = 0;
  try {
    val = stoi(str);
  } catch (const std::invalid_argument& exc){
    LOGW<<" Failed to convert string to Int,Invalid_argument,"<<str;
    _val = 0;
    return -1;
  } catch (const std::out_of_range& exc){
    LOGW<<" Failed to convert string to Int,Out of range,"<<str;
    _val = 0;
    return -1;
  } catch (...) {
    LOGW<<" Failed to convert string to Int,"<<str;
    _val = 0;
    return -1;
  }
  _val = val;
  return 0;
}

/// end of code ///
