/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_ERROR_ERROR_TYPES_H_
#define NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_ERROR_ERROR_TYPES_H_


namespace ncstreamer_remote {
enum class ErrorCategory {
  kNoCategory = 0,
  kUnknown,
  kMisc,
  kConnection = 101,
  kStart,
  kStop,
  kComments,
  kWebcam,
  kChromaKey,
};


class Error {
 public:
  enum class Connection {
    kNoError = 0,
    kUnknownError,

    /// "NCStreamer is not running"
    kNoNcStreamer = 101,

    /// "your previous request is not responded yet"
    kBusy,

    /// "an error occurred while initializing NCStreamer-Remote module"
    kRemoteInitAsio,

    /// "couldn't try to connect to NCStreamer"
    kRemoteConnect,

    /// "couldn't try to send message to NCStreamer"
    kRemoteSend,

    /// "failed to connect to NCStreamer"
    kOnRemoteFail,

    /// "the connection to NCStreamer is closed"
    kOnRemoteClose,
  };

  enum class Start {
    kNoError = 0,
    kUnknownError,

    /// "you are not logged in to any streaming service provider"
    kNoUser = 101,

    /// "NCStreamer is already on streaming your source"
    kNotStandbySelf,

    /// "NCStreamer is already on streaming other's source"
    kNotStandbyOther,

    /// "the source doesn't exist in the available source list"
    kUnknownTitle,

    /// "select page type where the live video is posted, please"
    kMePageSelectEmpty,

    /// "select privacy to whom the live video is visible, please"
    kPrivacySelectEmpty,

    /// "select one of your pages where the live video is posted, please"
    kOwnPageSelectEmpty,

    /// "an error occurred from OBS module"
    kObsInternal,

    /// starts with "StreamingService[Facebook Live]"
    kStreamingServiceFacebookLive,
  };

  enum class Stop {
    kNoError = 0,
    kUnknownError,

    /// "NCStreamer is not on streaming anything now"
    kNotOnAir = 101,

    /// "NCStreamer is not on streaming your source"
    kTitleMismatch,
  };

  enum class Comments {
    kNoError = 0,
    kUnknownError,

    /// "Live video comments not ready"
    kCommentsNotReady = 101,

    /// "Live video comments internal error"
    kCommentsInternal,
  };

  enum class Webcam {
    kNoError = 0,
    kUnknownError,

    /// "an error occurred from OBS internal"
    kWebcamSearch = 101,

    /// "webcam on error"
    kWebcamOn,

    /// "webcam off error"
    kWebcamOff,

    /// "webcam size error"
    kWebcamSize,

    /// "webcam position error"
    kWebcamPosition,
  };

  enum class ChromaKey {
    kNoError = 0,
    kUnknownError,

    /// "chroma key on error"
    kChromaKeyOn = 101,

    /// "chroma key off error"
    kChromaKeyOff,

    /// "chroma key color error"
    kChromaKeyColor,

    /// "chroma key similarity error"
    kChromaKeySimilarity,
  };
};
}  // namespace ncstreamer_remote


#endif  // NCSTREAMER_REMOTE_DLL_INCLUDE_NCSTREAMER_REMOTE_ERROR_ERROR_TYPES_H_
