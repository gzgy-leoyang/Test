#ifndef ADAPTER_H
#define ADAPTER_H

#include <string>

class Adapter {
public:
  Adapter();
  ~Adapter();

  static int readStringFromFile( const char*  _file,
                                 std::string& _str );
  static int writeStringToFile( const char* _file,
                                std::string& _str );

  static int readIntFromFile( const char*  _file,
                              int& _val );
  static int readBinaryFromFile( const char* _file,
                                 void* _str,
                                 const uint8_t _size);
};
#endif
