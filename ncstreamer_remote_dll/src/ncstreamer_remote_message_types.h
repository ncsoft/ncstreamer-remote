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
    kStreamingStartEvent,
    kStreamingStopRequest = 211,
    kStreamingStopResponse,
    kStreamingStopEvent,
    kSettingsQualityUpdateRequest = 301,
    kSettingsQualityUpdateResponse,
    kStreamingCommentsRequest = 401,
    kStreamingCommentsResponse,
    kSettingsWebcamSearchRequest = 501,
    kSettingsWebcamSearchResponse,
    kSettingsWebcamOnRequest = 511,
    kSettingsWebcamOnResponse,
    kSettingsWebcamOffRequest = 521,
    kSettingsWebcamOffResponse,
    kSettingsWebcamSizeRequest = 531,
    kSettingsWebcamSizeResponse,
    kSettingsWebcamPositionRequest = 541,
    kSettingsWebcamPositionResponse,
    kSettingsChromaKeyOnRequest = 601,
    kSettingsChromaKeyOnResponse,
    kSettingsChromaKeyOffRequest = 611,
    kSettingsChromaKeyOffResponse,
    kSettingsChromaKeyColorRequest = 621,
    kSettingsChromaKeyColorResponse,
    kSettingsChromaKeySimilarityRequest = 631,
    kSettingsChromaKeySimilarityResponse,
    kNcStreamerExitRequest = 901,
    kNcStreamerExitResponse,  // not used.
  };

  class Error {
   public:
    class Start {
     public:
      static const char *const kNoUser;
      static const char *const kNotStandbySelf;
      static const char *const kNotStandbyOther;
      static const char *const kUnknownTitle;
      static const char *const kMePageSelectEmpty;
      static const char *const kPrivacySelectEmpty;
      static const char *const kOwnPageSelectEmpty;
      static const char *const kObsInternal;
      static const char *const kStreamingServiceFacebookLive;
    };
    class Stop {
     public:
      static const char *const kNotOnAir;
      static const char *const kTitleMismatch;
    };
    class Comments {
     public:
      static const char *const kCommentsNotReady;
      static const char *const kCommentsInternal;
    };
    class Webcam {
     public:
      static const char *const kWebcamSearch;
      static const char *const kWebcamOn;
      static const char *const kWebcamOff;
      static const char *const kWebcamSize;
      static const char *const kWebcamPosition;
    };
    class ChromaKey {
     public:
      static const char *const kChromaKeyOn;
      static const char *const kChromaKeyOff;
      static const char *const kChromaKeyColor;
      static const char *const kChromaKeySimilarity;
    };
  };
};
}  // namespace ncstreamer


#endif  // NCSTREAMER_REMOTE_DLL_SRC_NCSTREAMER_REMOTE_MESSAGE_TYPES_H_
