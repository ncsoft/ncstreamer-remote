/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#ifndef NCSTREAMER_REMOTE_REFERENCE_SRC_WINDOWS_MESSAGE_HANDLER_H_
#define NCSTREAMER_REMOTE_REFERENCE_SRC_WINDOWS_MESSAGE_HANDLER_H_


#include "Windows.h"  // NOLINT


namespace ncstreamer_remote_reference {
INT_PTR CALLBACK MainDialogProc(
    HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam);
}  // namespace ncstreamer_remote_reference


#endif  // NCSTREAMER_REMOTE_REFERENCE_SRC_WINDOWS_MESSAGE_HANDLER_H_
