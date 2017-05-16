/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "Windows.h"  // NOLINT

#include "ncstreamer_remote_reference/src/windows_message_handler.h"


int WINAPI wWinMain(HINSTANCE instance,
                    HINSTANCE /*prev_instance*/,
                    LPTSTR cmd_line,
                    int cmd_show) {
  HWND dlg = ncstreamer_remote_reference::CreateMainDialog(
      instance, cmd_show);

  int result{-1};
  while (true) {
    MSG msg{NULL};
    result = ::GetMessage(&msg, NULL, 0, 0);
    if (result <= 0) {
      break;
    }

    if (::IsDialogMessage(dlg, &msg) == FALSE) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
  }

  return result;
}
