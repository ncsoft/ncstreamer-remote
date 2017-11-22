/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_dll/src/ncstreamer_remote_message_types.h"


namespace ncstreamer {
const char *const RemoteMessage::Error::
    Start::kNoUser{"no user"};

const char *const RemoteMessage::Error::
    Start::kNotStandbySelf{"not standby: self"};

const char *const RemoteMessage::Error::
    Start::kNotStandbyOther{"not standby: other"};

const char *const RemoteMessage::Error::
    Start::kUnknownTitle{"unknown title"};

const char *const RemoteMessage::Error::
    Start::kMePageSelectEmpty{"mePage select empty"};

const char *const RemoteMessage::Error::
    Start::kPrivacySelectEmpty{"privacy select empty"};

const char *const RemoteMessage::Error::
    Start::kOwnPageSelectEmpty{"ownPage select empty"};

const char *const RemoteMessage::Error::
    Start::kObsInternal{"obs internal"};

const char *const RemoteMessage::Error::
    Start::kStreamingServiceFacebookLive{"StreamingService[Facebook Live]"};


const char *const RemoteMessage::Error::
    Stop::kNotOnAir{"not onAir"};

const char *const RemoteMessage::Error::
    Stop::kTitleMismatch{"title mismatch"};


const char *const RemoteMessage::Error::
    Comments::kCommentsInternal{"comments error"};
}  // namespace ncstreamer
