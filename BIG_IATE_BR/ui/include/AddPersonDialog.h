#pragma once

#include <wx/wx.h>
#include <wx/datectrl.h>
#include <vector>
#include <string>
#include <memory>
#include "../../configs/include/types.h"

// Forward declaration
namespace bigiate::recognition {
    class FaceRecognizer;
}

class AddPersonDialog : public wxDialog {
public:
    AddPersonDialog(wxWindow* parent,
        std::shared_ptr<bigiate::recognition::FaceRecognizer> recognizer,
        const std::vector<bigiate::config::CameraConfig>& cameras);

    struct PersonData {
        std::vector<std::string> imagePaths;
        std::vector<std::vector<float>> embeddings;
        std::string fullName;
        std::string personType;
        std::string birthDate;
        char gender;
        std::string phone;
        std::string email;
        std::string address;
    };

    PersonData GetData() const { return m_data; }
    bool HasData() const { return !m_data.imagePaths.empty() && !m_data.fullName.empty(); }

private:
    void OnSelectPhoto(wxCommandEvent& event);
    void OnCameraCapture(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void UpdatePhotoCount();

    PersonData m_data;

    wxStaticText* m_lblPhotoPath;
    wxTextCtrl* m_txtFullName;
    wxChoice* m_choiceType;
    wxDatePickerCtrl* m_dateBirth;
    wxRadioButton* m_radioMale;
    wxRadioButton* m_radioFemale;
    wxTextCtrl* m_txtPhone;
    wxTextCtrl* m_txtEmail;
    wxTextCtrl* m_txtAddress;

    std::shared_ptr<bigiate::recognition::FaceRecognizer> m_recognizer;
    std::vector<bigiate::config::CameraConfig> m_cameras;

    wxDECLARE_EVENT_TABLE();
};