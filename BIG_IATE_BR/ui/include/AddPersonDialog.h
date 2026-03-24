#pragma once

#include <wx/wx.h>

class AddPersonDialog : public wxDialog {
public:
    AddPersonDialog(wxWindow* parent);

    struct PersonData {
        std::string imagePath;
        std::string fullName;
        std::string personType;
    };

    PersonData GetData() const { return m_data; }
    bool HasData() const { return m_hasData; }

private:
    void OnSelectPhoto(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    PersonData m_data;
    bool m_hasData = false;

    wxStaticText* m_lblPhotoPath;
    wxTextCtrl* m_txtFullName;
    wxChoice* m_choiceType;

    wxDECLARE_EVENT_TABLE();
};