/*
 *  log_ctrl.h - The log controller class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2015-2-16 Zhang Ziyi
 *  Initial version.
 */
#ifndef LOG_CTRL_H_
#define LOG_CTRL_H_

#include "cp_log_cmn.h"
#include "multiplexer.h"
#include "client_mgr.h"
#include "log_pipe_hdl.h"
#include "modem_stat_hdl.h"
#include "wcnd_stat_hdl.h"
#include "log_config.h"
#include "log_file.h"
#include "stor_mgr.h"

class LogController {
 public:
  LogController();
  ~LogController();

  int init(LogConfig* config);
  int run();

  ClientManager* cli_mgr() { return m_cli_mgr; }

  LogPipeHandler* get_cp(CpType type);

  void process_cp_evt(CpType type, CpStateHandler::CpEvent evt);

  // Client requests
  int enable_log(const CpType* cps, size_t num);
  int disable_log(const CpType* cps, size_t num);
  bool get_log_state(CpType ct) const;
  int enable_md();
  int disable_md();
  int mini_dump();
  int reload_slog_conf();
  size_t get_log_file_size() const;
  int set_log_file_size(size_t len);
  bool get_log_overwrite() const;
  int set_log_overwrite(bool en = true);
  size_t get_data_part_size() const;
  int set_data_part_size(size_t sz);
  size_t get_sd_size() const;
  int set_sd_size(size_t sz);
  bool get_md_int_stor() const;
  int set_md_int_stor(bool md_int_stor);
  int clear_log();
  /*  enable_wcdma_iq - enable WCDMA I/Q saving.
   *  @cpt: CP type
   *
   *  Return LCR_SUCCESS on success, LCR_xxx on error.
   *  Possible errors:
   *    LCR_CP_NONEXISTENT  CP not exist
   */
  int enable_wcdma_iq(CpType cpt);
  /*  disable_wcdma_iq - disable WCDMA I/Q saving.
   *  @cpt: CP type
   *
   *  Return LCR_SUCCESS on success, LCR_xxx on error.
   *  Possible errors:
   *    LCR_CP_NONEXISTENT  CP not exist
   */
  int disable_wcdma_iq(CpType cpt);
  /*  copy_cp_files - copy cp (log/memory dump) files into modem_log.
   *  @t - cp type
   *  @file_type - log/dump type of files to be copied.
   *  @src_file_path - path of which to be copied to modem log.
   *
   *  Return LCR_SUCCESS on success, LCR_ERROR on error.
   *  Possible errors:
   *    LCR_CP_NONEXISTENT  CP not exist
   */
  int copy_cp_files(CpType t, LogFile::LogType file_type,
                    const LogString& src_file_path);

#ifdef SUPPORT_AGDSP
  LogConfig::AgDspLogDestination get_agdsp_log_dest() const;
  int set_agdsp_log_dest(LogConfig::AgDspLogDestination dest);
  bool get_agdsp_pcm_dump() const;
  int set_agdsp_pcm_dump(bool pcm);
#endif

  /*  flush - flush buffered log into file.
   *
   *  Return LCR_SUCCESS.
   */
  int flush();

  /*
   *    save_sipc_dump - Save the SIPC dump.
   *    @stor: the CP storage handle
   *    @t: the time to be used as the file name
   *
   *    Return Value:
   *      Return 0 on success, -1 otherwise.
   */
  static int save_sipc_dump(CpStorage* stor, const struct tm& t);

  /*
   *    save_etb - save ETB.
   *    @stor: the CP storage handle
   *    @lt: the time to be used as the file name
   *
   *    Return Value:
   *      Return 0 on success, -1 otherwise.
   */
  static int save_etb(CpStorage* stor, const struct tm& lt);

 private:
  typedef LogList<LogPipeHandler*>::iterator LogPipeIter;

  LogConfig* m_config;
  Multiplexer m_multiplexer;
  ClientManager* m_cli_mgr;
  LogList<LogPipeHandler*> m_log_pipes;
  ModemStateHandler* m_modem_state;
  WcndStateHandler* m_wcn_state;

  // Storage manager
  StorageManager m_stor_mgr;
  /*
   * create_handler - Create the LogPipeHandler object and try to
   *                  open the device if it's enabled in the config.
   */
  LogPipeHandler* create_handler(const LogConfig::ConfigEntry* e);

  /*
   * init_state_handler - Create the ModemStateHandler object for
   *                      connections to modemd and wcnd.
   */
  ModemStateHandler* init_state_handler(const char* serv_name);

  /*
   * init_wcn_state_handler - Create the ExtWcnStateHandler
   *                          or the IntWcnStateHandler.
   * @sock_name - Unix domain socket name of the WCN state provider.
   * @assert_msg - assert notification
   *
   * Return Value:
   *   initialized wcn state handler on success or
   *   nullptr if fail to create wcn state handler
   */
  WcndStateHandler* init_wcn_state_handler(const char* sock_name,
                                           const char* assert_msg);
  /*
   * init_wcn_state_handler - Create the ExtWcnStateHandler
   *                          or the IntWcnStateHandler.
   * @addr - IP address of the WCN state provider.
   * @port - TCP port of the WCN state provider.
   * @assert_msg - assert notification
   *
   * Return Value:
   *   initialized wcn state handler on success or
   *   nullptr if fail to create wcn state handler
   */
  WcndStateHandler* init_wcn_state_handler(const char* addr,
                                           unsigned short port,
                                           const char* assert_msg);

  static LogList<LogPipeHandler*>::iterator find_log_handler(
      LogList<LogPipeHandler*>& handlers, CpType t);
  static LogList<LogPipeHandler*>::const_iterator find_log_handler(
      const LogList<LogPipeHandler*>& handlers, CpType t);
};

#endif  // !LOG_CTRL_H_
