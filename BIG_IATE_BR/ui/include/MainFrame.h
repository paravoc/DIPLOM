#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/timer.h>
#include <vector>
#include <string>
#include <memory>

// Подключаем типы из конфига
#include "../../configs/include/types.h"

// Forward declarations
namespace bigiate::ui {
    class CameraPanel;
    class LogsWindow;
}
class LogPanel;
class StatsPanel;

namespace bigiate::db {
    class DBQueries;
}

class MainFrame : public wxFrame {
public:
    MainFrame();
    virtual ~MainFrame();

    // Публичные методы для внешнего использования
    void SetCameras(const std::vector<bigiate::config::CameraConfig>& cameras);
    void UpdateCameraFrame(int cameraId, const wxImage& frame);
    void UpdateCameraStatus(int cameraId, bool online);

    // Установка DBQueries из AppCore
    void setDBQueries(std::shared_ptr<bigiate::db::DBQueries> dbQueries);

private:
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnUpdateTime(wxTimerEvent& event);
    void SetupUI();
    void UpdateStats();
    void onShowLogs(wxCommandEvent& event);  // ← только один раз!

    wxSplitterWindow* m_splitter;
    bigiate::ui::CameraPanel* m_cameraPanel;
    LogPanel* m_logPanel;
    StatsPanel* m_statsPanel;
    wxTimer* m_timer;
    wxStatusBar* m_statusBar;

    // Для окна логов
    bigiate::ui::LogsWindow* m_logsWindow{ nullptr };
    std::shared_ptr<bigiate::db::DBQueries> m_dbQueries;

    wxDECLARE_EVENT_TABLE();
};