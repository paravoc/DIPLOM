#include "../include/LogPanel.h"
#include <algorithm>

#define _T(str) wxString::FromUTF8(str)

enum {
    ID_FilterAll = 1000,
    ID_FilterAuth,
    ID_FilterDenied,
    ID_FilterUnknown,
    ID_FilterSystem,
    ID_TimeFilterApply,
    ID_TimeFilterClear
};

wxBEGIN_EVENT_TABLE(LogPanel, wxPanel)
EVT_LIST_COL_CLICK(wxID_ANY, LogPanel::OnColumnClick)
EVT_BUTTON(ID_FilterAll, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterAuth, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterDenied, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterUnknown, LogPanel::OnFilterClick)
EVT_BUTTON(ID_FilterSystem, LogPanel::OnFilterClick)
EVT_BUTTON(ID_TimeFilterApply, LogPanel::OnTimeFilterApply)
EVT_BUTTON(ID_TimeFilterClear, LogPanel::OnTimeFilterClear)
wxEND_EVENT_TABLE()

LogPanel::LogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_timeFilterEnabled(false) {

    m_successColor = wxColour(0, 180, 0);
    m_errorColor = wxColour(180, 0, 0);
    m_warningColor = wxColour(255, 140, 0);
    m_infoColor = wxColour(0, 120, 215);

    SetBackgroundColour(wxColour(35, 35, 45));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ========== ПАНЕЛЬ ФИЛЬТРАЦИИ ПО ТИПУ ==========
    wxPanel* filterPanel = new wxPanel(this);
    filterPanel->SetBackgroundColour(wxColour(40, 40, 50));

    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* filterLabel = new wxStaticText(filterPanel, wxID_ANY, _T("Фильтр:"));
    filterLabel->SetForegroundColour(*wxWHITE);
    filterSizer->Add(filterLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_btnAll = new wxButton(filterPanel, ID_FilterAll, _T("📋 Все"));
    m_btnAll->SetBackgroundColour(wxColour(0, 120, 215));
    m_btnAll->SetForegroundColour(*wxWHITE);

    m_btnAuth = new wxButton(filterPanel, ID_FilterAuth, _T("✅ Разрешённые"));
    m_btnAuth->SetBackgroundColour(wxColour(0, 120, 215));
    m_btnAuth->SetForegroundColour(*wxWHITE);

    m_btnDenied = new wxButton(filterPanel, ID_FilterDenied, _T("❌ Запрещённые"));
    m_btnDenied->SetBackgroundColour(wxColour(0, 120, 215));
    m_btnDenied->SetForegroundColour(*wxWHITE);

    m_btnUnknown = new wxButton(filterPanel, ID_FilterUnknown, _T("❓ Неопознанные"));
    m_btnUnknown->SetBackgroundColour(wxColour(0, 120, 215));
    m_btnUnknown->SetForegroundColour(*wxWHITE);

    m_btnSystem = new wxButton(filterPanel, ID_FilterSystem, _T("⚙️ Системные"));
    m_btnSystem->SetBackgroundColour(wxColour(0, 120, 215));
    m_btnSystem->SetForegroundColour(*wxWHITE);

    filterSizer->Add(m_btnAll, 0, wxRIGHT, 5);
    filterSizer->Add(m_btnAuth, 0, wxRIGHT, 5);
    filterSizer->Add(m_btnDenied, 0, wxRIGHT, 5);
    filterSizer->Add(m_btnUnknown, 0, wxRIGHT, 5);
    filterSizer->Add(m_btnSystem, 0, wxRIGHT, 5);

    filterPanel->SetSizer(filterSizer);
    mainSizer->Add(filterPanel, 0, wxEXPAND | wxALL, 5);

    // ========== ПАНЕЛЬ ФИЛЬТРАЦИИ ПО ВРЕМЕНИ ==========
    wxPanel* timePanel = new wxPanel(this);
    timePanel->SetBackgroundColour(wxColour(40, 40, 50));

    wxBoxSizer* timeSizer = new wxBoxSizer(wxHORIZONTAL);

    m_enableTimeFilter = new wxCheckBox(timePanel, wxID_ANY, _T("Фильтр по времени"));
    m_enableTimeFilter->SetForegroundColour(*wxWHITE);
    timeSizer->Add(m_enableTimeFilter, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    timeSizer->Add(new wxStaticText(timePanel, wxID_ANY, _T("От:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    m_timeFrom = new wxTimePickerCtrl(timePanel, wxID_ANY, wxDateTime::Now().SetHour(0).SetMinute(0).SetSecond(0));
    timeSizer->Add(m_timeFrom, 0, wxRIGHT, 10);

    timeSizer->Add(new wxStaticText(timePanel, wxID_ANY, _T("До:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    m_timeTo = new wxTimePickerCtrl(timePanel, wxID_ANY, wxDateTime::Now().SetHour(23).SetMinute(59).SetSecond(59));
    timeSizer->Add(m_timeTo, 0, wxRIGHT, 10);

    wxButton* applyBtn = new wxButton(timePanel, ID_TimeFilterApply, _T("Применить"));
    applyBtn->SetBackgroundColour(wxColour(0, 120, 215));
    applyBtn->SetForegroundColour(*wxWHITE);
    timeSizer->Add(applyBtn, 0, wxRIGHT, 5);

    wxButton* clearBtn = new wxButton(timePanel, ID_TimeFilterClear, _T("Сбросить"));
    clearBtn->SetBackgroundColour(wxColour(100, 100, 110));
    clearBtn->SetForegroundColour(*wxWHITE);
    timeSizer->Add(clearBtn, 0, wxRIGHT, 5);

    m_timeFilterStatus = new wxStaticText(timePanel, wxID_ANY, _T(""));
    m_timeFilterStatus->SetForegroundColour(wxColour(0, 180, 0));
    timeSizer->Add(m_timeFilterStatus, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    timePanel->SetSizer(timeSizer);
    mainSizer->Add(timePanel, 0, wxEXPAND | wxALL, 5);

    // ========== ТАБЛИЦА ==========
    m_listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_listCtrl->SetBackgroundColour(wxColour(45, 45, 55));
    m_listCtrl->SetForegroundColour(*wxWHITE);

    SetupColumns();
    mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    SetSizer(mainSizer);

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
}

LogPanel::~LogPanel() {}

void LogPanel::SetupColumns() {
    m_listCtrl->AppendColumn(_T("Время"), wxLIST_FORMAT_LEFT, 90);
    m_listCtrl->AppendColumn(_T("Событие"), wxLIST_FORMAT_LEFT, 150);
    m_listCtrl->AppendColumn(_T("Камера"), wxLIST_FORMAT_LEFT, 100);
    m_listCtrl->AppendColumn(_T("Результат"), wxLIST_FORMAT_LEFT, 100);
    m_listCtrl->AppendColumn(_T("Детали"), wxLIST_FORMAT_LEFT, 150);
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

    // Вычисляем timestamp для сортировки
    long hour = 0, minute = 0, second = 0;
    if (time.Len() >= 8) {
        hour = wxAtoi(time.Mid(0, 2));
        minute = wxAtoi(time.Mid(3, 2));
        second = wxAtoi(time.Mid(6, 2));
    }
    entry.timestamp = hour * 3600 + minute * 60 + second;

    m_allLogs.push_back(entry);

    // Сортируем по времени (новые сверху)
    std::sort(m_allLogs.begin(), m_allLogs.end(),
        [](const LogEntry& a, const LogEntry& b) {
            return a.timestamp > b.timestamp;
        });

    UpdateDisplay();
}

bool LogPanel::ShouldShowLog(const LogEntry& log) {
    // Проверка фильтра по типу
    if (!m_activeFilters.empty()) {
        bool typeMatch = false;
        for (int filter : m_activeFilters) {
            if (filter == -1 || log.type == filter) {
                typeMatch = true;
                break;
            }
        }
        if (!typeMatch) return false;
    }

    // Проверка фильтра по времени
    if (m_timeFilterEnabled) {
        wxDateTime from = m_timeFrom->GetValue();
        wxDateTime to = m_timeTo->GetValue();

        long fromSec = from.GetHour() * 3600 + from.GetMinute() * 60 + from.GetSecond();
        long toSec = to.GetHour() * 3600 + to.GetMinute() * 60 + to.GetSecond();

        if (log.timestamp < fromSec || log.timestamp > toSec) {
            return false;
        }
    }

    return true;
}

void LogPanel::UpdateDisplay() {
    m_listCtrl->DeleteAllItems();

    for (const auto& log : m_allLogs) {
        if (ShouldShowLog(log)) {
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
}

void LogPanel::OnFilterClick(wxCommandEvent& event) {
    int id = event.GetId();

    if (id == ID_FilterAll) {
        // Кнопка "Все" - очищаем все фильтры
        m_activeFilters.clear();
        m_activeFilters.push_back(-1);

        // Сбрасываем подсветку всех кнопок
        m_btnAll->SetBackgroundColour(wxColour(0, 120, 215));
        m_btnAuth->SetBackgroundColour(wxColour(0, 120, 215));
        m_btnDenied->SetBackgroundColour(wxColour(0, 120, 215));
        m_btnUnknown->SetBackgroundColour(wxColour(0, 120, 215));
        m_btnSystem->SetBackgroundColour(wxColour(0, 120, 215));

        m_btnAll->SetBackgroundColour(wxColour(0, 80, 150));
    }
    else {
        // Убираем фильтр "Все" если он был
        auto it = std::find(m_activeFilters.begin(), m_activeFilters.end(), -1);
        if (it != m_activeFilters.end()) {
            m_activeFilters.erase(it);
            m_btnAll->SetBackgroundColour(wxColour(0, 120, 215));
        }

        int filterType = -1;
        wxButton* btn = nullptr;

        if (id == ID_FilterAuth) { filterType = 0; btn = m_btnAuth; }
        else if (id == ID_FilterDenied) { filterType = 1; btn = m_btnDenied; }
        else if (id == ID_FilterUnknown) { filterType = 2; btn = m_btnUnknown; }
        else if (id == ID_FilterSystem) { filterType = 3; btn = m_btnSystem; }

        // Проверяем есть ли уже этот фильтр
        auto found = std::find(m_activeFilters.begin(), m_activeFilters.end(), filterType);
        if (found != m_activeFilters.end()) {
            // Убираем фильтр
            m_activeFilters.erase(found);
            btn->SetBackgroundColour(wxColour(0, 120, 215));
        }
        else {
            // Добавляем фильтр
            m_activeFilters.push_back(filterType);
            btn->SetBackgroundColour(wxColour(0, 80, 150));
        }

        // Если фильтров не осталось - показываем всё
        if (m_activeFilters.empty()) {
            m_activeFilters.push_back(-1);
            m_btnAll->SetBackgroundColour(wxColour(0, 80, 150));
        }
    }

    UpdateDisplay();
}

void LogPanel::OnTimeFilterApply(wxCommandEvent& event) {
    if (!m_enableTimeFilter->IsChecked()) {
        m_timeFilterEnabled = false;
        m_timeFilterStatus->SetLabel(_T(""));
        UpdateDisplay();
        return;
    }

    wxDateTime from = m_timeFrom->GetValue();
    wxDateTime to = m_timeTo->GetValue();

    if (from > to) {
        wxMessageBox(_T("Время 'От' должно быть меньше времени 'До'"),
            _T("Ошибка"), wxOK | wxICON_WARNING);
        return;
    }

    m_timeFilterEnabled = true;
    wxString status = wxString::Format(_T("Фильтр: %02d:%02d - %02d:%02d"),
        from.GetHour(), from.GetMinute(),
        to.GetHour(), to.GetMinute());
    m_timeFilterStatus->SetLabel(status);

    UpdateDisplay();
}

void LogPanel::OnTimeFilterClear(wxCommandEvent& event) {
    m_enableTimeFilter->SetValue(false);
    m_timeFilterEnabled = false;
    m_timeFilterStatus->SetLabel(_T(""));
    UpdateDisplay();
}

void LogPanel::OnColumnClick(wxListEvent& event) {
    int col = event.GetColumn();

    // Сортировка по выбранной колонке
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