/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_dll/include/ncstreamer_remote/ncstreamer_remote.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <unordered_map>

#include "boost/algorithm/string.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "Windows.h"  // NOLINT

#include "ncstreamer_remote_dll/src/error/error_converter.h"
#include "ncstreamer_remote_dll/src/ncstreamer_remote_message_types.h"


namespace {
namespace placeholders = websocketpp::lib::placeholders;
}  // unnamed namespace


namespace ncstreamer_remote {
void NcStreamerRemote::SetUp(uint16_t remote_port) {
  assert(!static_instance);
  static_instance = new NcStreamerRemote{remote_port};
}


void NcStreamerRemote::SetUpDefault() {
  SetUp(9003);
}


void NcStreamerRemote::ShutDown() {
  assert(static_instance);
  delete static_instance;
  static_instance = nullptr;
}


NcStreamerRemote *NcStreamerRemote::Get() {
  assert(static_instance);
  return static_instance;
}


void NcStreamerRemote::RegisterConnectHandler(
    const ConnectHandler &connect_handler) {
  connect_handler_ = connect_handler;
}


void NcStreamerRemote::RegisterDisconnectHandler(
    const DisconnectHandler &disconnect_handler) {
  disconnect_handler_ = disconnect_handler;
}


void NcStreamerRemote::RegisterStartEventHandler(
    const StartEventHandler &start_event_handler) {
  start_event_handler_ = start_event_handler;
}


void NcStreamerRemote::RegisterStopEventHandler(
    const StopEventHandler &stop_event_handler) {
  stop_event_handler_ = stop_event_handler;
}


void NcStreamerRemote::RequestStatus(
    const ErrorHandler &error_handler,
    const StatusResponseHandler &status_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_status_response_handler_ = status_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendStatusRequest();
    });
    return;
  }

  SendStatusRequest();
}


void NcStreamerRemote::RequestStart(
    const std::wstring &title,
    const ErrorHandler &error_handler,
    const StartResponseHandler &start_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_start_response_handler_ = start_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this, title]() {
      SendStartRequest(title);
    });
    return;
  }

  SendStartRequest(title);
}


void NcStreamerRemote::RequestStop(
    const std::wstring &title,
    const ErrorHandler &error_handler,
    const StopResponseHandler &stop_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_stop_response_handler_ = stop_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this, title]() {
      SendStopRequest(title);
    });
    return;
  }

  SendStopRequest(title);
}


void NcStreamerRemote::RequestQualityUpdate(
    const std::wstring &quality,
    const ErrorHandler &error_handler,
    const SuccessHandler &quality_update_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_quality_update_response_handler_ = quality_update_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this, quality]() {
      SendQualityUpdateRequest(quality);
    });
    return;
  }

  SendQualityUpdateRequest(quality);
}


void NcStreamerRemote::RequestExit(
    const ErrorHandler &error_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendExitRequest();
    });
    return;
  }

  SendExitRequest();
}


void NcStreamerRemote::RequstComments(
    const std::wstring &created_time,
    const ErrorHandler &error_handler,
    const CommentsResponseHandler &comments_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_comments_response_handler_ = comments_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this, created_time]() {
      SendCommentsRequest(created_time);
    });
    return;
  }

  SendCommentsRequest(created_time);
}


void NcStreamerRemote::RequestViewers(
    const ErrorHandler &error_handler,
    const ViewersResponseHandler &viewers_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_viewers_response_handler_ = viewers_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendViewersRequest();
    });
    return;
  }

  SendViewersRequest();
}


void NcStreamerRemote::RequestWebcamSearch(
    const ErrorHandler &error_handler,
    const WebcamSearchResponseHandler &webcam_search_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_webcam_search_response_handler_ = webcam_search_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendWebcamSearchRequest();
    });
    return;
  }

  SendWebcamSearchRequest();
}


void NcStreamerRemote::RequestWebcamOn(
    const std::wstring &device_id,
    const float &normal_width,
    const float &normal_height,
    const float &normal_x,
    const float &normal_y,
    const ErrorHandler &error_handler,
    const WebcamResponseHandler &webcam_on_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_webcam_on_response_handler_ = webcam_on_response_handler;

  if (!remote_connection_.lock()) {
    Connect([
        device_id, normal_width, normal_height, normal_x, normal_y, this]() {
      SendWebcamOnRequest(
          device_id, normal_width, normal_height, normal_x, normal_y);
    });
    return;
  }

  SendWebcamOnRequest(
      device_id, normal_width, normal_height, normal_x, normal_y);
}


void NcStreamerRemote::RequestWebcamOff(
    const ErrorHandler &error_handler,
    const WebcamResponseHandler &webcam_off_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_webcam_off_response_handler_ = webcam_off_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendWebcamOffRequest();
    });
    return;
  }

  SendWebcamOffRequest();
}


void NcStreamerRemote::RequestWebcamSize(
    const float &normal_width,
    const float &normal_height,
    const ErrorHandler &error_handler,
    const WebcamResponseHandler &webcam_size_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_webcam_size_response_handler_ = webcam_size_response_handler;

  if (!remote_connection_.lock()) {
    Connect([normal_width, normal_height, this]() {
      SendWebcamSizeRequest(normal_width, normal_height);
    });
    return;
  }

  SendWebcamSizeRequest(normal_width, normal_height);
}


void NcStreamerRemote::RequestWebcamPosition(
    const float &normal_x,
    const float &normal_y,
    const ErrorHandler &error_handler,
    const WebcamResponseHandler &webcam_position_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_webcam_position_response_handler_ = webcam_position_response_handler;

  if (!remote_connection_.lock()) {
    Connect([normal_x, normal_y, this]() {
      SendWebcamPositionRequest(normal_x, normal_y);
    });
    return;
  }

  SendWebcamPositionRequest(normal_x, normal_y);
}


void NcStreamerRemote::RequestChromaKeyOn(
    const uint32_t &color,
    const int &similarity,
    const ErrorHandler &error_handler,
    const ChromaKeyResponseHandler &chroma_key_on_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_chroma_key_on_response_handler_ = chroma_key_on_response_handler;

  if (!remote_connection_.lock()) {
    Connect([color, similarity, this]() {
      SendChromaKeyOnRequest(color, similarity);
    });
    return;
  }

  SendChromaKeyOnRequest(color, similarity);
}


void NcStreamerRemote::RequestChromaKeyOff(
    const ErrorHandler &error_handler,
    const ChromaKeyResponseHandler &chroma_key_off_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_chroma_key_off_response_handler_ = chroma_key_off_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendChromaKeyOffRequest();
    });
    return;
  }

  SendChromaKeyOffRequest();
}


void NcStreamerRemote::RequestChromaKeyColor(
    const uint32_t &color,
    const ErrorHandler &error_handler,
    const ChromaKeyResponseHandler &chroma_key_color_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_chroma_key_color_response_handler_ =
      chroma_key_color_response_handler;

  if (!remote_connection_.lock()) {
    Connect([color, this]() {
      SendChromaKeyColorRequest(color);
    });
    return;
  }

  SendChromaKeyColorRequest(color);
}


void NcStreamerRemote::RequestChromaKeySimilarity(
    const int &similarity,
    const ErrorHandler &error_handler,
    const ChromaKeyResponseHandler &chroma_key_similarity_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_chroma_key_similarity_response_handler_ =
      chroma_key_similarity_response_handler;

  if (!remote_connection_.lock()) {
    Connect([similarity, this]() {
      SendChromaKeySimilarityRequest(similarity);
    });
    return;
  }

  SendChromaKeySimilarityRequest(similarity);
}


void NcStreamerRemote::RequestMicSearch(
    const ErrorHandler &error_handler,
    const MicSearchResponseHandler &mic_search_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_mic_search_response_handler_ = mic_search_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendMicSearchRequest();
    });
    return;
  }

  SendMicSearchRequest();
}


void NcStreamerRemote::RequestMicOn(
    const std::wstring &device_id,
    const float &volume,
    const ErrorHandler &error_handler,
    const MicResponseHandler &mic_on_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_mic_on_response_handler_ = mic_on_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this, device_id, volume]() {
      SendMicOnRequest(device_id, volume);
    });
    return;
  }

  SendMicOnRequest(device_id, volume);
}


void NcStreamerRemote::RequestMicOff(
    const ErrorHandler &error_handler,
    const MicResponseHandler &mic_off_response_handler) {
  if (busy_ == true) {
    HandleConnectionError(Error::Connection::kBusy, error_handler);
    return;
  }
  busy_ = true;

  current_error_handler_ = error_handler;
  current_mic_off_response_handler_ = mic_off_response_handler;

  if (!remote_connection_.lock()) {
    Connect([this]() {
      SendMicOffRequest();
    });
    return;
  }

  SendMicOffRequest();
}


NcStreamerRemote::NcStreamerRemote(uint16_t remote_port)
    : remote_uri_{new websocketpp::uri{false, "::1", remote_port, ""}},
      io_service_{},
      io_service_work_{io_service_},
      remote_{},
      remote_threads_{},
      remote_log_{},
      remote_connection_{},
      timer_to_keep_connected_{io_service_},
      busy_{},
      connect_handler_{},
      disconnect_handler_{},
      start_event_handler_{},
      stop_event_handler_{},
      current_error_handler_{},
      current_status_response_handler_{},
      current_start_response_handler_{},
      current_stop_response_handler_{},
      current_quality_update_response_handler_{},
      current_comments_response_handler_{},
      current_viewers_response_handler_{},
      current_webcam_search_response_handler_{},
      current_webcam_on_response_handler_{},
      current_webcam_off_response_handler_{},
      current_webcam_size_response_handler_{},
      current_webcam_position_response_handler_{},
      current_chroma_key_on_response_handler_{},
      current_chroma_key_off_response_handler_{},
      current_chroma_key_color_response_handler_{},
      current_chroma_key_similarity_response_handler_{},
      current_mic_search_response_handler_{},
      current_mic_on_response_handler_{},
      current_mic_off_response_handler_{} {
  busy_ = false;

  remote_log_.open("ncstreamer_remote.log");
  remote_.set_access_channels(websocketpp::log::alevel::all);
  remote_.set_access_channels(websocketpp::log::elevel::all);
  remote_.get_alog().set_ostream(&remote_log_);
  remote_.get_elog().set_ostream(&remote_log_);

  websocketpp::lib::error_code ec;
  remote_.init_asio(&io_service_, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteInitAsio, ec);
    assert(false);
    return;
  }

  remote_.set_fail_handler(websocketpp::lib::bind(
      &NcStreamerRemote::OnRemoteFail, this, placeholders::_1));
  remote_.set_close_handler(websocketpp::lib::bind(
      &NcStreamerRemote::OnRemoteClose, this, placeholders::_1));
  remote_.set_message_handler(websocketpp::lib::bind(
      &NcStreamerRemote::OnRemoteMessage, this,
          placeholders::_1, placeholders::_2));

  static const std::size_t kRemoteThreadsSize{1};  // just one enough.
  for (std::size_t i = 0; i < kRemoteThreadsSize; ++i) {
    remote_threads_.emplace_back([this]() {
      remote_.run();
    });
  }

  KeepConnected();
}


NcStreamerRemote::~NcStreamerRemote() {
  remote_.stop();
  for (auto &t : remote_threads_) {
    if (t.joinable() == true) {
      t.join();
    }
  }
}


bool NcStreamerRemote::ExistsNcStreamer() {
  HWND wnd = ::FindWindow(nullptr, ncstreamer::kNcStreamerWindowTitle);
  return (wnd != NULL);
}


void NcStreamerRemote::KeepConnected() {
  if (remote_connection_.lock()) {
    return;
  }

  Connect([this](
      ErrorCategory err_category,
      int err_code,
      const std::wstring &err_msg) {
    timer_to_keep_connected_.expires_from_now(Chrono::seconds{1});
    timer_to_keep_connected_.async_wait([this](
        const boost::system::error_code &ec) {
      if (ec) {
        return;
      }
      KeepConnected();
    });
  }, [this]() {
    if (connect_handler_) {
      connect_handler_();
    }
  });
}


void NcStreamerRemote::Connect(
    const OpenHandler &open_handler) {
  Connect(current_error_handler_, open_handler);
}


void NcStreamerRemote::Connect(
    const ErrorHandler &error_handler,
    const OpenHandler &open_handler) {
  if (ExistsNcStreamer() == false) {
    HandleError(Error::Connection::kNoNcStreamer, error_handler);
    return;
  }

  websocketpp::lib::error_code ec;
  auto connection = remote_.get_connection(remote_uri_, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteConnect, ec, error_handler);
    return;
  }

  remote_.connect(connection);
  connection->set_open_handler([this, open_handler](
      websocketpp::connection_hdl connection) {
    remote_connection_ = connection;
    open_handler();
  });
}


void NcStreamerRemote::SendStatusRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingStatusRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendStartRequest(const std::wstring &title) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingStartRequest));
    tree.put("title", converter.to_bytes(title));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendStopRequest(const std::wstring &title) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingStopRequest));
    tree.put("title", converter.to_bytes(title));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendQualityUpdateRequest(const std::wstring &quality) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsQualityUpdateRequest));
    tree.put("quality", converter.to_bytes(quality));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendExitRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kNcStreamerExitRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendCommentsRequest(const std::wstring &created_time) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingCommentsRequest));
    tree.put("createdTime", converter.to_bytes(created_time));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendViewersRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingViewersRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendWebcamSearchRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsWebcamSearchRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendWebcamOnRequest(
    const std::wstring &device_id,
    const float &normal_width,
    const float &normal_height,
    const float &normal_x,
    const float &normal_y) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsWebcamOnRequest));
    tree.put("device_id", converter.to_bytes(device_id));
    tree.put("normal_width", normal_width);
    tree.put("normal_height", normal_height);
    tree.put("normal_x", normal_x);
    tree.put("normal_y", normal_y);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendWebcamOffRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsWebcamOffRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendWebcamSizeRequest(
    const float &normal_width,
    const float &normal_height) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsWebcamSizeRequest));
    tree.put("normal_width", normal_width);
    tree.put("normal_height", normal_height);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendWebcamPositionRequest(
    const float &normal_x,
    const float &normal_y) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsWebcamPositionRequest));
    tree.put("normal_x", normal_x);
    tree.put("normal_y", normal_y);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendChromaKeyOnRequest(
    const uint32_t &color,
    const int &similarity) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsChromaKeyOnRequest));
    tree.put("color", color);
    tree.put("similarity", similarity);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendChromaKeyOffRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsChromaKeyOffRequest));

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendChromaKeyColorRequest(const uint32_t &color) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsChromaKeyColorRequest));
    tree.put("color", color);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendChromaKeySimilarityRequest(const int &similarity) {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsChromaKeySimilarityRequest));
    tree.put("similarity", similarity);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendMicSearchRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kSettingsMicSearchRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendMicOnRequest(
    const std::wstring &device_id,
    const float &volume) {
  std::stringstream msg;
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsMicOnRequest));
    tree.put("device_id", converter.to_bytes(device_id));
    tree.put("volume", volume);

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::SendMicOffRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::
            kSettingsMicOffRequest));

    boost::property_tree::write_json(msg, tree, false);
  }

  websocketpp::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    HandleError(Error::Connection::kRemoteSend, ec);
    return;
  }
}


void NcStreamerRemote::OnRemoteFail(websocketpp::connection_hdl connection) {
  HandleDisconnect(Error::Connection::kOnRemoteFail);
}


void NcStreamerRemote::OnRemoteClose(websocketpp::connection_hdl connection) {
  HandleDisconnect(Error::Connection::kOnRemoteClose);
}


void NcStreamerRemote::OnRemoteMessage(
    websocketpp::connection_hdl connection,
    websocketpp::connection<AsioClient>::message_ptr msg) {
  busy_ = false;

  boost::property_tree::ptree response;
  ncstreamer::RemoteMessage::MessageType msg_type{
      ncstreamer::RemoteMessage::MessageType::kUndefined};

  std::stringstream ss{msg->get_payload()};
  try {
    boost::property_tree::read_json(ss, response);
    msg_type = static_cast<ncstreamer::RemoteMessage::MessageType>(
        response.get<int>("type"));
  } catch (const std::exception &/*e*/) {
    msg_type = ncstreamer::RemoteMessage::MessageType::kUndefined;
  }

  using MessageHandler = std::function<void(
      const boost::property_tree::ptree &/*response*/)>;
  static const std::unordered_map<ncstreamer::RemoteMessage::MessageType,
                                  MessageHandler> kMessageHandlers{
      {ncstreamer::RemoteMessage::MessageType::kStreamingStartEvent,
       std::bind(&NcStreamerRemote::OnRemoteStartEvent,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingStopEvent,
       std::bind(&NcStreamerRemote::OnRemoteStopEvent,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingStatusResponse,
       std::bind(&NcStreamerRemote::OnRemoteStatusResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingStartResponse,
       std::bind(&NcStreamerRemote::OnRemoteStartResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingStopResponse,
       std::bind(&NcStreamerRemote::OnRemoteStopResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsQualityUpdateResponse,
       std::bind(&NcStreamerRemote::OnRemoteQualityUpdateResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingCommentsResponse,
       std::bind(&NcStreamerRemote::OnRemoteCommentsResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kStreamingViewersResponse,
       std::bind(&NcStreamerRemote::OnRemoteViewersResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsWebcamSearchResponse,
       std::bind(&NcStreamerRemote::OnRemoteWebcamSearchResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsWebcamOnResponse,
       std::bind(&NcStreamerRemote::OnRemoteWebcamOnResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsWebcamOffResponse,
       std::bind(&NcStreamerRemote::OnRemoteWebcamOffResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsWebcamSizeResponse,
       std::bind(&NcStreamerRemote::OnRemoteWebcamSizeResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsWebcamPositionResponse,
       std::bind(&NcStreamerRemote::OnRemoteWebcamPositionResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsChromaKeyOnResponse,
       std::bind(&NcStreamerRemote::OnRemoteChromaKeyOnResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsChromaKeyOffResponse,
       std::bind(&NcStreamerRemote::OnRemoteChromaKeyOffResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsChromaKeyColorResponse,
       std::bind(&NcStreamerRemote::OnRemoteChromaKeyColorResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsMicSearchResponse,
       std::bind(&NcStreamerRemote::OnRemoteMicSearchResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsMicOnResponse,
       std::bind(&NcStreamerRemote::OnRemoteMicOnResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::kSettingsMicOffResponse,
       std::bind(&NcStreamerRemote::OnRemoteMicOffResponse,
           this, std::placeholders::_1)},
      {ncstreamer::RemoteMessage::MessageType::
          kSettingsChromaKeySimilarityResponse,
       std::bind(&NcStreamerRemote::OnRemoteChromaKeySimilarityResponse,
           this, std::placeholders::_1)}};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    LogWarning(
        "unknown message type: " + std::to_string(static_cast<int>(msg_type)));
    return;
  }
  i->second(response);
}


void NcStreamerRemote::OnRemoteStartEvent(
    const boost::property_tree::ptree &evt) {
  if (!start_event_handler_) {
    return;
  }

  std::string source{};
  std::string user_page{};
  std::string privacy{};
  std::string description{};
  std::string mic{};
  std::string service_provider{};
  std::string stream_url{};
  std::string post_url{};
  try {
    source = evt.get<std::string>("source");
    user_page = evt.get<std::string>("userPage");
    privacy = evt.get<std::string>("privacy");
    description = evt.get<std::string>("description");
    mic = evt.get<std::string>("mic");
    service_provider = evt.get<std::string>("serviceProvider");
    stream_url = evt.get<std::string>("streamUrl");
    post_url = evt.get<std::string>("postUrl");
  } catch (const std::exception &/*e*/) {
    source.clear();
    user_page.clear();
    privacy.clear();
    description.clear();
    mic.clear();
    service_provider.clear();
    stream_url.clear();
    post_url.clear();
  }

  if (source.empty() == true) {
    LogError("source.empty()");
    return;
  }

  std::vector<std::string> tokens{};
  boost::split(tokens, source, boost::is_any_of(":"));
  const std::string &source_title = tokens.at(0);

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  start_event_handler_(
      converter.from_bytes(source_title),
      converter.from_bytes(user_page),
      converter.from_bytes(privacy),
      converter.from_bytes(description),
      converter.from_bytes(mic),
      converter.from_bytes(service_provider),
      converter.from_bytes(stream_url),
      converter.from_bytes(post_url));
}


void NcStreamerRemote::OnRemoteStopEvent(
    const boost::property_tree::ptree &evt) {
  if (!stop_event_handler_) {
    return;
  }

  std::string source{};
  try {
    source = evt.get<std::string>("source");
  } catch (const std::exception &/*e*/) {
    source.clear();
  }

  if (source.empty() == true) {
    LogError("source.empty()");
    return;
  }

  std::vector<std::string> tokens{};
  boost::algorithm::split(tokens, source, boost::is_any_of(":"));
  const std::string &source_title = tokens.at(0);

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  stop_event_handler_(
      converter.from_bytes(source_title));
}


void NcStreamerRemote::OnRemoteStatusResponse(
    const boost::property_tree::ptree &response) {
  std::string status{};
  std::string source_title{};
  std::string user_name{};
  std::string quality{};
  try {
    status = response.get<std::string>("status");
    source_title = response.get<std::string>("sourceTitle");
    user_name = response.get<std::string>("userName");
    quality = response.get<std::string>("quality");
  } catch (const std::exception &/*e*/) {
    status.clear();
    source_title.clear();
  }

  if (status.empty() == true) {
    LogError("status.empty()");
    return;
  }
  if (quality.empty() == true) {
    LogError("quality.empty()");
    return;
  }

  if (!current_status_response_handler_) {
    LogError("!current_status_response_handler_");
    return;
  }

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  current_status_response_handler_(
      converter.from_bytes(status),
      converter.from_bytes(source_title),
      converter.from_bytes(user_name),
      converter.from_bytes(quality));
}


void NcStreamerRemote::OnRemoteStartResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("start response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToStartError(error);

    current_error_handler_(
        ErrorCategory::kStart,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_start_response_handler_(true);
  }
}


void NcStreamerRemote::OnRemoteStopResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("stop response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToStopError(error);

    current_error_handler_(
        ErrorCategory::kStop,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_stop_response_handler_(true);
  }
}


void NcStreamerRemote::OnRemoteQualityUpdateResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("stop response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    current_error_handler_(
        ErrorCategory::kMisc, 0, converter.from_bytes(error));
  } else {
    current_quality_update_response_handler_(true);
  }
}


void NcStreamerRemote::OnRemoteCommentsResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  std::string chat_message{};
  try {
    error = response.get<std::string>("error");
    chat_message = response.get<std::string>("comments");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("comments response broken");
    return;
  }

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  if (error.empty() == false) {
    const auto &err_info = ErrorConverter::ToCommentsError(error);
    current_error_handler_(
        ErrorCategory::kComments,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
    } else {
      current_comments_response_handler_(converter.from_bytes(chat_message));
    }
}


void NcStreamerRemote::OnRemoteViewersResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  std::string viewers_message{};
  try {
    error = response.get<std::string>("error");
    viewers_message = response.get<std::string>("viewers");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("viewers response broken");
    return;
  }

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  if (error.empty() == false) {
    const auto &err_info = ErrorConverter::ToViewersError(error);
    current_error_handler_(
        ErrorCategory::kViewers,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_viewers_response_handler_(
        converter.from_bytes(viewers_message));
  }
}



void NcStreamerRemote::OnRemoteWebcamSearchResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  std::vector<std::wstring> webcams;
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
    const auto &webcam_list = response.get_child("webcamList");
    for (const auto &webcam : webcam_list) {
      const std::string &id = webcam.second.get<std::string>("id");
      std::wstring device{converter.from_bytes(id)};
      webcams.emplace_back(device);
    }
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("webcam search response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToWebcamError(error);
    current_error_handler_(
        ErrorCategory::kWebcam,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_webcam_search_response_handler_(webcams);
  }
}


void NcStreamerRemote::OnRemoteWebcamOnResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("webcam on response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToWebcamError(error);
    current_error_handler_(
        ErrorCategory::kWebcam,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_webcam_on_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteWebcamOffResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("webcam off response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToWebcamError(error);
    current_error_handler_(
        ErrorCategory::kWebcam,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_webcam_off_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteWebcamSizeResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("webcam size response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToWebcamError(error);
    current_error_handler_(
        ErrorCategory::kWebcam,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_webcam_size_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteWebcamPositionResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("webcam position response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToWebcamError(error);
    current_error_handler_(
        ErrorCategory::kWebcam,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_webcam_position_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteChromaKeyOnResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("chroma key on response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToChromaKeyError(error);
    current_error_handler_(
        ErrorCategory::kChromaKey,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_chroma_key_on_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteChromaKeyOffResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("chroma key off response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToChromaKeyError(error);
    current_error_handler_(
        ErrorCategory::kChromaKey,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_chroma_key_off_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteChromaKeyColorResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("chroma key color response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToChromaKeyError(error);
    current_error_handler_(
        ErrorCategory::kChromaKey,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_chroma_key_on_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteChromaKeySimilarityResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("chroma key similarity response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToChromaKeyError(error);
    current_error_handler_(
        ErrorCategory::kChromaKey,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_chroma_key_on_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteMicSearchResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  std::vector<std::wstring> mic_devices;
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
    const auto &mic_list = response.get_child("micList");
    for (const auto &mic : mic_list) {
      const std::string &id = mic.second.get<std::string>("id");
      const std::string &name = mic.second.get<std::string>("name");
      std::wstring device{
          converter.from_bytes(id) + L":" + converter.from_bytes(name)};
      mic_devices.emplace_back(device);
    }
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("mic search response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToMicError(error);
    current_error_handler_(
        ErrorCategory::kMic,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_mic_search_response_handler_(mic_devices);
  }
}


void NcStreamerRemote::OnRemoteMicOnResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("mic on response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToMicError(error);
    current_error_handler_(
        ErrorCategory::kMic,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_mic_on_response_handler_();
  }
}


void NcStreamerRemote::OnRemoteMicOffResponse(
    const boost::property_tree::ptree &response) {
  bool exception_occurred{false};
  std::string error{};
  try {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    error = response.get<std::string>("error");
  } catch (const std::exception &/*e*/) {
    exception_occurred = true;
  }

  if (exception_occurred == true) {
    LogError("mic off response broken");
    return;
  }

  if (error.empty() == false) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const auto &err_info = ErrorConverter::ToMicError(error);
    current_error_handler_(
        ErrorCategory::kMic,
        static_cast<int>(err_info.first),
        converter.from_bytes(err_info.second));
  } else {
    current_mic_off_response_handler_();
  }
}


void NcStreamerRemote::HandleDisconnect(
    Error::Connection err_code) {
  remote_connection_.reset();
  busy_ = false;
  LogWarning(ErrorConverter::ToConnectionError(err_code));

  if (disconnect_handler_) {
    disconnect_handler_();
  }

  KeepConnected();
}


void NcStreamerRemote::HandleConnectionError(
    Error::Connection err_code,
    const ErrorHandler &err_handler) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  err_handler(
      ErrorCategory::kConnection,
      static_cast<int>(err_code),
      converter.from_bytes(ErrorConverter::ToConnectionError(err_code)));
}


void NcStreamerRemote::HandleError(
    Error::Connection err_code,
    const websocketpp::lib::error_code &ec) {
  HandleError(err_code, ec, current_error_handler_);
}


void NcStreamerRemote::HandleError(
    Error::Connection err_code,
    const websocketpp::lib::error_code &ec,
    const ErrorHandler &err_handler) {
  const auto &err_msg = ErrorConverter::ToConnectionError(err_code);
  std::stringstream ss;
  ss << err_msg << ": " << ec.message();
  HandleError(err_code, ss.str(), err_handler);
}


void NcStreamerRemote::HandleError(
    Error::Connection err_code) {
  const auto &err_msg = ErrorConverter::ToConnectionError(err_code);
  HandleError(err_code, err_msg, current_error_handler_);
}


void NcStreamerRemote::HandleError(
    Error::Connection err_code,
    const std::string &err_msg,
    const ErrorHandler &err_handler) {
  busy_ = false;
  LogError(err_msg);

  if (err_handler) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    err_handler(
        ErrorCategory::kConnection,
        static_cast<int>(err_code),
        converter.from_bytes(err_msg));
  }
}


void NcStreamerRemote::HandleError(
    Error::Connection err_code,
    const ErrorHandler &err_handler) {
  const auto &err_msg = ErrorConverter::ToConnectionError(err_code);
  HandleError(err_code, err_msg, err_handler);
}


void NcStreamerRemote::LogWarning(const std::string &warn_msg) {
  remote_.get_elog().write(websocketpp::log::elevel::warn, warn_msg);
}


void NcStreamerRemote::LogError(const std::string &err_msg) {
  remote_.get_elog().write(websocketpp::log::elevel::rerror, err_msg);
}


NcStreamerRemote *NcStreamerRemote::static_instance{nullptr};
}  // namespace ncstreamer_remote
