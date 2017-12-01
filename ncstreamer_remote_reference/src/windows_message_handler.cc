/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


#include "ncstreamer_remote_reference/src/windows_message_handler.h"

#if _MSC_VER >= 1900
#include <chrono>  // NOLINT
namespace Chrono = std::chrono;
#else
#include "boost/chrono/include.hpp"
namespace Chrono = boost::chrono;
#endif  // _MSC_VER >= 1900

#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "ncstreamer_remote/ncstreamer_remote.h"
#include "ncstreamer_remote_reference/src_generated/resource.h"


namespace {
enum {
  WM_USER__REMOTE_CONNECTED = WM_USER + 1001,
  WM_USER__REMOTE_DISCONNECTED,
  WM_USER__REMOTE_EVENT_START = WM_USER + 1011,
  WM_USER__REMOTE_EVENT_STOP,
  WM_USER__REMOTE_RESPONSE_FAIL = WM_USER + 1234,
  WM_USER__REMOTE_RESPONSE_STATUS,
  WM_USER__REMOTE_RESPONSE_START,
  WM_USER__REMOTE_RESPONSE_STOP,
  WM_USER__REMOTE_RESPONSE_WEBCAM_SEARCH,
  WM_USER__REMOTE_RESPONSE_WEBCAM_ON,
  WM_USER__REMOTE_RESPONSE_WEBCAM_OFF,
  WM_USER__REMOTE_RESPONSE_CHROMA_KEY_ON,
  WM_USER__REMOTE_RESPONSE_CHROMA_KEY_OFF,
};


using StartInfoTuple = std::tuple<
    std::wstring /*source_title*/,
    std::wstring /*user_page*/,
    std::wstring /*privacy*/,
    std::wstring /*description*/,
    std::wstring /*mic*/,
    std::wstring /*service_provider*/,
    std::wstring /*stream_url*/,
    std::wstring /*post_url*/>;


using StatusTuple = std::tuple<
    std::wstring /*status*/,
    std::wstring /*source_title*/,
    std::wstring /*user_name*/,
    std::wstring /*quality*/>;


HWND static_main_dialog{NULL};
HWND static_clock_panel{NULL};
HWND static_status_button{NULL};
HWND static_start_button{NULL};
HWND static_stop_button{NULL};
HWND static_webcam_searach_button{NULL};
HWND static_webcam_on_button{NULL};
HWND static_webcam_off_button{NULL};
HWND static_chroma_key_on_button{NULL};
HWND static_chroma_key_off_button{NULL};
HWND static_message_panel{NULL};

HFONT static_clock_font{NULL};

std::vector<ncstreamer_remote::NcStreamerRemote::WebcamDevice> static_webcams;

void SetMessage(const std::wstring &msg) {
  ::SetWindowText(static_message_panel, msg.c_str());
}


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
  auto current_tp = Chrono::system_clock::now();
  time_t current_tt = Chrono::system_clock::to_time_t(current_tp);
  tm *current_tm = std::localtime(&current_tt);
  auto current_millisec = Chrono::duration_cast<Chrono::milliseconds>(
      current_tp.time_since_epoch()).count() % 1000;

  std::wstringstream ss;
  ss << std::put_time(current_tm, L"%Y-%m-%d %H:%M:%S")
     << L"." << std::setfill(L'0') << std::setw(3) << current_millisec;

  return ss.str();
}


std::wstring GetTitle(HWND wnd) {
  static const int kBufSize{MAX_PATH};
  wchar_t buf[kBufSize];
  ::GetWindowText(wnd, buf, kBufSize);
  return std::wstring{buf};
}


void OnStatusButton() {
  SetMessage(L"OnStatusButton");

  ncstreamer_remote::NcStreamerRemote::Get()->RequestStatus([](
      ncstreamer_remote::ErrorCategory category,
      int err_code,
      const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, [](const std::wstring &status,
        const std::wstring &source_title,
        const std::wstring &user_name,
        const std::wstring &quality) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_STATUS,
        (WPARAM) nullptr,
        (LPARAM) new StatusTuple{
                status, source_title, user_name, quality});
  });
}


void OnStartButton() {
  SetMessage(L"OnStartButton");

  const std::wstring &this_title = GetTitle(static_main_dialog);
  ncstreamer_remote::NcStreamerRemote::Get()->RequestStart(
      this_title, [](ncstreamer_remote::ErrorCategory category,
                     int err_code,
                     const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, [](bool success) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_START,
        (WPARAM) nullptr,
        (LPARAM) new std::tuple<bool>{success});
  });
}


void OnStopButton() {
  SetMessage(L"OnStopButton");

  const std::wstring &this_title = GetTitle(static_main_dialog);
  ncstreamer_remote::NcStreamerRemote::Get()->RequestStop(
      this_title, [](ncstreamer_remote::ErrorCategory category,
                     int err_code,
                     const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, [](bool success) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_STOP,
        (WPARAM) nullptr,
        (LPARAM) new std::tuple<bool>{success});
  });
}


void OnWebcamSearchButton() {
  SetMessage(L"OnWebcamSearchButton");

  ncstreamer_remote::NcStreamerRemote::Get()->RequestWebcamSearch(
      [](ncstreamer_remote::ErrorCategory category,
         int err_code,
         const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, [](const std::vector<ncstreamer_remote::
        NcStreamerRemote::WebcamDevice> webcams) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_WEBCAM_SEARCH,
        (WPARAM) nullptr,
        (LPARAM) new std::vector<ncstreamer_remote::NcStreamerRemote::
            WebcamDevice>{webcams});
  });
}


void OnWebcamOnButton() {
  SetMessage(L"OnWebcamOnButton");

  if (static_webcams.size() == 0) {
    std::wstringstream ss;
    ss << L"error: there is no webcam." << L"\r\n";
    ::SetWindowText(static_message_panel, ss.str().c_str());
    return;
  }
  const std::wstring &id{static_webcams.at(0).id()};
  const int &default_width{static_webcams.at(0).default_width()};
  const int &default_height{static_webcams.at(0).default_height()};
  const float &ratio{
      static_cast<float>(default_height) / static_cast<float>(default_width)};
  const float &normal_width{0.25f};
  const float &normal_height{ratio * normal_width};
  const float &normal_x{1.0f - normal_width};
  const float &normal_y{1.0f - normal_height};

  ncstreamer_remote::NcStreamerRemote::Get()->RequestWebcamOn(
      id, normal_width, normal_height, normal_x, normal_y,
      [](ncstreamer_remote::ErrorCategory category,
         int err_code,
         const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, []() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_WEBCAM_ON,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
}


void OnWebcamOffButton() {
  SetMessage(L"OnWebcamOffButton");

  ncstreamer_remote::NcStreamerRemote::Get()->RequestWebcamOff(
      [](ncstreamer_remote::ErrorCategory category,
         int err_code,
         const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, []() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_WEBCAM_OFF,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
}


void OnChromaKeyOnButton() {
  SetMessage(L"OnChromaKeyOnButton");

  const uint32_t &color{0xFF0000};  // blue
  const int &silmilarity{400};

  ncstreamer_remote::NcStreamerRemote::Get()->RequestChromaKeyOn(
      color, silmilarity,
      [](ncstreamer_remote::ErrorCategory category,
         int err_code,
         const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, []() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_CHROMA_KEY_ON,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
}


void OnChromaKeyOffButton() {
  SetMessage(L"OnChromaKeyOffButton");

  ncstreamer_remote::NcStreamerRemote::Get()->RequestChromaKeyOff(
      [](ncstreamer_remote::ErrorCategory category,
         int err_code,
         const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  }, []() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_CHROMA_KEY_OFF,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
}


void OnExitButton() {
  SetMessage(L"OnExitButton");

  ncstreamer_remote::NcStreamerRemote::Get()->RequestExit([](
      ncstreamer_remote::ErrorCategory category,
      int err_code,
      const std::wstring &err_msg) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_RESPONSE_FAIL,
        (WPARAM) nullptr,
        (LPARAM) new std::wstring{err_msg});
  });
}


void OnRemoteConnected(LPARAM /*lparam*/) {
  SetMessage(L"Remote connected.");
}


void OnRemoteDisconnected(LPARAM /*lparam*/) {
  SetMessage(L"Remote disconnected.");
}


void OnRemoteEventStart(LPARAM lparam) {
  std::unique_ptr<StartInfoTuple> params{
      reinterpret_cast<StartInfoTuple *>(lparam)};
  const auto &source_title = std::get<0>(*params);
  const auto &user_page = std::get<1>(*params);
  const auto &privacy = std::get<2>(*params);
  const auto &description = std::get<3>(*params);
  const auto &mic = std::get<4>(*params);
  const auto &service_provider = std::get<5>(*params);
  const auto &stream_url = std::get<6>(*params);
  const auto &post_url = std::get<7>(*params);

  std::wstringstream ss;
  ss << L"Streaming started." << L"\r\n"
     << L"source_title: " << source_title << L"\r\n"
     << L"user_page: " << user_page << L"\r\n"
     << L"privacy: " << privacy << L"\r\n"
     << L"description: " << description << L"\r\n"
     << L"mic: " << mic << L"\r\n"
     << L"service_provider: " << service_provider << L"\r\n"
     << L"stream_url: " << stream_url << L"\r\n"
     << L"post_url: " << post_url << L"\r\n";

  SetMessage(ss.str());
}


void OnRemoteEventStop(LPARAM lparam) {
  std::unique_ptr<std::tuple<std::wstring>> params{
      reinterpret_cast<std::tuple<std::wstring> *>(lparam)};
  const std::wstring &source_title = std::get<0>(*params);

  std::wstringstream ss;
  ss << L"Streaming stopped." << L"\r\n"
     << L"source_title: " << source_title << L"\r\n";

  SetMessage(ss.str());
}


void OnRemoteResponseFail(LPARAM lparam) {
  std::unique_ptr<std::wstring> err_msg{
      reinterpret_cast<std::wstring *>(lparam)};

  SetMessage(*err_msg);
}


void OnRemoteResponseStatus(LPARAM lparam) {
  std::unique_ptr<StatusTuple> params{
      reinterpret_cast<StatusTuple *>(lparam)};
  const auto &status = std::get<0>(*params);
  const auto &source_title = std::get<1>(*params);
  const auto &user_name = std::get<2>(*params);
  const auto &quality = std::get<3>(*params);

  std::wstringstream ss;
  ss << L"status: " << status << L"\r\n"
     << L"source_title: " << source_title << L"\r\n"
     << L"user_name: " << user_name << L"\r\n"
     << L"quality: " << quality << L"\r\n";

  SetMessage(ss.str());
}


void OnRemoteResponseStart(LPARAM lparam) {
  std::unique_ptr<std::tuple<bool>> params{
      reinterpret_cast<std::tuple<bool> *>(lparam)};
  const bool &success = std::get<0>(*params);

  std::wstringstream ss;
  ss << L"start success: " << success << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseStop(LPARAM lparam) {
  std::unique_ptr<std::tuple<bool>> params{
      reinterpret_cast<std::tuple<bool> *>(lparam)};
  const bool &success = std::get<0>(*params);

  std::wstringstream ss;
  ss << L"stop success: " << success << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseWebcamSearch(LPARAM lparam) {
  std::unique_ptr<std::vector<
      ncstreamer_remote::NcStreamerRemote::WebcamDevice>> params{
        reinterpret_cast<std::vector<
            ncstreamer_remote::NcStreamerRemote::WebcamDevice> *>(lparam)};
  static_webcams.clear();
  std::wstringstream ss;
  ss << L"webcam list: " << L"\r\n";
  for (const auto &webcam : *params) {
    static_webcams.emplace_back(webcam);
    ss << webcam.id() << L",\r\n" << "default size: (" <<
      webcam.default_width() << L", " << webcam.default_height() <<
        L")" << L"\r\n";
  }

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseWebcamOn(LPARAM /*lparam*/) {
  std::wstringstream ss;
  ss << L"webcam on success" << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseWebcamOff(LPARAM /*lparam*/) {
  std::wstringstream ss;
  ss << L"webcam off success" << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseChromaKeyOn(LPARAM /*lparam*/) {
  std::wstringstream ss;
  ss << L"chroma key on success" << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


void OnRemoteResponseChromaKeyOff(LPARAM /*lparam*/) {
  std::wstringstream ss;
  ss << L"chroma key off success" << L"\r\n";

  ::SetWindowText(static_message_panel, ss.str().c_str());
}


INT_PTR OnCommand(WPARAM wparam, LPARAM /*lparam*/) {
  switch (LOWORD(wparam)) {
    case IDC_BUTTON_STATUS: OnStatusButton(); return TRUE;
    case IDC_BUTTON_START: OnStartButton(); return TRUE;
    case IDC_BUTTON_STOP: OnStopButton(); return TRUE;
    case IDC_BUTTON_WEBCAM_SERACH: OnWebcamSearchButton(); return TRUE;
    case IDC_BUTTON_WEBCAM_ON: OnWebcamOnButton(); return TRUE;
    case IDC_BUTTON_WEBCAM_OFF: OnWebcamOffButton(); return TRUE;
    case IDC_BUTTON_CHROMA_KEY_ON: OnChromaKeyOnButton(); return TRUE;
    case IDC_BUTTON_CHROMA_KEY_OFF: OnChromaKeyOffButton(); return TRUE;
    case IDC_BUTTON_EXIT: OnExitButton(); return TRUE;
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
  static_webcam_searach_button = ::GetDlgItem(dlg, IDC_BUTTON_WEBCAM_SERACH);
  static_webcam_on_button = ::GetDlgItem(dlg, IDC_BUTTON_WEBCAM_ON);
  static_webcam_off_button = ::GetDlgItem(dlg, IDC_BUTTON_WEBCAM_OFF);
  static_chroma_key_on_button = ::GetDlgItem(dlg, IDC_BUTTON_CHROMA_KEY_ON);
  static_chroma_key_off_button = ::GetDlgItem(dlg, IDC_BUTTON_CHROMA_KEY_OFF);
  static_message_panel = ::GetDlgItem(dlg, IDC_EDIT_MESSAGE);

  static_clock_font = SetUpFont(static_clock_panel, 36);

  ::SetTimer(static_clock_panel, 0, 50 /*milliseconds*/, [](
      HWND wnd, UINT /*msg*/, UINT_PTR /*event_id*/, DWORD /*tick*/) {
    std::wstring tm = GetCurrentLocalTime();
    ::SetWindowText(wnd, tm.c_str());
  });

  ncstreamer_remote::NcStreamerRemote::SetUpDefault();
  ncstreamer_remote::NcStreamerRemote::Get()->RegisterConnectHandler([]() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_CONNECTED,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
  ncstreamer_remote::NcStreamerRemote::Get()->RegisterDisconnectHandler([]() {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_DISCONNECTED,
        (WPARAM) nullptr,
        (LPARAM) nullptr);
  });
  ncstreamer_remote::NcStreamerRemote::Get()->RegisterStartEventHandler([](
      const std::wstring &source_title,
      const std::wstring &user_page,
      const std::wstring &privacy,
      const std::wstring &description,
      const std::wstring &mic,
      const std::wstring &service_provider,
      const std::wstring &stream_url,
      const std::wstring &post_url) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_EVENT_START,
        (WPARAM) nullptr,
        (LPARAM) new StartInfoTuple{
            source_title,
            user_page,
            privacy,
            description,
            mic,
            service_provider,
            stream_url,
            post_url});
  });
  ncstreamer_remote::NcStreamerRemote::Get()->RegisterStopEventHandler([](
      const std::wstring &source_title) {
    ::PostMessage(
        static_main_dialog,
        WM_USER__REMOTE_EVENT_STOP,
        (WPARAM) nullptr,
        (LPARAM) new std::tuple<std::wstring>{source_title});
  });

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
    case WM_USER__REMOTE_CONNECTED:
        OnRemoteConnected(lparam); return TRUE;
    case WM_USER__REMOTE_DISCONNECTED:
        OnRemoteDisconnected(lparam); return TRUE;
    case WM_USER__REMOTE_EVENT_START:
        OnRemoteEventStart(lparam); return TRUE;
    case WM_USER__REMOTE_EVENT_STOP:
        OnRemoteEventStop(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_FAIL:
        OnRemoteResponseFail(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_STATUS:
        OnRemoteResponseStatus(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_START:
        OnRemoteResponseStart(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_STOP:
        OnRemoteResponseStop(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_WEBCAM_SEARCH:
        OnRemoteResponseWebcamSearch(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_WEBCAM_ON:
        OnRemoteResponseWebcamOn(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_WEBCAM_OFF:
        OnRemoteResponseWebcamOff(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_CHROMA_KEY_ON:
        OnRemoteResponseChromaKeyOn(lparam); return TRUE;
    case WM_USER__REMOTE_RESPONSE_CHROMA_KEY_OFF:
        OnRemoteResponseChromaKeyOff(lparam); return TRUE;
    case WM_CLOSE: DeleteMainDialog(); return TRUE;
    case WM_DESTROY: ::PostQuitMessage(/*exit code*/ 0); return TRUE;
    default: break;
  }

  return FALSE;
}
}  // namespace ncstreamer_remote_reference
