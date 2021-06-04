/*
 *  log_pipe_hdl.h - The CP log and dump handler class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2015-3-2 Zhang Ziyi
 *  Initial version.
 */
#ifndef LOG_PIPE_HDL_H_
#define LOG_PIPE_HDL_H_

#include <time.h>
#ifdef HOST_TEST_
#include "prop_test.h"
#else
#include <cutils/properties.h>
#endif
#include "cp_log_cmn.h"
#include "cp_stat_hdl.h"
#include "data_consumer.h"
#include "fd_hdl.h"
#include "log_config.h"
#include "log_file.h"
#include "req_err.h"

class ClientHandler;
class CpStorage;
class DiagDeviceHandler;
class LogFile;
class StorageManager;

class LogPipeHandler : public FdHandler {
 public:
  LogPipeHandler(LogController* ctrl, Multiplexer* multi,
                 const LogConfig::ConfigEntry* conf, StorageManager& stor_mgr);
  ~LogPipeHandler();

  CpType type() const { return m_type; }

  const LogString& name() const { return m_modem_name; }

  bool enabled() const { return m_enable; }

  bool log_diag_dev_same() const { return m_log_diag_same; }

  const LogString& log_dev_path() const { return m_log_dev_path; }

  const LogString& diag_dev_path() const { return m_diag_dev_path; }

  /*
   *    copy_files - copy log/dump... file to modem log.
   *    @file_type : file type (log files or dump files)
   *    @src_file_path : original source file path
   *
   *    Return Value:
   *       LCR_SUCCESS if success
   *       LCR_ERROR if fail
   */
  int copy_files(LogFile::LogType file_type, const LogString& src_file_path);

  void set_log_diag_dev_path(const LogString& log, const LogString& diag) {
    m_log_dev_path = log;
    m_diag_dev_path = diag;
    m_log_diag_same = false;
  }

  void set_log_diag_dev_path(const LogString& path) {
    m_log_dev_path = path;
    m_log_diag_same = true;
    m_diag_dev_path.clear();
  }

  bool in_transaction() const { return m_cp_state > CWS_WORKING; }

  /*
   *    start - Initialize the log device and start logging.
   *
   *    This function put the LogPipeHandler in an adminitrative
   *    enable state and try to open the log device.
   *
   *    Return Value:
   *      0
   */
  virtual int start() = 0;

  /*
   *    stop - Stop logging.
   *
   *    This function put the LogPipeHandler in an adminitrative
   *    disable state and close the log device.
   *
   *    Return Value:
   *      0
   */
  virtual int stop() = 0;

  /*  pause - pause logging and I/Q before log clearing.
   *
   *  This function assume m_enable is true.
   *
   *  Return Value:
   *    0
   */
  virtual int pause() { return 0; }
  /*  resume - resume logging and I/Q after log clearing.
   *
   *  This function assume m_enable is true.
   *
   *  Return Value:
   *    0
   */
  virtual int resume() { return 0; }

  void process(int events);

  /*
   *    process_assert - Process the CP assertion.
   *
   *    This function will be called by the LogController when the
   *    CP asserts.
   */
  virtual void process_assert(const struct tm& lt) = 0;

  void process_reset();

  /*  process_alive - process the alive event.
   *
   *  This function reopen the log device file if necessary.
   *
   *  Derived class shall override the function if necessary.
   */
  virtual void process_alive();
  /*
   *  save_sleep_log - Start a sleep log saving transaction.
   *  @client: the ClientHandler object.
   *
   *  Return Values:
   *    LCR_IN_PROGRESS: the transaction is started successfully.
   *    LCR_IN_TRANSACTION: there is a transaction ongoing.
   *    LCR_LOG_DISABLED: the log is disabled for the CP and the
   *                      transaction can not be started.
   */
  int save_sleep_log(ClientHandler* client);
  /*
   *  save_ringbuf - Start a ringbuf saving transaction.
   *  @client: the ClientHandler object.
   *
   *  Return Values:
   *    LCR_IN_PROGRESS: the transaction is started successfully.
   *    LCR_IN_TRANSACTION: there is a transaction ongoing.
   *    LCR_LOG_DISABLED: the log is disabled for the CP and the
   *                      transaction can not be started.
   */
  int save_ringbuf(ClientHandler* client);
  /*
   *  cancel_trans_result_notify - Cancel the result notification of
   *                               the current transaction.
   *  @client: the ClientHandler object.
   */
  void cancel_trans_result_notify(ClientHandler* client);
  /*  flush - flush buffered log into file.
   */
  void flush();
  /*  open_dump_mem_file - Open .mem file to store CP memory from
   *                       /proc/cpxxx/mem.
   */
  LogFile* open_dump_mem_file(const struct tm& lt);

  CpStateHandler* cp_state_hdl() const { return m_cp_state_hdl; }

  void set_cp_state_hdl(CpStateHandler* hdl) { m_cp_state_hdl = hdl; }

 protected:
  enum CpWorkState {
    CWS_NOT_WORKING,
    CWS_WORKING,
    CWS_DUMP,
    CWS_SAVE_SLEEP_LOG,
    CWS_SAVE_RINGBUF,
    CWS_QUERY_VER
  };

  size_t m_max_buf;
  size_t m_max_buf_num;
  size_t m_log_commit_threshold;
  // Log data buffer
  DataBuffer* m_buffer;

  CpWorkState cp_state() const { return m_cp_state; }

  void set_cp_state(CpWorkState cs) { m_cp_state = cs; }

  int open();
  /*
   *    close_devices - Close the log device, stop sleep log/RingBuf.
   *
   *    This function assumes m_fd is opened.
   */
  void close_devices();

  StorageManager& stor_mgr() { return m_stor_mgr; }
  CpStorage* storage() { return m_storage; }
  virtual int create_storage();

  DiagDeviceHandler* create_diag_device(DataConsumer* consumer);

  DataConsumer* consumer() { return m_consumer; }

  /*  end_dump - clean up dump state.
   */
  void end_dump();

  /*  start_transaction - Save the DiagDeviceHandler and the consumer,
   *                      and change the m_cp_state.
   */
  void start_transaction(DiagDeviceHandler* dev, DataConsumer* consumer,
                         CpWorkState state);

  /*  stop_transaction - stop current transaction.
   *
   *  This function will destruct the transaction object and diagnosis
   *  device handler.
   *
   *  Derived class shall override this function.
   */
  virtual void stop_transaction(CpWorkState state);

  /*
   *    start_logging - start cp log when enabled.
   *    Return value:
   *      Return 0 if start log with success.
   */
  int start_logging();

  /*
   *    stop_logging - Stop logging.
   *
   *    This function put the LogPipeHandler in an adminitrative
   *    disable state and close the log device.
   *
   *    Return Value:
   *      0
   */
  int stop_logging();
  /*
   *    will_be_reset - check whether the CP will be reset.
   */
  bool will_be_reset() const;

  void close_on_assert();
  void inform_cp_controller(CpStateHandler::CpNotify evt);

  virtual void on_new_log_file(LogFile* /*f*/) {}

  static void new_log_callback(LogPipeHandler* cp, LogFile* f) {
    cp->on_new_log_file(f);
  }

  /*
   *    log_buffer - Buffer shared by all LogPipeHandlers.
   *
   *    process function of all LogPipeHandler objects are called
   *    from the same thread.
   */
  static const size_t LOG_BUFFER_SIZE = (32 * 1024);
  static uint8_t log_buffer[LOG_BUFFER_SIZE];

  /*
   *    reopen_log_dev - Try to open the log device and the mini
   *                     dump device.
   *
   *    This function is called by the multiplexer on check event.
   */
  static void reopen_log_dev(void* param);

  static void set_wcn_dump_prop();

 private:
  // Log turned on
  bool m_enable;
  LogString m_modem_name;
  CpType m_type;
  // Log device is the same as the diag device ?
  bool m_log_diag_same;
  // Log device file path
  LogString m_log_dev_path;
  // Diag device file path
  LogString m_diag_dev_path;
  // Reset property
  const char* m_reset_prop;
  CpWorkState m_cp_state;
  // CP state connection
  CpStateHandler* m_cp_state_hdl;
  // Current dump/sleep log/RingBuf handler
  DiagDeviceHandler* m_diag_handler;
  DataConsumer* m_consumer;
  // Current transaction client
  ClientHandler* m_trans_client;
  StorageManager& m_stor_mgr;
  CpStorage* m_storage;

  int start_sleep_log();
  int start_ringbuf();

  /*  diag_transaction_notify - Diagnosis port transaction result
   *                            notification function.
   *  @client: client parameter. It's the LogPipeHandler* pointer.
   *  @result: the transaction result.
   *
   *  This function is called by current DataConsumer object.
   */
  static void diag_transaction_notify(void* client,
                                      DataConsumer::LogProcResult res);
  static void buf_avail_callback(void* client);
};

#endif  // !LOG_PIPE_HDL_H_
