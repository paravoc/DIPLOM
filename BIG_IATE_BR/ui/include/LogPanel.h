#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timectrl.h>
#include <vector>

struct LogEntry {
    wxString time;
    long timestamp; // для сортировки
    wxString name;
    wxString camera;
    wxString result;
    wxString details;
    int type; // 0=разрешен, 1=запрещен, 2=неопознан, 3=система
};

class LogPanel : public wxPanel {
public:
    LogPanel(wxWindow* parent);
    virtual ~LogPanel();

    void AddLog(const wxString& time, const wxString& name,
        const wxString& camera, const wxString& result,
        const wxString& details, int type);
    void ClearLogs();
    void UpdateDisplay();

    int GetAuthorizedCount() const;
    int GetDeniedCount() const;
    int GetUnknownCount() const;
    int GetSystemCount() const;

    // Получить текущие фильтры
    std::vector<int> GetActiveFilters() const { return m_activeFilters; }

private:
    void OnColumnClick(wxListEvent& event);
    void OnFilterClick(wxCommandEvent& event);
    void OnTimeFilterApply(wxCommandEvent& event);
    void OnTimeFilterClear(wxCommandEvent& event);
    void SetupColumns();
    bool ShouldShowLog(const LogEntry& log);

    wxListCtrl* m_listCtrl;
    std::vector<LogEntry> m_allLogs;

    // Множественные фильтры
    std::vector<int> m_activeFilters; // -1 означает "Все"

    // Фильтр по времени
    wxTimePickerCtrl* m_timeFrom;
    wxTimePickerCtrl* m_timeTo;
    wxCheckBox* m_enableTimeFilter;
    wxStaticText* m_timeFilterStatus;
    bool m_timeFilterEnabled;

    // Кнопки фильтров
    wxButton* m_btnAll;
    wxButton* m_btnAuth;
    wxButton* m_btnDenied;
    wxButton* m_btnUnknown;
    wxButton* m_btnSystem;

    wxColour m_successColor;
    wxColour m_errorColor;
    wxColour m_warningColor;
    wxColour m_infoColor;

    wxDECLARE_EVENT_TABLE();
};