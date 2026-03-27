#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>
#include <map>
#include <string>
#include "../../database/include/DBModels.h"

class DeleteEncodingDialog : public wxDialog {
public:
    DeleteEncodingDialog(wxWindow* parent,
        const std::vector<bigiate::db::Person>& persons,
        const std::map<int, std::vector<bigiate::db::FaceEncoding>>& encodings);

    struct DeleteData {
        int personId = -1;
        int encodingId = -1;
        std::string personName;
    };

    DeleteData GetSelected() const { return m_selected; }
    bool HasSelection() const { return m_selected.encodingId != -1; }

private:
    void OnPersonSelected(wxCommandEvent& event);
    void OnEncodingSelected(wxListEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void UpdateEncodingList();

    wxChoice* m_personChoice;
    wxListCtrl* m_encodingList;
    wxButton* m_deleteBtn;

    std::vector<bigiate::db::Person> m_persons;
    std::map<int, std::vector<bigiate::db::FaceEncoding>> m_encodings;
    DeleteData m_selected;

    wxDECLARE_EVENT_TABLE();
};