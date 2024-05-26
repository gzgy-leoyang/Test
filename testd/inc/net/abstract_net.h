#ifndef ABSTRACT_NET_H
#define ABSTRACT_NET_H

//#include "../main.h"
//#include "../support/debug.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

class Abstract_net
{
public:
    Abstract_net();
    virtual int transmit_message( const int& _info,
                                  char* _buf,
                                  const int& _size );

protected:
};
#endif
