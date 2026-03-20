#pragma once
#include <wx/wx.h>

class StatsPanel : public wxPanel {
public:
    StatsPanel(wxWindow* parent);
    virtual ~StatsPanel();

    void UpdateStats(int authorized, int denied, int unknown, int system);
    void ShowDetailedStats();

private:
    void OnShowDetails(wxCommandEvent& event);

    wxStaticText* m_statAuth;
    wxStaticText* m_statDenied;
    wxStaticText* m_statUnknown;
    wxStaticText* m_statSystem;
    wxButton* m_detailsBtn;

    wxDECLARE_EVENT_TABLE();
};