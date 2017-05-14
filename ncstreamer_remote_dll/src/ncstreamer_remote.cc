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
    : io_service_{},
      io_service_work_{io_service_},
      remote_{},
      remote_threads_{},
      remote_log_{},
      remote_connection_{},
      current_error_handler_{} {
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

  ws::uri_ptr remote_uri{new ws::uri{false, "localhost", remote_port, ""}};
  ws::lib::error_code conn_ec;
  auto connection = remote_.get_connection(
      remote_uri, conn_ec);
  if (conn_ec) {
    // TODO(khpark): log error.
    return;
  }

  remote_.connect(connection);

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
  auto msg_type{ncstreamer::RemoteMessage::MessageType::kUndefined};

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
                                  MessageHandler> kMessageHandlers{};

  auto i = kMessageHandlers.find(msg_type);
  if (i == kMessageHandlers.end()) {
    // TBD
    assert(false);
    return;
  }
  i->second(response);
}


NcStreamerRemote *NcStreamerRemote::static_instance{nullptr};
}  // namespace ncstreamer_remote
