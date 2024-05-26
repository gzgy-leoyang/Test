#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

extern int event_0CEE_UDS( char* _buf,int _dlc);
extern int event_0CEF_UDS( char* _buf,int _dlc);

extern int event_0CEA( char* _buf,int _dlc);
extern int event_0CEB( char* _buf,int _dlc);

class Event_handler 
{
  public :
    Event_handler();
    ~Event_handler();
};

#endif
