#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/statline.h>

enum {
    ID_Settings = wxID_HIGHEST + 1,
    ID_AddCamera,
    ID_RemoveCamera,
    ID_ExportLog,
    ID_ClearLog,
    ID_Fullscreen
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);

private:
    void OnExit(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnAddCamera(wxCommandEvent& event);
    void OnRemoveCamera(wxCommandEvent& event);
    void OnExportLog(wxCommandEvent& event);
    void OnClearLog(wxCommandEvent& event);
    void OnFullscreen(wxCommandEvent& event);
    void OnUpdateTime(wxTimerEvent& event);
    void OnCameraClick(wxMouseEvent& event);

    void SetupModernUI();
    void CreateCameraGrid(int rows, int cols);
    void AddLogEntry(const wxString& time, const wxString& event,
        const wxString& camera, const wxString& result,
        const wxString& details, bool success);

    wxMenuBar* m_menuBar;
    wxStatusBar* m_statusBar;
    wxSplitterWindow* m_mainSplitter;
    wxPanel* m_cameraPanel;
    wxPanel* m_logPanel;
    wxListCtrl* m_logList;
    wxTimer* m_timer;
    wxStaticText* m_timeText;
    wxStaticText* m_dateText;
    wxStaticText* m_statsText;

    int m_cameraRows;
    int m_cameraCols;
    wxColour m_bgColor;
    wxColour m_panelColor;
    wxColour m_accentColor;
    wxColour m_textColor;
    wxColour m_successColor;
    wxColour m_warningColor;
    wxColour m_errorColor;

    wxDECLARE_EVENT_TABLE();
};