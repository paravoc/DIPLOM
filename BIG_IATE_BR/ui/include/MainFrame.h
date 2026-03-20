#pragma once
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/timer.h>

class CameraPanel;
class LogPanel;
class StatsPanel;

class MainFrame : public wxFrame {
public:
    MainFrame();
    virtual ~MainFrame();

private:
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnUpdateTime(wxTimerEvent& event);
    void SetupUI();
    void UpdateStats();

    wxSplitterWindow* m_splitter;
    CameraPanel* m_cameraPanel;
    LogPanel* m_logPanel;
    StatsPanel* m_statsPanel;
    wxTimer* m_timer;
    wxStatusBar* m_statusBar;

    wxDECLARE_EVENT_TABLE();
};