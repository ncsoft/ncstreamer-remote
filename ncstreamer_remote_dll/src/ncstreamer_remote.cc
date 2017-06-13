/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_dll/src/ncstreamer_remote.h"

#include <cassert>
#include <codecvt>
#include <locale>
#include <sstream>
#include <unordered_map>

#include "boost/property_tree/json_parser.hpp"

#include "Windows.h"  // NOLINT

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
  SetUp(9002);
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


void NcStreamerRemote::RequestStatus(
    const ErrorHandler &error_handler,
    const StatusResponseHandler &status_response_handler) {
  if (busy_ == true) {
    error_handler(L"busy");
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
    error_handler(L"busy");
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
    error_handler(L"busy");
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
    error_handler(L"busy");
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
    error_handler(L"busy");
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


NcStreamerRemote::NcStreamerRemote(uint16_t remote_port)
    : remote_uri_{new websocketpp::uri{false, "localhost", remote_port, ""}},
      io_service_{},
      io_service_work_{io_service_},
      remote_{},
      remote_threads_{},
      remote_log_{},
      remote_connection_{},
      busy_{},
      current_error_handler_{},
      current_status_response_handler_{},
      current_start_response_handler_{},
      current_stop_response_handler_{},
      current_quality_update_response_handler_{} {
  busy_ = false;

  remote_log_.open("ncstreamer_remote.log");
  remote_.set_access_channels(websocketpp::log::alevel::all);
  remote_.set_access_channels(websocketpp::log::elevel::all);
  remote_.get_alog().set_ostream(&remote_log_);
  remote_.get_elog().set_ostream(&remote_log_);

  websocketpp::lib::error_code ec;
  remote_.init_asio(&io_service_, ec);
  if (ec) {
    HandleError("remote init_asio", ec);
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


void NcStreamerRemote::Connect(
    const ConnectHandler &connect_handler) {
  Connect(current_error_handler_, connect_handler);
}


void NcStreamerRemote::Connect(
    const ErrorHandler &error_handler,
    const ConnectHandler &connect_handler) {
  if (ExistsNcStreamer() == false) {
    HandleError("no ncstreamer", error_handler);
    return;
  }

  websocketpp::lib::error_code ec;
  auto connection = remote_.get_connection(remote_uri_, ec);
  if (ec) {
    HandleError("remote connect", ec, error_handler);
    return;
  }

  remote_.connect(connection);
  connection->set_open_handler([this, connect_handler](
      websocketpp::connection_hdl connection) {
    remote_connection_ = connection;
    connect_handler();
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
    HandleError("remote send", ec);
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
    HandleError("remote send", ec);
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
    HandleError("remote send", ec);
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
    HandleError("remote send", ec);
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
    HandleError("remote send", ec);
    return;
  }
}


void NcStreamerRemote::OnRemoteFail(websocketpp::connection_hdl connection) {
  HandleDisconnect("on remote fail");
}


void NcStreamerRemote::OnRemoteClose(websocketpp::connection_hdl connection) {
  HandleDisconnect("on remote close");
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
           this, std::placeholders::_1)}};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    LogWarning(
        "unknown message type: " + std::to_string(static_cast<int>(msg_type)));
    return;
  }
  i->second(response);
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
    current_error_handler_(converter.from_bytes(error));
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
    current_error_handler_(converter.from_bytes(error));
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
    current_error_handler_(converter.from_bytes(error));
  } else {
    current_quality_update_response_handler_(true);
  }
}


void NcStreamerRemote::HandleDisconnect(
    const std::string &disconnect_type) {
  remote_connection_.reset();
  busy_ = false;
  LogWarning(disconnect_type);

  if (current_error_handler_) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    current_error_handler_(converter.from_bytes(disconnect_type));
  }
}


void NcStreamerRemote::HandleError(
    const std::string &err_type,
    const websocketpp::lib::error_code &ec) {
  HandleError(err_type, ec, current_error_handler_);
}


void NcStreamerRemote::HandleError(
    const std::string &err_type,
    const websocketpp::lib::error_code &ec,
    const ErrorHandler &err_handler) {
  std::stringstream ss;
  ss << err_type << ": " << ec.message();
  HandleError(ss.str(), err_handler);
}


void NcStreamerRemote::HandleError(
    const std::string &err_msg) {
  HandleError(err_msg, current_error_handler_);
}


void NcStreamerRemote::HandleError(
    const std::string &err_msg,
    const ErrorHandler &err_handler) {
  busy_ = false;
  LogError(err_msg);

  if (err_handler) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    err_handler(converter.from_bytes(err_msg));
  }
}


void NcStreamerRemote::LogWarning(const std::string &warn_msg) {
  remote_.get_elog().write(websocketpp::log::elevel::warn, warn_msg);
}


void NcStreamerRemote::LogError(const std::string &err_msg) {
  remote_.get_elog().write(websocketpp::log::elevel::rerror, err_msg);
}


NcStreamerRemote *NcStreamerRemote::static_instance{nullptr};
}  // namespace ncstreamer_remote
