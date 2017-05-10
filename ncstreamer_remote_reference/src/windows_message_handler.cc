/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_reference/src/windows_message_handler.h"


namespace ncstreamer_remote_reference {
INT_PTR CALLBACK MainDialogProc(
    HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_CLOSE: ::DestroyWindow(dlg); return TRUE;
    case WM_DESTROY: ::PostQuitMessage(/*exit code*/ 0); return TRUE;
    default: break;
  }

  return FALSE;
}
}  // namespace ncstreamer_remote_reference
