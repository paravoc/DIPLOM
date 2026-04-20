// ui/include/LogsWindow.h
#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <memory>
#include <map>
#include <string>

// Правильные пространства имён
namespace bigiate::db {
    class DBQueries;
    struct AccessLog;
}

namespace bigiate::ui {

    class LogsWindow : public wxFrame {
    public:
        LogsWindow(wxWindow* parent, std::shared_ptr<bigiate::db::DBQueries> dbQueries);
        virtual ~LogsWindow();

    private:
        void onSearch(wxCommandEvent& event);
        void onDateChanged(wxDateEvent& event);
        void onExport(wxCommandEvent& event);
        void onClear(wxCommandEvent& event);
        void onRefresh(wxCommandEvent& event);
        void loadLogs(const std::string& fromDate = "", const std::string& toDate = "", const std::string& search = "");
        wxString getPersonName(int personId);

        wxListCtrl* m_logList;
        wxDatePickerCtrl* m_dateFrom;
        wxDatePickerCtrl* m_dateTo;
        wxTextCtrl* m_searchBox;
        wxButton* m_searchBtn;
        wxButton* m_exportBtn;
        wxButton* m_clearBtn;
        wxButton* m_refreshBtn;
        wxStaticText* m_statusText;

        std::shared_ptr<bigiate::db::DBQueries> m_dbQueries;
        std::map<int, std::string> m_personNames;

        wxDECLARE_EVENT_TABLE();
    };

} // namespace bigiate::ui