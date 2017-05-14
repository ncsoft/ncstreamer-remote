/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_reference/src/windows_message_handler.h"

#include <chrono>  // NOLINT
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

#include "ncstreamer_remote_dll/src/ncstreamer_remote.h"
#include "ncstreamer_remote_reference/src_generated/resource.h"


namespace {
HWND static_main_dialog{NULL};
HWND static_clock_panel{NULL};
HWND static_status_button{NULL};
HWND static_start_button{NULL};
HWND static_stop_button{NULL};
HWND static_message_panel{NULL};

HFONT static_clock_font{NULL};


HFONT SetUpFont(HWND wnd, int font_size) {
  HFONT font = ::CreateFont(
      font_size,
      0,
      0,
      0,
      FW_NORMAL,
      FALSE,
      FALSE,
      FALSE,
      DEFAULT_CHARSET,
      OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS,
      DEFAULT_QUALITY,
      DEFAULT_PITCH | FF_DONTCARE,
      NULL);

  ::SendMessage(wnd, WM_SETFONT, (WPARAM) font, NULL);
  return font;
}


std::wstring GetCurrentLocalTime() {
  auto current_tp = std::chrono::system_clock::now();
  time_t current_tt = std::chrono::system_clock::to_time_t(current_tp);
  tm *current_tm = std::localtime(&current_tt);
  auto current_millisec = std::chrono::duration_cast<std::chrono::milliseconds>(
      current_tp.time_since_epoch()).count() % 1000;

  std::wstringstream ss;
  ss << std::put_time(current_tm, L"%F %T")
     << L"." << std::setfill(L'0') << std::setw(3) << current_millisec;

  return ss.str();
}


void OnStatusButton() {
}


void OnStartButton() {
}


void OnStopButton() {
}


INT_PTR OnCommand(WPARAM wparam, LPARAM /*lparam*/) {
  switch (LOWORD(wparam)) {
    case IDC_BUTTON_STATUS: OnStatusButton(); return TRUE;
    case IDC_BUTTON_START: OnStartButton(); return TRUE;
    case IDC_BUTTON_STOP: OnStopButton(); return TRUE;
    default: break;
  }
  return FALSE;
}
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
  static_clock_panel = ::GetDlgItem(dlg, IDC_STATIC_CLOCK);
  static_status_button = ::GetDlgItem(dlg, IDC_BUTTON_STATUS);
  static_start_button = ::GetDlgItem(dlg, IDC_BUTTON_START);
  static_stop_button = ::GetDlgItem(dlg, IDC_BUTTON_STOP);
  static_message_panel = ::GetDlgItem(dlg, IDC_EDIT_MESSAGE);

  static_clock_font = SetUpFont(static_clock_panel, 36);

  ::SetTimer(static_clock_panel, 0, 50 /*milliseconds*/, [](
      HWND wnd, UINT /*msg*/, UINT_PTR /*event_id*/, DWORD /*tick*/) {
    std::wstring tm = GetCurrentLocalTime();
    ::SetWindowText(wnd, tm.c_str());
  });

  ncstreamer_remote::NcStreamerRemote::SetUpDefault();

  ::ShowWindow(dlg, cmd_show);
  return dlg;
}


void DeleteMainDialog() {
  ncstreamer_remote::NcStreamerRemote::ShutDown();

  ::DeleteObject(static_clock_font);

  ::DestroyWindow(static_main_dialog);
}


INT_PTR CALLBACK MainDialogProc(
    HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_COMMAND: return OnCommand(wparam, lparam);
    case WM_CLOSE: DeleteMainDialog(); return TRUE;
    case WM_DESTROY: ::PostQuitMessage(/*exit code*/ 0); return TRUE;
    default: break;
  }

  return FALSE;
}
}  // namespace ncstreamer_remote_reference
