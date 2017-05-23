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

  using StatusResponseHandler = std::function<void(
      const std::wstring &status,
      const std::wstring &source_title)>;

  static NCSTREAMER_REMOTE_DLL_API void SetUp(uint16_t remote_port);
  static NCSTREAMER_REMOTE_DLL_API void SetUpDefault();

  static NCSTREAMER_REMOTE_DLL_API void ShutDown();
  static NCSTREAMER_REMOTE_DLL_API NcStreamerRemote *Get();

  void NCSTREAMER_REMOTE_DLL_API RequestStatus(
      const ErrorHandler &error_handler,
      const StatusResponseHandler &status_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestExit(
      const ErrorHandler &error_handler);

 private:
  using AsioClient = ws::config::asio_client;
  using ConnectHandler = std::function<void()>;

  explicit NcStreamerRemote(uint16_t remote_port);
  virtual ~NcStreamerRemote();

  void Connect(
    const ErrorHandler &error_handler,
    const ConnectHandler &connect_handler);

  void SendStatusRequest();
  void SendExitRequest();

  void OnRemoteFail(ws::connection_hdl connection);
  void OnRemoteOpen(ws::connection_hdl connection);
  void OnRemoteClose(ws::connection_hdl connection);
  void OnRemoteMessage(
      ws::connection_hdl connection,
      ws::connection<AsioClient>::message_ptr msg);

  void OnRemoteStatusResponse(
      const boost::property_tree::ptree &response);

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

  ConnectHandler current_connect_handler_;
};
}  // namespace ncstreamer_remote


#endif  // NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_H_
