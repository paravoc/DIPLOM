#include "../include/StartupDialog.h"
#include <wx/statline.h>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_RUN = 1000,
    ID_ADD_PERSON,
    ID_LIST_PERSONS,
    ID_EXIT
};

wxBEGIN_EVENT_TABLE(StartupDialog, wxDialog)
EVT_BUTTON(ID_RUN, StartupDialog::OnRun)
EVT_BUTTON(ID_ADD_PERSON, StartupDialog::OnAddPerson)
EVT_BUTTON(ID_LIST_PERSONS, StartupDialog::OnListPersons)
EVT_BUTTON(ID_EXIT, StartupDialog::OnExit)
wxEND_EVENT_TABLE()

StartupDialog::StartupDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _T("BIG IATE"),
        wxDefaultPosition, wxSize(350, 400)) {

    SetBackgroundColour(wxColour(30, 30, 40));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок
    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("BIG IATE"));
    title->SetFont(wxFont(28, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 180, 255));
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 40);

    wxStaticText* subtitle = new wxStaticText(this, wxID_ANY, _T("Система контроля доступа"));
    subtitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    subtitle->SetForegroundColour(wxColour(180, 180, 200));
    mainSizer->Add(subtitle, 0, wxALIGN_CENTER | wxBOTTOM, 30);

    // Кнопки
    wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btnRun = new wxButton(this, ID_RUN, _T("▶ Запустить"), wxDefaultPosition, wxSize(220, 45));
    btnRun->SetBackgroundColour(wxColour(0, 120, 200));
    btnRun->SetForegroundColour(*wxWHITE);
    btnRun->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    btnSizer->Add(btnRun, 0, wxALIGN_CENTER | wxBOTTOM, 12);

    wxButton* btnAdd = new wxButton(this, ID_ADD_PERSON, _T("➕ Добавить человека"), wxDefaultPosition, wxSize(220, 40));
    btnAdd->SetBackgroundColour(wxColour(60, 60, 80));
    btnAdd->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnAdd, 0, wxALIGN_CENTER | wxBOTTOM, 12);

    wxButton* btnList = new wxButton(this, ID_LIST_PERSONS, _T("📋 Список людей"), wxDefaultPosition, wxSize(220, 40));
    btnList->SetBackgroundColour(wxColour(60, 60, 80));
    btnList->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnList, 0, wxALIGN_CENTER | wxBOTTOM, 12);

    wxButton* btnExit = new wxButton(this, ID_EXIT, _T("✖ Выход"), wxDefaultPosition, wxSize(220, 40));
    btnExit->SetBackgroundColour(wxColour(100, 60, 60));
    btnExit->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnExit, 0, wxALIGN_CENTER | wxBOTTOM, 20);

    mainSizer->Add(btnSizer, 1, wxALIGN_CENTER);

    SetSizer(mainSizer);
    Centre();
}

void StartupDialog::OnRun(wxCommandEvent& event) {
    m_selectedAction = ACTION_RUN;
    EndModal(wxID_OK);
}

void StartupDialog::OnAddPerson(wxCommandEvent& event) {
    m_selectedAction = ACTION_ADD_PERSON;
    EndModal(wxID_OK);
}

void StartupDialog::OnListPersons(wxCommandEvent& event) {
    m_selectedAction = ACTION_LIST_PERSONS;
    EndModal(wxID_OK);
}

void StartupDialog::OnExit(wxCommandEvent& event) {
    m_selectedAction = ACTION_EXIT;
    EndModal(wxID_CANCEL);
}