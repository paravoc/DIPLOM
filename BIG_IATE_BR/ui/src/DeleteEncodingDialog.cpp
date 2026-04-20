#include "../include/DeleteEncodingDialog.h"
#include <wx/statline.h>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_PERSON_CHOICE = 3000,
    ID_ENCODING_LIST,
    ID_DELETE_BTN,
    ID_CANCEL_BTN
};

wxBEGIN_EVENT_TABLE(DeleteEncodingDialog, wxDialog)
EVT_CHOICE(ID_PERSON_CHOICE, DeleteEncodingDialog::OnPersonSelected)
EVT_LIST_ITEM_SELECTED(ID_ENCODING_LIST, DeleteEncodingDialog::OnEncodingSelected)
EVT_BUTTON(ID_DELETE_BTN, DeleteEncodingDialog::OnDelete)
EVT_BUTTON(ID_CANCEL_BTN, DeleteEncodingDialog::OnCancel)
wxEND_EVENT_TABLE()

DeleteEncodingDialog::DeleteEncodingDialog(wxWindow* parent,
    const std::vector<bigiate::db::Person>& persons,
    const std::map<int, std::vector<bigiate::db::FaceEncoding>>& encodings)
    : wxDialog(parent, wxID_ANY, _T("Удаление фото"),
        wxDefaultPosition, wxSize(650, 500)),
    m_persons(persons),
    m_encodings(encodings) {

    m_selected.personId = -1;
    m_selected.encodingId = -1;

    SetBackgroundColour(wxColour(35, 35, 45));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок
    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("УДАЛЕНИЕ ФОТО ЧЕЛОВЕКА"));
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 180, 255));
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 20);

    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(600, 2), wxLI_HORIZONTAL);
    line->SetForegroundColour(wxColour(80, 80, 100));
    mainSizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 15);

    // Выбор человека
    wxBoxSizer* personSizer = new wxBoxSizer(wxHORIZONTAL);
    personSizer->Add(new wxStaticText(this, wxID_ANY, _T("Выберите человека:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_personChoice = new wxChoice(this, ID_PERSON_CHOICE, wxDefaultPosition, wxSize(350, 28));
    m_personChoice->Append(_T("-- Выберите человека --"));
    for (const auto& p : m_persons) {
        wxString name = wxString::Format(_T("ID: %d | %s | %s"), p.id,
            wxString::FromUTF8(p.fullName),
            wxString::FromUTF8(p.personType));
        m_personChoice->Append(name);
    }
    m_personChoice->SetSelection(0);
    personSizer->Add(m_personChoice, 1, wxEXPAND);

    mainSizer->Add(personSizer, 0, wxEXPAND | wxALL, 15);

    // Список фото
    wxStaticText* listLabel = new wxStaticText(this, wxID_ANY, _T("Фото человека:"));
    listLabel->SetForegroundColour(*wxWHITE);
    mainSizer->Add(listLabel, 0, wxLEFT | wxRIGHT, 15);

    m_encodingList = new wxListCtrl(this, ID_ENCODING_LIST, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_SINGLE_SEL);
    m_encodingList->SetBackgroundColour(wxColour(45, 45, 55));
    m_encodingList->SetForegroundColour(*wxWHITE);
    m_encodingList->AppendColumn(_T("ID фото"), wxLIST_FORMAT_LEFT, 80);
    m_encodingList->AppendColumn(_T("Дата"), wxLIST_FORMAT_LEFT, 130);
    m_encodingList->AppendColumn(_T("Основное"), wxLIST_FORMAT_LEFT, 80);
    m_encodingList->AppendColumn(_T("Путь к фото"), wxLIST_FORMAT_LEFT, 300);

    mainSizer->Add(m_encodingList, 1, wxEXPAND | wxALL, 15);

    // Информация
    wxStaticText* infoText = new wxStaticText(this, wxID_ANY,
        _T("⚠️ Удаляется только фото (эмбеддинг). Человек остаётся в базе."));
    infoText->SetForegroundColour(wxColour(255, 200, 100));
    infoText->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    mainSizer->Add(infoText, 0, wxALIGN_CENTER | wxALL, 10);

    // Кнопки
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer();

    m_deleteBtn = new wxButton(this, ID_DELETE_BTN, _T("🗑️ Удалить выбранное фото"), wxDefaultPosition, wxSize(200, 40));
    m_deleteBtn->SetBackgroundColour(wxColour(180, 60, 60));
    m_deleteBtn->SetForegroundColour(*wxWHITE);
    m_deleteBtn->Enable(false);
    btnSizer->Add(m_deleteBtn, 0, wxRIGHT, 15);

    wxButton* cancelBtn = new wxButton(this, ID_CANCEL_BTN, _T("✖ Отмена"), wxDefaultPosition, wxSize(100, 40));
    cancelBtn->SetBackgroundColour(wxColour(100, 100, 120));
    cancelBtn->SetForegroundColour(*wxWHITE);
    btnSizer->Add(cancelBtn, 0);

    btnSizer->AddStretchSpacer();
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 20);

    SetSizer(mainSizer);
    Centre();
}

void DeleteEncodingDialog::UpdateEncodingList() {
    m_encodingList->DeleteAllItems();

    int personId = m_selected.personId;
    if (personId <= 0) return;

    auto it = m_encodings.find(personId);
    if (it == m_encodings.end()) return;

    int index = 0;
    for (const auto& enc : it->second) {
        wxString idStr = wxString::Format(_T("%d"), enc.id);
        wxString dateStr = wxString::FromUTF8(enc.captureDate);
        wxString primaryStr = enc.isPrimary ? _T("Да") : _T("Нет");
        wxString pathStr = wxString::FromUTF8(enc.sourceImagePath);

        long item = m_encodingList->InsertItem(index, idStr);
        m_encodingList->SetItem(item, 1, dateStr);
        m_encodingList->SetItem(item, 2, primaryStr);
        m_encodingList->SetItem(item, 3, pathStr);

        if (enc.isPrimary) {
            m_encodingList->SetItemTextColour(item, wxColour(0, 200, 0));
        }

        index++;
    }

    if (index == 0) {
        m_encodingList->InsertItem(0, _T("Нет фото"));
    }
}

void DeleteEncodingDialog::OnPersonSelected(wxCommandEvent& event) {
    int sel = m_personChoice->GetSelection();
    if (sel <= 0) {
        m_selected.personId = -1;
        m_selected.encodingId = -1;
        m_encodingList->DeleteAllItems();
        m_deleteBtn->Enable(false);
        return;
    }

    // Находим человека по индексу (sel-1, потому что первый элемент "-- Выберите человека --")
    m_selected.personId = m_persons[sel - 1].id;
    m_selected.personName = m_persons[sel - 1].fullName;
    m_selected.encodingId = -1;

    UpdateEncodingList();
    m_deleteBtn->Enable(false);
}

void DeleteEncodingDialog::OnEncodingSelected(wxListEvent& event) {
    int idx = event.GetIndex();

    int personId = m_selected.personId;
    auto it = m_encodings.find(personId);
    if (it == m_encodings.end()) return;

    if (idx >= 0 && idx < (int)it->second.size()) {
        m_selected.encodingId = it->second[idx].id;
        m_deleteBtn->Enable(true);
    }
}

void DeleteEncodingDialog::OnDelete(wxCommandEvent& event) {
    if (m_selected.encodingId == -1) {
        wxMessageBox(_T("Выберите фото для удаления"), _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }
    int encodingId = m_selected.encodingId;
    std::string personName = m_selected.personName;

    // Отправляем сигнал, что нужно удалить
    wxCommandEvent deleteEvent(wxEVT_BUTTON, wxID_OK);
    deleteEvent.SetClientData(&encodingId);
    GetEventHandler()->ProcessEvent(deleteEvent);
}

void DeleteEncodingDialog::OnCancel(wxCommandEvent& event) {
    m_selected.encodingId = -1;
    EndModal(wxID_CANCEL);
}