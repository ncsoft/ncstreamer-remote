/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_reference/src/windows_message_handler.h"

#include "ncstreamer_remote_reference/src_generated/resource.h"


namespace {
HWND static_main_dialog{NULL};
}  // unnamed namespace


namespace ncstreamer_remote_reference {
HWND CreateMainDialog(
    HINSTANCE instance, int cmd_show) {
  HWND dlg = ::CreateDialogParam(
      instance,
      MAKEINTRESOURCE(IDD_DIALOG1),
      NULL,
      MainDialogProc,
      NULL);

  HICON icon = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
  ::SendMessage(dlg, WM_SETICON, ICON_SMALL, (LPARAM) icon);

  static_main_dialog = dlg;

  ::ShowWindow(dlg, cmd_show);
  return dlg;
}


void DeleteMainDialog() {
  ::DestroyWindow(static_main_dialog);
}


INT_PTR CALLBACK MainDialogProc(
    HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_CLOSE: DeleteMainDialog(); return TRUE;
    case WM_DESTROY: ::PostQuitMessage(/*exit code*/ 0); return TRUE;
    default: break;
  }

  return FALSE;
}
}  // namespace ncstreamer_remote_reference
