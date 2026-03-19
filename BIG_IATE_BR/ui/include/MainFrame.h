#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/splitter.h>

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);

private:
    void OnExit(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    // Компоненты интерфейса
    wxStatusBar* m_statusBar;
    wxMenuBar* m_menuBar;
    wxSplitterWindow* m_mainSplitter;
    wxPanel* m_cameraPanel;
    wxPanel* m_logPanel;

    wxDECLARE_EVENT_TABLE();
};

// ID для меню
enum {
    ID_Settings = wxID_HIGHEST + 1
};