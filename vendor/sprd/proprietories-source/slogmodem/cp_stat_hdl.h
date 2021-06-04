/*
 *  cp_stat_hdl.h - The general CP state handler.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2015-4-8 Zhang Ziyi
 *  Initial version.
 */
#ifndef _CP_STAT_HDL_H_
#define _CP_STAT_HDL_H_

#include <sys/socket.h>
#include <unistd.h>

#include "cp_log_cmn.h"
#include "data_proc_hdl.h"

class CpStateHandler : public DataProcessHandler {
 public:
  enum CpEvent { CE_NONE, CE_ASSERT, CE_RESET, CE_BLOCKED, CE_ALIVE };
  enum CpNotify { CE_DUMP_START, CE_DUMP_END };

  // Use Unix domain socket
  CpStateHandler(LogController* ctrl, Multiplexer* multiplexer,
                 const char* serv_name);
  // Use TCP socket
  CpStateHandler(LogController* ctrl, Multiplexer* multiplexer,
                 const char* addr, uint16_t port);

  int init();

  int send_dump_response(CpNotify evt);

 protected:
#define MODEM_STATE_BUF_SIZE 256

 private:
  int socket_protocol_;
  // The IP address if socket_protocol_ is AF_INET, otherwise the Unix domain
  // socket name.
  LogString addr_;
  // The TCP port if socket_protocol_ is AF_INET
  unsigned short port_;

  int process_data();
  void process_conn_closed();
  void process_conn_error(int err);

  virtual CpEvent parse_notify(const uint8_t* buf, size_t len,
                               CpType& type) = 0;

  static void connect_server(void* param);
};

#endif  // !_CP_STAT_HDL_H_
