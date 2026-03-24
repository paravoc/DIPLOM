#include "../include/AddPersonDialog.h"
#include <wx/filedlg.h>
#include <wx/statline.h>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_SELECT_PHOTO = 1000,
    ID_OK,
    ID_CANCEL
};

wxBEGIN_EVENT_TABLE(AddPersonDialog, wxDialog)
EVT_BUTTON(ID_SELECT_PHOTO, AddPersonDialog::OnSelectPhoto)
EVT_BUTTON(ID_OK, AddPersonDialog::OnOk)
EVT_BUTTON(ID_CANCEL, AddPersonDialog::OnCancel)
wxEND_EVENT_TABLE()

AddPersonDialog::AddPersonDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _T("Добавление человека"),
        wxDefaultPosition, wxSize(450, 350)) {

    SetBackgroundColour(wxColour(35, 35, 45));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок
    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("ДОБАВЛЕНИЕ ЧЕЛОВЕКА"));
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 180, 255));
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 20);

    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(400, 2), wxLI_HORIZONTAL);
    line->SetForegroundColour(wxColour(80, 80, 100));
    mainSizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 15);

    // Форма
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 10, 12);
    grid->SetFlexibleDirection(wxBOTH);
    grid->AddGrowableCol(1);

    // Фото
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Фото:*")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    wxBoxSizer* photoSizer = new wxBoxSizer(wxHORIZONTAL);
    m_lblPhotoPath = new wxStaticText(this, wxID_ANY, _T("Не выбрано"));
    m_lblPhotoPath->SetForegroundColour(wxColour(150, 150, 170));
    photoSizer->Add(m_lblPhotoPath, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxButton* btnPhoto = new wxButton(this, ID_SELECT_PHOTO, _T("Выбрать"), wxDefaultPosition, wxSize(70, 28));
    btnPhoto->SetBackgroundColour(wxColour(0, 120, 200));
    btnPhoto->SetForegroundColour(*wxWHITE);
    photoSizer->Add(btnPhoto, 0);
    grid->Add(photoSizer, 1, wxEXPAND);

    // ФИО
    grid->Add(new wxStaticText(this, wxID_ANY, _T("ФИО:*")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_txtFullName = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(280, 28));
    m_txtFullName->SetBackgroundColour(wxColour(50, 50, 65));
    m_txtFullName->SetForegroundColour(*wxWHITE);
    grid->Add(m_txtFullName, 1, wxEXPAND);

    // Тип
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Тип:*")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    wxArrayString types;
    types.Add("student");
    types.Add("teacher");
    types.Add("staff");
    types.Add("guest");
    m_choiceType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(120, 28), types);
    m_choiceType->SetSelection(0);
    m_choiceType->SetBackgroundColour(wxColour(50, 50, 65));
    m_choiceType->SetForegroundColour(*wxWHITE);
    grid->Add(m_choiceType, 1, wxEXPAND);

    mainSizer->Add(grid, 0, wxEXPAND | wxALL, 20);

    // Разделитель
    wxStaticLine* line2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(400, 2), wxLI_HORIZONTAL);
    line2->SetForegroundColour(wxColour(80, 80, 100));
    mainSizer->Add(line2, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);

    // Кнопки
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer();

    wxButton* btnOk = new wxButton(this, ID_OK, _T("Добавить"), wxDefaultPosition, wxSize(100, 35));
    btnOk->SetBackgroundColour(wxColour(0, 140, 0));
    btnOk->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnOk, 0, wxRIGHT, 10);

    wxButton* btnCancel = new wxButton(this, ID_CANCEL, _T("Отмена"), wxDefaultPosition, wxSize(100, 35));
    btnCancel->SetBackgroundColour(wxColour(140, 60, 60));
    btnCancel->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnCancel, 0);

    btnSizer->AddStretchSpacer();
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 20);

    SetSizer(mainSizer);
    Centre();
}

void AddPersonDialog::OnSelectPhoto(wxCommandEvent& event) {
    wxFileDialog dlg(this,
        _T("Выберите фото"),
        wxEmptyString,
        wxEmptyString,
        "Image files (*.jpg;*.png;*.bmp)|*.jpg;*.jpeg;*.png;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        m_data.imagePath = dlg.GetPath().ToStdString();
        m_lblPhotoPath->SetLabel(dlg.GetPath());
        m_lblPhotoPath->SetForegroundColour(wxColour(0, 200, 0));
    }
}

void AddPersonDialog::OnOk(wxCommandEvent& event) {
    if (m_data.imagePath.empty()) {
        wxMessageBox(_T("Выберите фото"), _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    if (m_txtFullName->GetValue().IsEmpty()) {
        wxMessageBox(_T("Введите ФИО"), _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    m_data.fullName = m_txtFullName->GetValue().ToStdString();
    m_data.personType = m_choiceType->GetStringSelection().ToStdString();

    m_hasData = true;
    EndModal(wxID_OK);
}

void AddPersonDialog::OnCancel(wxCommandEvent& event) {
    m_hasData = false;
    EndModal(wxID_CANCEL);
}