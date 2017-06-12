/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_
#define NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_


namespace ncstreamer {
static const wchar_t *const kNcStreamerWindowTitle{L"NCStreaming"};


class RemoteMessage {
 public:
  enum class MessageType {
    kUndefined = 0,
    kStreamingStatusRequest = 101,
    kStreamingStatusResponse,
    kStreamingStartRequest = 201,
    kStreamingStartResponse,
    kStreamingStopRequest = 211,
    kStreamingStopResponse,
    kSettingsQualityUpdateRequest = 301,
    kSettingsQualityUpdateResponse,
    kNcStreamerExitRequest = 901,
    kNcStreamerExitResponse,  // not used.
  };
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_
