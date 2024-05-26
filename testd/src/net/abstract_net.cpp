#include "net/abstract_net.h"
#include "main.h"

Abstract_net::
Abstract_net( )
{
}

int Abstract_net::
transmit_message( const int& _info,
                  char* _buf,
                  const int& _size )
{
  UNUSED(_info);
  UNUSED(_buf);
  UNUSED(_size);

  return 0;
}

//int Abstract_net::
//add_diagnostic_id( const uint32_t& _req_id,const uint32_t& _ack_id )
//{
//  m_dignostic_req_id = _req_id;
//  m_dignostic_ack_id = _ack_id;
//  return 0;
//}
//
//int Abstract_net::
//add_id_and_mask( QList<uint32_t> _list,const uint32_t& _mask )
//{
//  Q_UNUSED( _list );
//  Q_UNUSED( _mask);
//  return 0;
//}
/// end of code ///
