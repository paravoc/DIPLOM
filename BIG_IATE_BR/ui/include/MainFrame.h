#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/timer.h>
#include <vector>
#include <string>

// Подключаем типы из конфига
#include "../../configs/include/types.h"

// Forward declarations
namespace bigiate::ui {
    class CameraPanel;
}
class LogPanel;
class StatsPanel;

class MainFrame : public wxFrame {
public:
    MainFrame();
    virtual ~MainFrame();

    // Публичные методы для внешнего использования
    void SetCameras(const std::vector<bigiate::config::CameraConfig>& cameras);
    void UpdateCameraFrame(int cameraId, const wxImage& frame);
    void UpdateCameraStatus(int cameraId, bool online);

private:
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnUpdateTime(wxTimerEvent& event);
    void SetupUI();
    void UpdateStats();

    wxSplitterWindow* m_splitter;
    bigiate::ui::CameraPanel* m_cameraPanel;
    LogPanel* m_logPanel;
    StatsPanel* m_statsPanel;
    wxTimer* m_timer;
    wxStatusBar* m_statusBar;

    wxDECLARE_EVENT_TABLE();
};