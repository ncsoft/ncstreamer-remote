/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_dll/src/error/error_converter.h"

#include <unordered_map>

#include "ncstreamer_remote_dll/src/ncstreamer_remote_message_types.h"


namespace ncstreamer_remote {
std::string ErrorConverter::ToConnectionError(
    Error::Connection err_code) {
  using DescriptionMap = std::unordered_map<Error::Connection, std::string>;
  static const DescriptionMap kDescriptions{
      {Error::Connection::kNoNcStreamer,
       "NCStreamer is not running"},
      {Error::Connection::kBusy,
       "your previous request is not responded yet"},
      {Error::Connection::kRemoteInitAsio,
       "an error occurred while initializing NCStreamer-Remote module"},
      {Error::Connection::kRemoteConnect,
       "couldn't try to connect to NCStreamer"},
      {Error::Connection::kRemoteSend,
       "couldn't try to send message to NCStreamer"},
      {Error::Connection::kOnRemoteFail,
       "failed to connect to NCStreamer"},
      {Error::Connection::kOnRemoteClose,
       "the connection to NCStreamer is closed"}};

  auto i = kDescriptions.find(err_code);
  if (i == kDescriptions.end()) {
    static const std::string kUnknownCode{"unknown error code"};
    return kUnknownCode;
  }

  return i->second;
}


std::pair<Error::Start, std::string>
    ErrorConverter::ToStartError(const std::string &error) {
  using StartError = std::pair<Error::Start, std::string>;
  static const std::unordered_map<std::string, StartError> kStartErrors{
      {ncstreamer::RemoteMessage::Error::Start::kNoUser,
       {Error::Start::kNoUser,
        "you are not logged in to any streaming service provider"}},
      {ncstreamer::RemoteMessage::Error::Start::kNotStandbySelf,
       {Error::Start::kNotStandbySelf,
        "NCStreamer is already on streaming your source"}},
      {ncstreamer::RemoteMessage::Error::Start::kNotStandbyOther,
       {Error::Start::kNotStandbyOther,
        "NCStreamer is already on streaming other's source"}},
      {ncstreamer::RemoteMessage::Error::Start::kUnknownTitle,
       {Error::Start::kUnknownTitle,
        "the source doesn't exist in the available source list"}},
      {ncstreamer::RemoteMessage::Error::Start::kMePageSelectEmpty,
       {Error::Start::kMePageSelectEmpty,
        "select page type where the live video is posted, please"}},
      {ncstreamer::RemoteMessage::Error::Start::kPrivacySelectEmpty,
       {Error::Start::kPrivacySelectEmpty,
        "select privacy to whom the live video is visible, please"}},
      {ncstreamer::RemoteMessage::Error::Start::kOwnPageSelectEmpty,
       {Error::Start::kOwnPageSelectEmpty,
        "select one of your pages where the live video is posted, please"}},
      {ncstreamer::RemoteMessage::Error::Start::kObsInternal,
       {Error::Start::kObsInternal,
        "an error occurred from OBS module"}}};

  auto i = kStartErrors.find(error);
  if (i != kStartErrors.end()) {
    return i->second;
  }

  static const std::string kFacebook{
      ncstreamer::RemoteMessage::Error::Start::kStreamingServiceFacebookLive};
  if (error.substr(0, kFacebook.size()) == kFacebook) {
    return {Error::Start::kStreamingServiceFacebookLive, error};
  }

  return {Error::Start::kUnknownError, error};
}


std::pair<Error::Stop, std::string>
    ErrorConverter::ToStopError(const std::string &error) {
  using StopError = std::pair<Error::Stop, std::string>;
  static const std::unordered_map<std::string, StopError> kStopErrors{
      {ncstreamer::RemoteMessage::Error::Stop::kNotOnAir,
       {Error::Stop::kNotOnAir,
        "NCStreamer is not on streaming anything now"}},
      {ncstreamer::RemoteMessage::Error::Stop::kTitleMismatch,
       {Error::Stop::kTitleMismatch,
        "NCStreamer is not on streaming your source"}}};

  auto i = kStopErrors.find(error);
  if (i != kStopErrors.end()) {
    return i->second;
  }

  return {Error::Stop::kUnknownError, error};
}


std::pair<Error::Comments, std::string>
    ErrorConverter::ToCommentsError(const std::string &error) {
  using CommentsError = std::pair<Error::Comments, std::string>;
  static const std::unordered_map<std::string, CommentsError>
      kCommentsErrors {
          {ncstreamer::RemoteMessage::Error::Comments::kCommentsNotReady,
           {Error::Comments::kCommentsNotReady,
            "Live video comments not ready"}},
          {ncstreamer::RemoteMessage::Error::Comments::kCommentsInternal,
           {Error::Comments::kCommentsInternal,
            "Live video comments internal error"}}};

  auto i = kCommentsErrors.find(error);
  if (i != kCommentsErrors.end()) {
    return i->second;
  }

  return {Error::Comments::kUnknownError, error};
}


std::pair<Error::Viewers, std::string>
    ErrorConverter::ToViewersError(const std::string &error) {
  using ViewersError = std::pair<Error::Viewers, std::string>;
  static const std::unordered_map<std::string, ViewersError> kViewersErrors {
      {ncstreamer::RemoteMessage::Error::Viewers::kViewersInternal,
       {Error::Viewers::kViewersInternal,
        "Get viewers internal error"}}};

  auto i = kViewersErrors.find(error);
  if (i != kViewersErrors.end()) {
    return i->second;
  }

  return {Error::Viewers::kUnknownError, error};
}


std::pair<Error::Webcam, std::string>
    ErrorConverter::ToWebcamError(const std::string &error) {
  using WebcamError = std::pair<Error::Webcam, std::string>;
  static const std::unordered_map<std::string, WebcamError>
      kWebcamError {
          {ncstreamer::RemoteMessage::Error::Webcam::kWebcamSearch,
           {Error::Webcam::kWebcamSearch,
            "an error occurred from OBS internal"}},
          {ncstreamer::RemoteMessage::Error::Webcam::kWebcamOn,
           {Error::Webcam::kWebcamOn,
            "incorrect arguments"}},
          {ncstreamer::RemoteMessage::Error::Webcam::kWebcamOff,
           {Error::Webcam::kWebcamOff,
            "webcam off error"}},
          {ncstreamer::RemoteMessage::Error::Webcam::kWebcamSize,
           {Error::Webcam::kWebcamSize,
            "webcam size error"}},
          {ncstreamer::RemoteMessage::Error::Webcam::kWebcamPosition,
           {Error::Webcam::kWebcamPosition,
            "webcam position error"}}};

  auto i = kWebcamError.find(error);
  if (i != kWebcamError.end()) {
    return i->second;
  }

  return {Error::Webcam::kUnknownError, error};
}


std::pair<Error::ChromaKey, std::string>
    ErrorConverter::ToChromaKeyError(const std::string &error) {
  using ChromaKeyError = std::pair<Error::ChromaKey, std::string>;
  static const std::unordered_map<std::string, ChromaKeyError>
      kChromaKeyError{
          {ncstreamer::RemoteMessage::Error::ChromaKey::kChromaKeyOn,
           {Error::ChromaKey::kChromaKeyOn,
            "chroma key on error"}},
          {ncstreamer::RemoteMessage::Error::ChromaKey::kChromaKeyOff,
           {Error::ChromaKey::kChromaKeyOff,
            "chroma key off error"}},
          {ncstreamer::RemoteMessage::Error::ChromaKey::kChromaKeyColor,
           {Error::ChromaKey::kChromaKeyColor,
            "chroma key color error"}},
          {ncstreamer::RemoteMessage::Error::ChromaKey::kChromaKeySimilarity,
           {Error::ChromaKey::kChromaKeySimilarity,
            "chroma key on error"}}};

  auto i = kChromaKeyError.find(error);
  if (i != kChromaKeyError.end()) {
    return i->second;
  }

  return {Error::ChromaKey::kUnknownError, error};
}


std::pair<Error::Mic, std::string>
    ErrorConverter::ToMicError(const std::string &error) {
  using MicError = std::pair<Error::Mic, std::string>;
  static const std::unordered_map<std::string, MicError>
      kMicError{
          {ncstreamer::RemoteMessage::Error::Mic::kMicSearch,
           {Error::Mic::kMicSearch,
            "mic search error"}},
          {ncstreamer::RemoteMessage::Error::Mic::kMicOn,
           {Error::Mic::kMicOn,
            "mic on error"}},
          {ncstreamer::RemoteMessage::Error::Mic::kMicOff,
           {Error::Mic::kMicOff,
            "mic off error"}}};

  auto i = kMicError.find(error);
  if (i != kMicError.end()) {
    return i->second;
  }

  return {Error::Mic::kUnknownError, error};
}
}  // namespace ncstreamer_remote
