#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/slider.h>
#include <vector>

struct LogEntry {
    wxString time;
    long timestamp;
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

private:
    void OnColumnClick(wxListEvent& event);
    void OnFilterClick(wxCommandEvent& event);
    void OnTimeSliderChanged(wxCommandEvent& event);
    void SetupColumns();
    bool ShouldShowLog(const LogEntry& log);
    void UpdateTimeLabels();
    void SetStatusText(const wxString& text);

    wxListCtrl* m_listCtrl;
    std::vector<LogEntry> m_allLogs;

    // Фильтры по типу
    bool m_filterAuth;
    bool m_filterDenied;
    bool m_filterUnknown;
    bool m_filterSystem;

    // Фильтр по времени (ползунки)
    wxSlider* m_sliderFrom;
    wxSlider* m_sliderTo;
    wxStaticText* m_labelFrom;
    wxStaticText* m_labelTo;
    wxCheckBox* m_enableTimeFilter;
    wxStaticText* m_timeFilterStatus;  // <-- ДОБАВЛЯЕМ
    bool m_timeFilterEnabled;

    // Кнопки фильтров
    wxButton* m_btnAuth;
    wxButton* m_btnDenied;
    wxButton* m_btnUnknown;
    wxButton* m_btnSystem;
    wxButton* m_btnClear;

    // Цвета
    wxColour m_successColor;
    wxColour m_errorColor;
    wxColour m_warningColor;
    wxColour m_infoColor;
    wxColour m_btnActiveColor;
    wxColour m_btnNormalColor;
    wxColour m_btnHoverColor;

    wxDECLARE_EVENT_TABLE();
};