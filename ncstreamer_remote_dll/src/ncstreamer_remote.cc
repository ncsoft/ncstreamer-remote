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


NcStreamerRemote::NcStreamerRemote(uint16_t remote_port)
    : remote_uri_{new ws::uri{false, "localhost", remote_port, ""}},
      io_service_{},
      io_service_work_{io_service_},
      remote_{},
      remote_threads_{},
      remote_log_{},
      remote_connection_{},
      current_error_handler_{},
      current_status_response_handler_{} {
  remote_log_.open("remote_server.log");
  remote_.set_access_channels(ws::log::alevel::all);
  remote_.set_access_channels(ws::log::elevel::all);
  remote_.get_alog().set_ostream(&remote_log_);
  remote_.get_elog().set_ostream(&remote_log_);

  ws::lib::error_code init_ec;
  remote_.init_asio(&io_service_, init_ec);
  if (init_ec) {
    // TODO(khpark): log error.
    assert(false);
    return;
  }

  remote_.set_fail_handler(ws::lib::bind(
      &NcStreamerRemote::OnRemoteFail, this, placeholders::_1));
  remote_.set_open_handler(ws::lib::bind(
      &NcStreamerRemote::OnRemoteOpen, this, placeholders::_1));
  remote_.set_close_handler(ws::lib::bind(
      &NcStreamerRemote::OnRemoteClose, this, placeholders::_1));
  remote_.set_message_handler(ws::lib::bind(
      &NcStreamerRemote::OnRemoteMessage, this,
          placeholders::_1, placeholders::_2));

  Connect();

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


void NcStreamerRemote::Connect() {
  ws::lib::error_code ec;
  auto connection = remote_.get_connection(
    remote_uri_, ec);
  if (ec) {
    // TODO(khpark): log error.
    return;
  }
  remote_.connect(connection);
}


void NcStreamerRemote::OnRemoteFail(ws::connection_hdl connection) {
}


void NcStreamerRemote::OnRemoteOpen(ws::connection_hdl connection) {
  remote_connection_ = connection;
}


void NcStreamerRemote::OnRemoteClose(ws::connection_hdl connection) {
}


void NcStreamerRemote::OnRemoteMessage(
    ws::connection_hdl connection,
    ws::connection<AsioClient>::message_ptr msg) {
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
           this, std::placeholders::_1)}};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    // TBD
    assert(false);
    return;
  }
  i->second(response);
}


void NcStreamerRemote::OnRemoteStatusResponse(
    const boost::property_tree::ptree &response) {
  std::string status{};
  std::string source_title{};
  try {
    status = response.get<std::string>("status");
    source_title = response.get<std::string>("source_title");
  } catch (const std::exception &/*e*/) {
    status.clear();
    source_title.clear();
  }

  if (status.empty() == true) {
    // TODO(khpark): log error.
    return;
  }

  if (!current_status_response_handler_) {
    // TODO(khpark): log error.
    return;
  }

  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  current_status_response_handler_(
      converter.from_bytes(status),
      converter.from_bytes(source_title));
}


void NcStreamerRemote::RequestStatus(
    const ErrorHandler &error_handler,
    const StatusResponseHandler &status_response_handler) {
  current_error_handler_ = error_handler;
  current_status_response_handler_ = status_response_handler;

  if (!remote_connection_.lock()) {
    // TODO(khpark): log error.
    return;
  }

  SendStatusRequest();
}


void NcStreamerRemote::SendStatusRequest() {
  std::stringstream msg;
  {
    boost::property_tree::ptree tree;
    tree.put("type", static_cast<int>(
        ncstreamer::RemoteMessage::MessageType::kStreamingStatusRequest));
    boost::property_tree::write_json(msg, tree, false);
  }

  ws::lib::error_code ec;
  remote_.send(
      remote_connection_, msg.str(), websocketpp::frame::opcode::text, ec);
  if (ec) {
    // TODO(khpark): log error.
    return;
  }
}


NcStreamerRemote *NcStreamerRemote::static_instance{nullptr};
}  // namespace ncstreamer_remote
