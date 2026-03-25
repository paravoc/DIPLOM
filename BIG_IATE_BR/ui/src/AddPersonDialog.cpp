#include "../include/AddPersonDialog.h"
#include "../include/PhotoCaptureDialog.h"
#include "../../recognition/include/FaceRecognizer.h"
#include <wx/filedlg.h>
#include <wx/statline.h>
#include <wx/datectrl.h>
#include <ctime>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_SELECT_PHOTO = 1000,
    ID_CAMERA_CAPTURE,
    ID_OK,
    ID_CANCEL
};

wxBEGIN_EVENT_TABLE(AddPersonDialog, wxDialog)
EVT_BUTTON(ID_SELECT_PHOTO, AddPersonDialog::OnSelectPhoto)
EVT_BUTTON(ID_CAMERA_CAPTURE, AddPersonDialog::OnCameraCapture)
EVT_BUTTON(ID_OK, AddPersonDialog::OnOk)
EVT_BUTTON(ID_CANCEL, AddPersonDialog::OnCancel)
wxEND_EVENT_TABLE()

AddPersonDialog::AddPersonDialog(wxWindow* parent,
    std::shared_ptr<bigiate::recognition::FaceRecognizer> recognizer,
    const std::vector<bigiate::config::CameraConfig>& cameras)
    : wxDialog(parent, wxID_ANY, _T("Добавление человека"),
        wxDefaultPosition, wxSize(550, 620)),
    m_recognizer(recognizer),
    m_cameras(cameras) {

    SetBackgroundColour(wxColour(35, 35, 45));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок
    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("ДОБАВЛЕНИЕ НОВОГО ЧЕЛОВЕКА"));
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(wxColour(0, 180, 255));
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP, 20);

    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(500, 2), wxLI_HORIZONTAL);
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
    photoSizer->Add(btnPhoto, 0, wxRIGHT, 5);

    wxButton* btnCamera = new wxButton(this, ID_CAMERA_CAPTURE, _T("📷 С камеры"), wxDefaultPosition, wxSize(80, 28));
    btnCamera->SetBackgroundColour(wxColour(0, 120, 200));
    btnCamera->SetForegroundColour(*wxWHITE);
    photoSizer->Add(btnCamera, 0);

    grid->Add(photoSizer, 1, wxEXPAND);

    // ФИО
    grid->Add(new wxStaticText(this, wxID_ANY, _T("ФИО:*")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_txtFullName = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(300, 28));
    m_txtFullName->SetBackgroundColour(wxColour(50, 50, 65));
    m_txtFullName->SetForegroundColour(*wxWHITE);
    grid->Add(m_txtFullName, 1, wxEXPAND);

    // Тип
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Тип:*")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    wxArrayString types;
    types.Add(_T("student"));
    types.Add(_T("teacher"));
    types.Add(_T("staff"));
    types.Add(_T("guest"));
    m_choiceType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(120, 28), types);
    m_choiceType->SetSelection(0);
    m_choiceType->SetBackgroundColour(wxColour(50, 50, 65));
    m_choiceType->SetForegroundColour(*wxWHITE);
    grid->Add(m_choiceType, 1, wxEXPAND);

    // Дата рождения
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Дата рождения:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_dateBirth = new wxDatePickerCtrl(this, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxSize(120, 28));
    m_dateBirth->SetBackgroundColour(wxColour(50, 50, 65));
    m_dateBirth->SetForegroundColour(*wxWHITE);
    grid->Add(m_dateBirth, 1, wxEXPAND);

    // Пол
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Пол:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    wxBoxSizer* genderSizer = new wxBoxSizer(wxHORIZONTAL);
    m_radioMale = new wxRadioButton(this, wxID_ANY, _T("Мужской"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_radioMale->SetForegroundColour(*wxWHITE);
    m_radioMale->SetValue(true);
    m_radioFemale = new wxRadioButton(this, wxID_ANY, _T("Женский"));
    m_radioFemale->SetForegroundColour(*wxWHITE);
    genderSizer->Add(m_radioMale, 0, wxRIGHT, 20);
    genderSizer->Add(m_radioFemale, 0);
    grid->Add(genderSizer, 1, wxEXPAND);

    // Телефон
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Телефон:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_txtPhone = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, 28));
    m_txtPhone->SetBackgroundColour(wxColour(50, 50, 65));
    m_txtPhone->SetForegroundColour(*wxWHITE);
    grid->Add(m_txtPhone, 1, wxEXPAND);

    // Email
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Email:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_txtEmail = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250, 28));
    m_txtEmail->SetBackgroundColour(wxColour(50, 50, 65));
    m_txtEmail->SetForegroundColour(*wxWHITE);
    grid->Add(m_txtEmail, 1, wxEXPAND);

    // Адрес
    grid->Add(new wxStaticText(this, wxID_ANY, _T("Адрес:")), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_txtAddress = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(350, 60), wxTE_MULTILINE);
    m_txtAddress->SetBackgroundColour(wxColour(50, 50, 65));
    m_txtAddress->SetForegroundColour(*wxWHITE);
    grid->Add(m_txtAddress, 1, wxEXPAND);

    mainSizer->Add(grid, 1, wxEXPAND | wxALL, 20);

    // Разделитель
    wxStaticLine* line2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(500, 2), wxLI_HORIZONTAL);
    line2->SetForegroundColour(wxColour(80, 80, 100));
    mainSizer->Add(line2, 0, wxEXPAND | wxLEFT | wxRIGHT, 20);

    // Кнопки
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer();

    wxButton* btnOk = new wxButton(this, ID_OK, _T("✅ Добавить"), wxDefaultPosition, wxSize(120, 35));
    btnOk->SetBackgroundColour(wxColour(0, 140, 0));
    btnOk->SetForegroundColour(*wxWHITE);
    btnOk->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    btnSizer->Add(btnOk, 0, wxRIGHT, 10);

    wxButton* btnCancel = new wxButton(this, ID_CANCEL, _T("✖ Отмена"), wxDefaultPosition, wxSize(100, 35));
    btnCancel->SetBackgroundColour(wxColour(140, 60, 60));
    btnCancel->SetForegroundColour(*wxWHITE);
    btnSizer->Add(btnCancel, 0);

    btnSizer->AddStretchSpacer();
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 20);

    SetSizer(mainSizer);
    Centre();
}

void AddPersonDialog::UpdatePhotoCount() {
    wxString text;
    if (m_data.imagePaths.empty()) {
        text = _T("Не выбрано");
        m_lblPhotoPath->SetForegroundColour(wxColour(150, 150, 170));
    }
    else if (m_data.imagePaths.size() == 1) {
        text = wxString::Format(_T("Выбрано 1 фото"));
        m_lblPhotoPath->SetForegroundColour(wxColour(0, 200, 0));
    }
    else {
        text = wxString::Format(_T("Выбрано %d фото"), (int)m_data.imagePaths.size());
        m_lblPhotoPath->SetForegroundColour(wxColour(0, 200, 0));
    }
    m_lblPhotoPath->SetLabel(text);
}

void AddPersonDialog::OnSelectPhoto(wxCommandEvent& event) {
    wxFileDialog dlg(this,
        _T("Выберите фото"),
        wxEmptyString,
        wxEmptyString,
        "Image files (*.jpg;*.png;*.bmp)|*.jpg;*.jpeg;*.png;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dlg.GetPaths(paths);

        for (size_t i = 0; i < paths.size(); i++) {
            m_data.imagePaths.push_back(paths[i].ToStdString());
        }
        UpdatePhotoCount();
    }
}

void AddPersonDialog::OnCameraCapture(wxCommandEvent& event) {
    PhotoCaptureDialog dlg(this, m_recognizer, m_cameras);

    if (dlg.ShowModal() == wxID_OK && dlg.HasPhotos()) {
        auto photos = dlg.GetCapturedPhotos();

        for (const auto& photo : photos) {
            static int photoCounter = 0;
            std::string tempPath = "temp_face_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(photoCounter++) + ".jpg";
            cv::imwrite(tempPath, photo.faceImage);
            m_data.imagePaths.push_back(tempPath);
            m_data.embeddings.push_back(photo.embedding);
        }

        UpdatePhotoCount();

        wxMessageBox(wxString::Format(_T("Сделано %d фото! Нажмите Добавить для сохранения."),
            (int)m_data.imagePaths.size()),
            _T("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void AddPersonDialog::OnOk(wxCommandEvent& event) {
    if (m_data.imagePaths.empty()) {
        wxMessageBox(_T("Выберите фото или сделайте снимок с камеры"), _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    if (m_txtFullName->GetValue().IsEmpty()) {
        wxMessageBox(_T("Введите ФИО"), _T("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    m_data.fullName = m_txtFullName->GetValue().ToStdString();
    m_data.personType = m_choiceType->GetStringSelection().ToStdString();
    m_data.birthDate = m_dateBirth->GetValue().Format("%Y-%m-%d").ToStdString();
    m_data.gender = m_radioMale->GetValue() ? 'M' : 'F';
    m_data.phone = m_txtPhone->GetValue().ToStdString();
    m_data.email = m_txtEmail->GetValue().ToStdString();
    m_data.address = m_txtAddress->GetValue().ToStdString();

    EndModal(wxID_OK);
}

void AddPersonDialog::OnCancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}