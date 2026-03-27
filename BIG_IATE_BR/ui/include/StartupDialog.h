#pragma once

#include <wx/wx.h>

class StartupDialog : public wxDialog {
public:
    StartupDialog(wxWindow* parent);

    enum Action {
        ACTION_RUN = 0,
        ACTION_ADD_PERSON,
        ACTION_LIST_PERSONS,
        ACTION_DELETE_ENCODING,  
        ACTION_EXIT
    };

    Action GetSelectedAction() const { return m_selectedAction; }

private:
    void OnRun(wxCommandEvent& event);
    void OnAddPerson(wxCommandEvent& event);
    void OnListPersons(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnDeleteEncoding(wxCommandEvent& event);

    Action m_selectedAction = ACTION_EXIT;

    wxDECLARE_EVENT_TABLE();
};