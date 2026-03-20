#include "../include/StatsPanel.h"

#define _T(str) wxString::FromUTF8(str)

wxBEGIN_EVENT_TABLE(StatsPanel, wxPanel)
EVT_BUTTON(wxID_ANY, StatsPanel::OnShowDetails)
wxEND_EVENT_TABLE()

StatsPanel::StatsPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    SetBackgroundColour(wxColour(40, 40, 50));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("СТАТИСТИКА"));
    title->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(*wxWHITE);
    sizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 8);

    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 5, 10);

    grid->Add(new wxStaticText(this, wxID_ANY, _T("✅ Разрешено:")), 0, wxALIGN_RIGHT);
    m_statAuth = new wxStaticText(this, wxID_ANY, _T("0"));
    m_statAuth->SetForegroundColour(wxColour(0, 180, 0));
    grid->Add(m_statAuth, 0, wxALIGN_LEFT);

    grid->Add(new wxStaticText(this, wxID_ANY, _T("❌ Запрещено:")), 0, wxALIGN_RIGHT);
    m_statDenied = new wxStaticText(this, wxID_ANY, _T("0"));
    m_statDenied->SetForegroundColour(wxColour(180, 0, 0));
    grid->Add(m_statDenied, 0, wxALIGN_LEFT);

    grid->Add(new wxStaticText(this, wxID_ANY, _T("❓ Неопознано:")), 0, wxALIGN_RIGHT);
    m_statUnknown = new wxStaticText(this, wxID_ANY, _T("0"));
    m_statUnknown->SetForegroundColour(wxColour(255, 140, 0));
    grid->Add(m_statUnknown, 0, wxALIGN_LEFT);

    grid->Add(new wxStaticText(this, wxID_ANY, _T("⚙️ Система:")), 0, wxALIGN_RIGHT);
    m_statSystem = new wxStaticText(this, wxID_ANY, _T("0"));
    m_statSystem->SetForegroundColour(wxColour(0, 120, 215));
    grid->Add(m_statSystem, 0, wxALIGN_LEFT);

    sizer->Add(grid, 0, wxALIGN_CENTER | wxALL, 10);

    m_detailsBtn = new wxButton(this, wxID_ANY, _T("📊 Подробная статистика"));
    m_detailsBtn->SetBackgroundColour(wxColour(0, 120, 215));
    m_detailsBtn->SetForegroundColour(*wxWHITE);
    sizer->Add(m_detailsBtn, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    SetSizer(sizer);
}

StatsPanel::~StatsPanel() {}

void StatsPanel::UpdateStats(int authorized, int denied, int unknown, int system) {
    m_statAuth->SetLabel(wxString::Format(_T("%d"), authorized));
    m_statDenied->SetLabel(wxString::Format(_T("%d"), denied));
    m_statUnknown->SetLabel(wxString::Format(_T("%d"), unknown));
    m_statSystem->SetLabel(wxString::Format(_T("%d"), system));
}

void StatsPanel::OnShowDetails(wxCommandEvent& event) {
    ShowDetailedStats();
}

void StatsPanel::ShowDetailedStats() {
    wxDialog dialog(this, wxID_ANY, _T("Детальная статистика"), wxDefaultPosition, wxSize(500, 400));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(&dialog, wxID_ANY, _T("СТАТИСТИКА СИСТЕМЫ"));
    title->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    sizer->Add(title, 0, wxALIGN_CENTER | wxALL, 15);

    int total = wxAtoi(m_statAuth->GetLabel()) + wxAtoi(m_statDenied->GetLabel()) +
        wxAtoi(m_statUnknown->GetLabel()) + wxAtoi(m_statSystem->GetLabel());

    wxStaticText* text = new wxStaticText(&dialog, wxID_ANY,
        wxString::Format(_T("Разрешённые проходы: %s\nЗапрещённые проходы: %s\nНеопознанные: %s\nСистемные события: %s\n\nВсего событий: %d"),
            m_statAuth->GetLabel(), m_statDenied->GetLabel(),
            m_statUnknown->GetLabel(), m_statSystem->GetLabel(), total));

    sizer->Add(text, 0, wxALIGN_CENTER | wxALL, 10);

    wxButton* closeBtn = new wxButton(&dialog, wxID_OK, _T("Закрыть"));
    closeBtn->SetBackgroundColour(wxColour(0, 120, 215));
    closeBtn->SetForegroundColour(*wxWHITE);
    sizer->Add(closeBtn, 0, wxALIGN_CENTER | wxALL, 15);

    dialog.SetSizer(sizer);
    dialog.Centre();
    dialog.ShowModal();
}