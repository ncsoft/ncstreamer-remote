/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_H_
#define NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_H_


#ifdef NCSTREAMER_REMOTE_DLL_EXPORTS
#define NCSTREAMER_REMOTE_DLL_API __declspec(dllexport)
#else
#define NCSTREAMER_REMOTE_DLL_API __declspec(dllimport)
#endif


#include <atomic>
#include <fstream>
#include <functional>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/property_tree/ptree.hpp"

#ifdef _MSC_VER
#pragma warning(disable: 4267)
#endif
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4267)
#endif


namespace ncstreamer_remote {
namespace ws = websocketpp;


class NcStreamerRemote {
 public:
  using ErrorHandler = std::function<void(
      const std::wstring &err_msg)>;

  using SuccessHandler = std::function<void(
      bool success)>;

  /// @param status One of ['standby', 'setup', 'starting', 'onAir', 'stopping']
  using StatusResponseHandler = std::function<void(
      const std::wstring &status,
      const std::wstring &source_title,
      const std::wstring &user_name,
      const std::wstring &quality)>;

  using StartResponseHandler = std::function<void(
      bool success)>;

  using StopResponseHandler = std::function<void(
      bool success)>;

  static NCSTREAMER_REMOTE_DLL_API void SetUp(uint16_t remote_port);
  static NCSTREAMER_REMOTE_DLL_API void SetUpDefault();

  static NCSTREAMER_REMOTE_DLL_API void ShutDown();
  static NCSTREAMER_REMOTE_DLL_API NcStreamerRemote *Get();

  void NCSTREAMER_REMOTE_DLL_API RequestStatus(
      const ErrorHandler &error_handler,
      const StatusResponseHandler &status_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestStart(
      const std::wstring &title,
      const ErrorHandler &error_handler,
      const StartResponseHandler &start_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestStop(
      const std::wstring &title,
      const ErrorHandler &error_handler,
      const StopResponseHandler &stop_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestQualityUpdate(
      const std::wstring &quality,
      const ErrorHandler &error_handler,
      const SuccessHandler &quality_update_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestExit(
      const ErrorHandler &error_handler);

 private:
  using AsioClient = ws::config::asio_client;
  using ConnectHandler = std::function<void()>;

  explicit NcStreamerRemote(uint16_t remote_port);
  virtual ~NcStreamerRemote();

  bool ExistsNcStreamer();

  void Connect(
    const ErrorHandler &error_handler,
    const ConnectHandler &connect_handler);

  void Connect(
    const ConnectHandler &connect_handler);

  void SendStatusRequest();
  void SendStartRequest(const std::wstring &title);
  void SendStopRequest(const std::wstring &title);
  void SendQualityUpdateRequest(const std::wstring &quality);
  void SendExitRequest();

  void OnRemoteFail(ws::connection_hdl connection);
  void OnRemoteClose(ws::connection_hdl connection);
  void OnRemoteMessage(
      ws::connection_hdl connection,
      ws::connection<AsioClient>::message_ptr msg);

  void OnRemoteStatusResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteStartResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteStopResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteQualityUpdateResponse(
      const boost::property_tree::ptree &response);

  void HandleError(
      const std::string &err_type,
      const ws::lib::error_code &ec,
      const ErrorHandler &err_handler);
  void HandleError(
      const std::string &err_type,
      const ws::lib::error_code &ec);
  void HandleError(
      const std::string &err_msg,
      const ErrorHandler &err_handler);
  void HandleError(
      const std::string &err_msg);

  void LogWarning(const std::string &warn_msg);
  void LogError(const std::string &err_msg);

  static NcStreamerRemote *static_instance;

  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  ws::client<AsioClient> remote_;
  std::vector<std::thread> remote_threads_;
  std::ofstream remote_log_;

  ws::uri_ptr remote_uri_;

  ws::connection_hdl remote_connection_;

  std::atomic_bool busy_;

  ErrorHandler current_error_handler_;
  StatusResponseHandler current_status_response_handler_;
  StartResponseHandler current_start_response_handler_;
  StopResponseHandler current_stop_response_handler_;
  SuccessHandler current_quality_update_response_handler_;
};
}  // namespace ncstreamer_remote


#endif  // NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_H_
