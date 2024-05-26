#include "support/utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int compile_datetime( char* _cdt )
{
  if ( _cdt == NULL )
    return -1;

        char  date_buf[20] = {'\0'};
  const char* month[]      = {"Jan","Feb","Mar",
                              "Apr","May","Jun",
                              "Jul","Aug","Sep",
                              "Oct","Nov","Dec"};
  const char* _date = (char*)(__DATE__);

  //int ll = strlen(_date);
  //if ( ll < 11 ){
  //  printf(" [ERROR] The lenght of __DATE__ is loss than %i\n",ll);
  //  return -1;
  //}

  // year
  memcpy((char*)(date_buf),(char*)(_date+9), 2 );
  int mon = 0;
  for ( int i=0;i<12;i++ ){
    if ( memcmp((void*)(month[i]),(void*)(_date),3) == 0){
      mon = i + 1;
      break;
    }
  }
  // mon
  date_buf[2] = mon / 10 + '0';
  date_buf[3] = mon % 10 + '0';
  // day
  if ( __DATE__[4] == ' ' ){
    date_buf[4] = '0';
  } else {
    date_buf[4]  = __DATE__[4] ;
  }
  date_buf[5]  = __DATE__[5] ;
  // hour
  date_buf[6]  = __TIME__[0];
  date_buf[7]  = __TIME__[1];
  // min
  date_buf[8] = __TIME__[3];
  date_buf[9] = __TIME__[4];
  memcpy( _cdt,date_buf,11 );
  return 0;
}
