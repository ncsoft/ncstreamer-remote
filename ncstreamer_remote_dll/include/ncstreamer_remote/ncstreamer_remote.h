/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_NCSTREAMER_REMOTE_H_
#define NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_NCSTREAMER_REMOTE_H_


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
#include "boost/asio/steady_timer.hpp"
#include "boost/property_tree/ptree.hpp"

#ifdef _MSC_VER
#pragma warning(disable: 4267)
#endif
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"
#ifdef _MSC_VER
#pragma warning(default: 4267)
#endif

#if _MSC_VER >= 1900
#include <chrono>  // NOLINT
namespace Chrono = std::chrono;
#else
#include "boost/chrono/include.hpp"
namespace Chrono = boost::chrono;
#endif  // _MSC_VER >= 1900

#include "ncstreamer_remote_dll/include/ncstreamer_remote/error/error_types.h"


namespace ncstreamer_remote {
class NcStreamerRemote {
 public:
  class WebcamDevice;

  using ConnectHandler = std::function<void()>;
  using DisconnectHandler = std::function<void()>;

  using ErrorHandler = std::function<void(
      ErrorCategory category,
      int err_code,
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
  using StartEventHandler = std::function<void(
      const std::wstring &source_title,
      const std::wstring &user_page,
      const std::wstring &privacy,
      const std::wstring &description,
      const std::wstring &mic,
      const std::wstring &service_provider,
      const std::wstring &stream_url,
      const std::wstring &post_url)>;

  using StopResponseHandler = std::function<void(
      bool success)>;
  using StopEventHandler = std::function<void(
      const std::wstring &source_title)>;

  using CommentsResponseHandler = std::function<void(
      const std::wstring &msg)>;
  using WebcamSearchResponseHandler = std::function<void(
      const std::vector<NcStreamerRemote::WebcamDevice> &webcams)>;
  using WebcamResponseHandler = std::function<void()>;
  using ChromaKeyResponseHandler = std::function<void()>;
  using MicResponseHandler = std::function<void()>;

  static NCSTREAMER_REMOTE_DLL_API void SetUp(uint16_t remote_port);
  static NCSTREAMER_REMOTE_DLL_API void SetUpDefault();

  static NCSTREAMER_REMOTE_DLL_API void ShutDown();
  static NCSTREAMER_REMOTE_DLL_API NcStreamerRemote *Get();

  void NCSTREAMER_REMOTE_DLL_API RegisterConnectHandler(
      const ConnectHandler &connect_handler);

  void NCSTREAMER_REMOTE_DLL_API RegisterDisconnectHandler(
      const DisconnectHandler &disconnect_handler);

  void NCSTREAMER_REMOTE_DLL_API RegisterStartEventHandler(
      const StartEventHandler &start_event_handler);

  void NCSTREAMER_REMOTE_DLL_API RegisterStopEventHandler(
      const StopEventHandler &stop_event_handler);

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

  void NCSTREAMER_REMOTE_DLL_API RequstComments(
      const std::wstring &created_time,
      const ErrorHandler &error_handler,
      const CommentsResponseHandler &comments_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestWebcamSearch(
      const ErrorHandler &error_handler,
      const WebcamSearchResponseHandler &webcam_search_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestWebcamOn(
      const std::wstring &device_id,
      const float &normal_width,
      const float &normal_height,
      const float &normal_x,
      const float &normal_y,
      const ErrorHandler &error_handler,
      const WebcamResponseHandler &webcam_on_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestWebcamOff(
      const ErrorHandler &error_handler,
      const WebcamResponseHandler &webcam_off_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestWebcamSize(
      const float &normal_width,
      const float &normal_height,
      const ErrorHandler &error_handler,
      const WebcamResponseHandler &webcam_size_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestWebcamPosition(
      const float &normal_x,
      const float &normal_y,
      const ErrorHandler &error_handler,
      const WebcamResponseHandler &webcam_position_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestChromaKeyOn(
      const uint32_t &color,
      const int &similarity,
      const ErrorHandler &error_handler,
      const ChromaKeyResponseHandler &chroma_key_on_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestChromaKeyOff(
      const ErrorHandler &error_handler,
      const ChromaKeyResponseHandler &chroma_key_off_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestChromaKeyColor(
      const uint32_t &color,
      const ErrorHandler &error_handler,
      const ChromaKeyResponseHandler &chroma_key_color_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestChromaKeySimilarity(
      const int &similarity,
      const ErrorHandler &error_handler,
      const ChromaKeyResponseHandler &chroma_key_similarity_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestMicOn(
      const ErrorHandler &error_handler,
      const MicResponseHandler &mic_on_response_handler);

  void NCSTREAMER_REMOTE_DLL_API RequestMicOff(
      const ErrorHandler &error_handler,
      const MicResponseHandler &mic_off_response_handler);

 private:
  using SteadyTimer = boost::asio::basic_waitable_timer<Chrono::steady_clock>;
  using AsioClient = websocketpp::config::asio_client;
  using OpenHandler = std::function<void()>;

  explicit NcStreamerRemote(uint16_t remote_port);
  virtual ~NcStreamerRemote();

  bool ExistsNcStreamer();

  void KeepConnected();

  void Connect(
    const ErrorHandler &error_handler,
    const OpenHandler &open_handler);

  void Connect(
    const OpenHandler &open_handler);

  void SendStatusRequest();
  void SendStartRequest(const std::wstring &title);
  void SendStopRequest(const std::wstring &title);
  void SendQualityUpdateRequest(const std::wstring &quality);
  void SendExitRequest();
  void SendCommentsRequest(const std::wstring &created_time);
  void SendWebcamSearchRequest();
  void SendWebcamOnRequest(
      const std::wstring &device_id,
      const float &normal_width,
      const float &normal_height,
      const float &normal_x,
      const float &normal_y);
  void SendWebcamOffRequest();
  void SendWebcamSizeRequest(
      const float &normal_width,
      const float &normal_height);
  void SendWebcamPositionRequest(
      const float &normal_x,
      const float &normal_y);
  void SendChromaKeyOnRequest(
      const uint32_t &color,
      const int &similarity);
  void SendChromaKeyOffRequest();
  void SendChromaKeyColorRequest(const uint32_t &color);
  void SendChromaKeySimilarityRequest(const int &similarity);
  void SendMicOnRequest();
  void SendMicOffRequest();

  void OnRemoteFail(websocketpp::connection_hdl connection);
  void OnRemoteClose(websocketpp::connection_hdl connection);
  void OnRemoteMessage(
      websocketpp::connection_hdl connection,
      websocketpp::connection<AsioClient>::message_ptr msg);

  void OnRemoteStartEvent(
      const boost::property_tree::ptree &evt);
  void OnRemoteStopEvent(
      const boost::property_tree::ptree &evt);

  void OnRemoteStatusResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteStartResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteStopResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteQualityUpdateResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteCommentsResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteWebcamSearchResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteWebcamOnResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteWebcamOffResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteWebcamSizeResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteWebcamPositionResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteChromaKeyOnResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteChromaKeyOffResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteChromaKeyColorResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteChromaKeySimilarityResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteMicOnResponse(
      const boost::property_tree::ptree &response);
  void OnRemoteMicOffResponse(
      const boost::property_tree::ptree &response);

  void HandleDisconnect(
      Error::Connection err_code);
  void HandleConnectionError(
      Error::Connection err_code,
      const ErrorHandler &err_handler);

  void HandleError(
      Error::Connection err_code,
      const websocketpp::lib::error_code &ec,
      const ErrorHandler &err_handler);
  void HandleError(
      Error::Connection err_code,
      const websocketpp::lib::error_code &ec);
  void HandleError(
      Error::Connection err_code,
      const std::string &err_msg,
      const ErrorHandler &err_handler);
  void HandleError(
      Error::Connection err_code,
      const ErrorHandler &err_handler);
  void HandleError(
      Error::Connection err_code);

  void LogWarning(const std::string &warn_msg);
  void LogError(const std::string &err_msg);

  static NcStreamerRemote *static_instance;

  boost::asio::io_service io_service_;
  boost::asio::io_service::work io_service_work_;
  websocketpp::client<AsioClient> remote_;
  std::vector<std::thread> remote_threads_;
  std::ofstream remote_log_;

  websocketpp::uri_ptr remote_uri_;

  websocketpp::connection_hdl remote_connection_;
  SteadyTimer timer_to_keep_connected_;

  std::atomic_bool busy_;

  ConnectHandler connect_handler_;
  DisconnectHandler disconnect_handler_;
  StartEventHandler start_event_handler_;
  StopEventHandler stop_event_handler_;

  ErrorHandler current_error_handler_;
  StatusResponseHandler current_status_response_handler_;
  StartResponseHandler current_start_response_handler_;
  StopResponseHandler current_stop_response_handler_;
  SuccessHandler current_quality_update_response_handler_;
  CommentsResponseHandler current_comments_response_handler_;
  WebcamSearchResponseHandler current_webcam_search_response_handler_;
  WebcamResponseHandler current_webcam_on_response_handler_;
  WebcamResponseHandler current_webcam_off_response_handler_;
  WebcamResponseHandler current_webcam_size_response_handler_;
  WebcamResponseHandler current_webcam_position_response_handler_;
  ChromaKeyResponseHandler current_chroma_key_on_response_handler_;
  ChromaKeyResponseHandler current_chroma_key_off_response_handler_;
  ChromaKeyResponseHandler current_chroma_key_color_response_handler_;
  ChromaKeyResponseHandler current_chroma_key_similarity_response_handler_;
  MicResponseHandler current_mic_on_response_handler_;
  MicResponseHandler current_mic_off_response_handler_;
};


class NcStreamerRemote::WebcamDevice {
 public:
  NCSTREAMER_REMOTE_DLL_API WebcamDevice(
      const std::wstring &id,
      const int &default_width,
      const int &default_height);
  virtual NCSTREAMER_REMOTE_DLL_API ~WebcamDevice();

  const std::wstring NCSTREAMER_REMOTE_DLL_API &id() const {
    return id_;
  }
  const int NCSTREAMER_REMOTE_DLL_API &default_width() const {
    return default_width_;
  }
  const int NCSTREAMER_REMOTE_DLL_API &default_height() const {
    return default_height_;
  }

 private:
  std::wstring id_;
  int default_width_;
  int default_height_;
};
}  // namespace ncstreamer_remote


#endif  // NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_NCSTREAMER_REMOTE_H_
