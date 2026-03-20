#include "../include/LogPanel.h"
#include <algorithm>
#include <wx/statline.h>
#include <wx/gbsizer.h>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_FilterAuth = 1000,
    ID_FilterDenied,
    ID_FilterUnknown,
    ID_FilterSystem,
    ID_FilterClear,
    ID_TimeFilterEnable,
    ID_SliderFrom,
    ID_SliderTo
};

wxBEGIN_EVENT_TABLE(LogPanel, wxPanel)
EVT_LIST_COL_CLICK(wxID_ANY, LogPanel::OnColumnClick)
EVT_BUTTON(ID_FilterAuth, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterDenied, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterUnknown, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterSystem, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterClear, LogPanel::OnFilterClick)
EVT_CHECKBOX(ID_TimeFilterEnable, LogPanel::OnTimeSliderChanged)
EVT_SLIDER(ID_SliderFrom, LogPanel::OnTimeSliderChanged)
EVT_SLIDER(ID_SliderTo, LogPanel::OnTimeSliderChanged)
wxEND_EVENT_TABLE()

LogPanel::LogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
    m_filterAuth(false), m_filterDenied(false), m_filterUnknown(false), m_filterSystem(false),
    m_timeFilterEnabled(false) {

    m_successColor = wxColour(0, 200, 0);
    m_errorColor = wxColour(220, 60, 60);
    m_warningColor = wxColour(255, 160, 0);
    m_infoColor = wxColour(80, 160, 255);
    m_btnActiveColor = wxColour(0, 160, 255);
    m_btnNormalColor = wxColour(55, 55, 70);
    m_btnHoverColor = wxColour(75, 75, 95);

    SetBackgroundColour(wxColour(30, 30, 40));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ========== ПАНЕЛЬ ФИЛЬТРОВ ==========
    wxPanel* filterPanel = new wxPanel(this);
    filterPanel->SetBackgroundColour(wxColour(38, 38, 48));
    filterPanel->SetForegroundColour(*wxWHITE);

    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);

    // Кнопки фильтров
    m_btnAuth = new wxButton(filterPanel, ID_FilterAuth, _T("✅ Разрешённые"));
    m_btnAuth->SetBackgroundColour(m_btnNormalColor);
    m_btnAuth->SetForegroundColour(*wxWHITE);
    m_btnAuth->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    filterSizer->Add(m_btnAuth, 0, wxRIGHT, 8);

    m_btnDenied = new wxButton(filterPanel, ID_FilterDenied, _T("❌ Запрещённые"));
    m_btnDenied->SetBackgroundColour(m_btnNormalColor);
    m_btnDenied->SetForegroundColour(*wxWHITE);
    filterSizer->Add(m_btnDenied, 0, wxRIGHT, 8);

    m_btnUnknown = new wxButton(filterPanel, ID_FilterUnknown, _T("❓ Неопознанные"));
    m_btnUnknown->SetBackgroundColour(m_btnNormalColor);
    m_btnUnknown->SetForegroundColour(*wxWHITE);
    filterSizer->Add(m_btnUnknown, 0, wxRIGHT, 8);

    m_btnSystem = new wxButton(filterPanel, ID_FilterSystem, _T("⚙️ Системные"));
    m_btnSystem->SetBackgroundColour(m_btnNormalColor);
    m_btnSystem->SetForegroundColour(*wxWHITE);
    filterSizer->Add(m_btnSystem, 0, wxRIGHT, 8);

    filterSizer->AddStretchSpacer();

    m_btnClear = new wxButton(filterPanel, ID_FilterClear, _T("🗑️ Сбросить всё"));
    m_btnClear->SetBackgroundColour(wxColour(100, 60, 60));
    m_btnClear->SetForegroundColour(*wxWHITE);
    filterSizer->Add(m_btnClear, 0, wxRIGHT, 0);

    filterPanel->SetSizer(filterSizer);
    mainSizer->Add(filterPanel, 0, wxEXPAND | wxALL, 8);

    // ========== ПАНЕЛЬ ВРЕМЕННОГО ФИЛЬТРА ==========
    wxPanel* timePanel = new wxPanel(this);
    timePanel->SetBackgroundColour(wxColour(38, 38, 48));

    wxBoxSizer* timeSizer = new wxBoxSizer(wxVERTICAL);

    // Включение фильтра
    wxBoxSizer* enableSizer = new wxBoxSizer(wxHORIZONTAL);
    m_enableTimeFilter = new wxCheckBox(timePanel, ID_TimeFilterEnable, _T("⏰ Фильтр по времени"));
    m_enableTimeFilter->SetForegroundColour(*wxWHITE);
    m_enableTimeFilter->SetBackgroundColour(wxColour(38, 38, 48));
    enableSizer->Add(m_enableTimeFilter, 0, wxALIGN_CENTER_VERTICAL);
    timeSizer->Add(enableSizer, 0, wxLEFT | wxTOP | wxBOTTOM, 8);

    // Ползунки для времени
    wxStaticText* timeRangeLabel = new wxStaticText(timePanel, wxID_ANY, _T("Интервал времени:"));
    timeRangeLabel->SetForegroundColour(wxColour(180, 180, 200));
    timeSizer->Add(timeRangeLabel, 0, wxLEFT | wxRIGHT, 8);

    wxBoxSizer* sliderSizer = new wxBoxSizer(wxHORIZONTAL);

    m_labelFrom = new wxStaticText(timePanel, wxID_ANY, _T("00:00"));
    m_labelFrom->SetForegroundColour(*wxWHITE);
    m_labelFrom->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    sliderSizer->Add(m_labelFrom, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_sliderFrom = new wxSlider(timePanel, ID_SliderFrom, 0, 0, 1440, wxDefaultPosition, wxSize(200, 25));
    sliderSizer->Add(m_sliderFrom, 1, wxEXPAND | wxRIGHT, 8);

    m_labelTo = new wxStaticText(timePanel, wxID_ANY, _T("23:59"));
    m_labelTo->SetForegroundColour(*wxWHITE);
    m_labelTo->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    sliderSizer->Add(m_labelTo, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_sliderTo = new wxSlider(timePanel, ID_SliderTo, 1440, 0, 1440, wxDefaultPosition, wxSize(200, 25));
    sliderSizer->Add(m_sliderTo, 1, wxEXPAND);

    timeSizer->Add(sliderSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

    // Статус фильтра
    m_timeFilterStatus = new wxStaticText(timePanel, wxID_ANY, _T(""));
    m_timeFilterStatus->SetForegroundColour(wxColour(100, 200, 100));
    m_timeFilterStatus->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    timeSizer->Add(m_timeFilterStatus, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 5);

    timePanel->SetSizer(timeSizer);
    mainSizer->Add(timePanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // ========== ТАБЛИЦА ==========
    m_listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_listCtrl->SetBackgroundColour(wxColour(40, 40, 50));
    m_listCtrl->SetForegroundColour(*wxWHITE);

    SetupColumns();
    mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(mainSizer);

    // Устанавливаем начальные значения ползунков
    m_sliderFrom->SetValue(0);
    m_sliderTo->SetValue(1440);
    UpdateTimeLabels();

    // Тестовые данные
    AddLog(_T("15:47:32"), _T("Иванов П."), _T("Камера 1"), _T("РАЗРЕШЕН"), _T("98.5%"), 0);
    AddLog(_T("15:46:12"), _T("Петров С."), _T("Камера 1"), _T("ЗАПРЕЩЕН"), _T("Не в базе"), 1);
    AddLog(_T("15:45:03"), _T("Сидоров А."), _T("Камера 2"), _T("ВЫХОД"), _T("2.3с"), 0);
    AddLog(_T("15:44:21"), _T("Неизвестный"), _T("Камера 3"), _T("НЕ ОПОЗНАН"), _T("15:44"), 2);
    AddLog(_T("15:43:21"), _T("СИСТЕМА"), _T("Камера 3"), _T("ОТКЛЮЧЕНА"), _T("Потеря связи"), 3);
    AddLog(_T("15:42:15"), _T("Смирнова А."), _T("Камера 1"), _T("РАЗРЕШЕН"), _T("97.2%"), 0);
    AddLog(_T("15:41:08"), _T("Кузнецов Д."), _T("Камера 2"), _T("РАЗРЕШЕН"), _T("99.1%"), 0);
    AddLog(_T("15:40:02"), _T("СИСТЕМА"), _T("Камера 4"), _T("ПОДКЛЮЧЕНА"), _T("OK"), 3);
    AddLog(_T("15:39:30"), _T("Васильев И."), _T("Камера 1"), _T("ЗАПРЕЩЕН"), _T("Черный список"), 1);
    AddLog(_T("08:30:15"), _T("Утренний проход"), _T("Камера 1"), _T("РАЗРЕШЕН"), _T("96.3%"), 0);
    AddLog(_T("22:15:45"), _T("Вечерний проход"), _T("Камера 2"), _T("РАЗРЕШЕН"), _T("94.1%"), 0);

    UpdateDisplay();
}

LogPanel::~LogPanel() {}

void LogPanel::SetupColumns() {
    m_listCtrl->AppendColumn(_T("Время"), wxLIST_FORMAT_LEFT, 90);
    m_listCtrl->AppendColumn(_T("Событие"), wxLIST_FORMAT_LEFT, 150);
    m_listCtrl->AppendColumn(_T("Камера"), wxLIST_FORMAT_LEFT, 100);
    m_listCtrl->AppendColumn(_T("Результат"), wxLIST_FORMAT_LEFT, 100);
    m_listCtrl->AppendColumn(_T("Детали"), wxLIST_FORMAT_LEFT, 150);
}

void LogPanel::SetStatusText(const wxString& text) {
    // Можно передать родительскому окну, или просто сохранить
    // Пока просто выводим в консоль для отладки
    wxPrintf(_T("Статус: %s\n"), text);
}

void LogPanel::UpdateTimeLabels() {
    int fromMin = m_sliderFrom->GetValue();
    int toMin = m_sliderTo->GetValue();

    int fromHour = fromMin / 60;
    int fromMinute = fromMin % 60;
    int toHour = toMin / 60;
    int toMinute = toMin % 60;

    m_labelFrom->SetLabel(wxString::Format(_T("%02d:%02d"), fromHour, fromMinute));
    m_labelTo->SetLabel(wxString::Format(_T("%02d:%02d"), toHour, toMinute));
}

void LogPanel::AddLog(const wxString& time, const wxString& name,
    const wxString& camera, const wxString& result,
    const wxString& details, int type) {
    LogEntry entry;
    entry.time = time;
    entry.name = name;
    entry.camera = camera;
    entry.result = result;
    entry.details = details;
    entry.type = type;

    long hour = 0, minute = 0, second = 0;
    if (time.Len() >= 8) {
        hour = wxAtoi(time.Mid(0, 2));
        minute = wxAtoi(time.Mid(3, 2));
        second = wxAtoi(time.Mid(6, 2));
    }
    entry.timestamp = hour * 3600 + minute * 60 + second;

    m_allLogs.push_back(entry);

    std::sort(m_allLogs.begin(), m_allLogs.end(),
        [](const LogEntry& a, const LogEntry& b) {
            return a.timestamp > b.timestamp;
        });

    UpdateDisplay();
}

bool LogPanel::ShouldShowLog(const LogEntry& log) {
    // Фильтр по типу
    bool typeMatch = false;
    if (m_filterAuth && log.type == 0) typeMatch = true;
    if (m_filterDenied && log.type == 1) typeMatch = true;
    if (m_filterUnknown && log.type == 2) typeMatch = true;
    if (m_filterSystem && log.type == 3) typeMatch = true;

    // Если нет активных фильтров - показываем всё
    if (!m_filterAuth && !m_filterDenied && !m_filterUnknown && !m_filterSystem) {
        typeMatch = true;
    }

    if (!typeMatch) return false;

    // Фильтр по времени
    if (m_timeFilterEnabled) {
        int fromMinutes = m_sliderFrom->GetValue();
        int toMinutes = m_sliderTo->GetValue();

        int logMinutes = (log.timestamp / 60);

        if (fromMinutes <= toMinutes) {
            if (logMinutes < fromMinutes || logMinutes > toMinutes) {
                return false;
            }
        }
        else {
            // Переход через полночь
            if (logMinutes < fromMinutes && logMinutes > toMinutes) {
                return false;
            }
        }
    }

    return true;
}

void LogPanel::UpdateDisplay() {
    m_listCtrl->DeleteAllItems();

    int count = 0;
    for (const auto& log : m_allLogs) {
        if (ShouldShowLog(log)) {
            count++;
            long idx = m_listCtrl->InsertItem(m_listCtrl->GetItemCount(), log.time);
            m_listCtrl->SetItem(idx, 1, log.name);
            m_listCtrl->SetItem(idx, 2, log.camera);
            m_listCtrl->SetItem(idx, 3, log.result);
            m_listCtrl->SetItem(idx, 4, log.details);

            if (log.type == 0) m_listCtrl->SetItemTextColour(idx, m_successColor);
            else if (log.type == 1) m_listCtrl->SetItemTextColour(idx, m_errorColor);
            else if (log.type == 2) m_listCtrl->SetItemTextColour(idx, m_warningColor);
            else m_listCtrl->SetItemTextColour(idx, m_infoColor);
        }
    }

    // Обновляем статус
    SetStatusText(wxString::Format(_T("Показано событий: %d / %d"), count, (int)m_allLogs.size()));
}

void LogPanel::OnFilterClick(wxCommandEvent& event) {
    int id = event.GetId();

    if (id == ID_FilterAuth) {
        m_filterAuth = !m_filterAuth;
        m_btnAuth->SetBackgroundColour(m_filterAuth ? m_btnActiveColor : m_btnNormalColor);
    }
    else if (id == ID_FilterDenied) {
        m_filterDenied = !m_filterDenied;
        m_btnDenied->SetBackgroundColour(m_filterDenied ? m_btnActiveColor : m_btnNormalColor);
    }
    else if (id == ID_FilterUnknown) {
        m_filterUnknown = !m_filterUnknown;
        m_btnUnknown->SetBackgroundColour(m_filterUnknown ? m_btnActiveColor : m_btnNormalColor);
    }
    else if (id == ID_FilterSystem) {
        m_filterSystem = !m_filterSystem;
        m_btnSystem->SetBackgroundColour(m_filterSystem ? m_btnActiveColor : m_btnNormalColor);
    }
    else if (id == ID_FilterClear) {
        // Сброс всех фильтров
        m_filterAuth = false;
        m_filterDenied = false;
        m_filterUnknown = false;
        m_filterSystem = false;

        m_btnAuth->SetBackgroundColour(m_btnNormalColor);
        m_btnDenied->SetBackgroundColour(m_btnNormalColor);
        m_btnUnknown->SetBackgroundColour(m_btnNormalColor);
        m_btnSystem->SetBackgroundColour(m_btnNormalColor);

        m_enableTimeFilter->SetValue(false);
        m_timeFilterEnabled = false;
        m_timeFilterStatus->SetLabel(_T(""));
        m_sliderFrom->SetValue(0);
        m_sliderTo->SetValue(1440);
        UpdateTimeLabels();
    }

    UpdateDisplay();
}

void LogPanel::OnTimeSliderChanged(wxCommandEvent& event) {
    if (event.GetId() == ID_TimeFilterEnable) {
        m_timeFilterEnabled = m_enableTimeFilter->IsChecked();
    }

    // Ограничение: from не может быть больше to
    int from = m_sliderFrom->GetValue();
    int to = m_sliderTo->GetValue();

    if (from > to) {
        if (event.GetId() == ID_SliderFrom) {
            m_sliderTo->SetValue(from);
        }
        else if (event.GetId() == ID_SliderTo) {
            m_sliderFrom->SetValue(to);
        }
    }

    UpdateTimeLabels();

    if (m_timeFilterEnabled) {
        int fromMin = m_sliderFrom->GetValue();
        int toMin = m_sliderTo->GetValue();
        int fromHour = fromMin / 60;
        int fromMinute = fromMin % 60;
        int toHour = toMin / 60;
        int toMinute = toMin % 60;

        wxString status = wxString::Format(_T("⏰ Фильтр активен: %02d:%02d - %02d:%02d"),
            fromHour, fromMinute, toHour, toMinute);
        m_timeFilterStatus->SetLabel(status);
    }
    else {
        m_timeFilterStatus->SetLabel(_T(""));
    }

    UpdateDisplay();
}

void LogPanel::OnColumnClick(wxListEvent& event) {
    int col = event.GetColumn();

    std::sort(m_allLogs.begin(), m_allLogs.end(),
        [col](const LogEntry& a, const LogEntry& b) {
            if (col == 0) return a.timestamp > b.timestamp;
            if (col == 1) return a.name < b.name;
            if (col == 2) return a.camera < b.camera;
            if (col == 3) return a.result < b.result;
            return false;
        });

    UpdateDisplay();
}

int LogPanel::GetAuthorizedCount() const {
    int count = 0;
    for (const auto& log : m_allLogs) if (log.type == 0) count++;
    return count;
}

int LogPanel::GetDeniedCount() const {
    int count = 0;
    for (const auto& log : m_allLogs) if (log.type == 1) count++;
    return count;
}

int LogPanel::GetUnknownCount() const {
    int count = 0;
    for (const auto& log : m_allLogs) if (log.type == 2) count++;
    return count;
}

int LogPanel::GetSystemCount() const {
    int count = 0;
    for (const auto& log : m_allLogs) if (log.type == 3) count++;
    return count;
}