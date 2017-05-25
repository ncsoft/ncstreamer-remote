/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_
#define NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_


namespace ncstreamer {
class RemoteMessage {
 public:
  enum class MessageType {
    kUndefined = 0,
    kStreamingStatusRequest,
    kStreamingStatusResponse,
    kStreamingStartRequest,
    kStreamingStartResponse,
    kStreamingStopRequest,
    kStreamingStopResponse,
    kNcStreamerExitRequest,
    kNcStreamerExitResponse,  // not used.
  };
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_
