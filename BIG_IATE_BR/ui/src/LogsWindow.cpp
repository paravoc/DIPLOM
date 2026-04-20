// ui/src/LogsWindow.cpp
#include "../include/LogsWindow.h"
#include "../../database/include/DBQueries.h"
#include "../../database/include/DBModels.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#define _T(str) wxString::FromUTF8(str)

namespace bigiate::ui {

    enum {
        ID_SEARCH = 1000,
        ID_EXPORT,
        ID_CLEAR,
        ID_REFRESH
    };

    wxBEGIN_EVENT_TABLE(LogsWindow, wxFrame)
        EVT_BUTTON(ID_SEARCH, LogsWindow::onSearch)
        EVT_BUTTON(ID_EXPORT, LogsWindow::onExport)
        EVT_BUTTON(ID_CLEAR, LogsWindow::onClear)
        EVT_BUTTON(ID_REFRESH, LogsWindow::onRefresh)
        EVT_DATE_CHANGED(wxID_ANY, LogsWindow::onDateChanged)
        wxEND_EVENT_TABLE()

        LogsWindow::LogsWindow(wxWindow* parent, std::shared_ptr<bigiate::db::DBQueries> dbQueries)
        : wxFrame(parent, wxID_ANY, _T("Журнал событий - Администратор"),
            wxDefaultPosition, wxSize(1000, 600)),
        m_dbQueries(dbQueries) {

        SetBackgroundColour(wxColour(35, 35, 45));

        wxPanel* mainPanel = new wxPanel(this, wxID_ANY);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // ========== ПАНЕЛЬ ФИЛЬТРОВ ==========
        wxPanel* filterPanel = new wxPanel(mainPanel, wxID_ANY);
        filterPanel->SetBackgroundColour(wxColour(45, 45, 55));

        wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);

        // Поиск по имени
        wxStaticText* searchLabel = new wxStaticText(filterPanel, wxID_ANY, _T("🔍 Поиск:"));
        filterSizer->Add(searchLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

        m_searchBox = new wxTextCtrl(filterPanel, wxID_ANY, "", wxDefaultPosition, wxSize(180, 25));
        filterSizer->Add(m_searchBox, 0, wxRIGHT, 10);

        // Дата от
        wxStaticText* fromLabel = new wxStaticText(filterPanel, wxID_ANY, _T("От:"));
        filterSizer->Add(fromLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

        wxDateTime today = wxDateTime::Now();
        wxDateTime weekAgo = today;
        weekAgo.Subtract(wxDateSpan::Days(7));

        m_dateFrom = new wxDatePickerCtrl(filterPanel, wxID_ANY, weekAgo, wxDefaultPosition, wxSize(100, 25));
        filterSizer->Add(m_dateFrom, 0, wxRIGHT, 10);

        // Дата до
        wxStaticText* toLabel = new wxStaticText(filterPanel, wxID_ANY, _T("До:"));
        filterSizer->Add(toLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

        m_dateTo = new wxDatePickerCtrl(filterPanel, wxID_ANY, today, wxDefaultPosition, wxSize(100, 25));
        filterSizer->Add(m_dateTo, 0, wxRIGHT, 10);

        // Кнопки
        m_searchBtn = new wxButton(filterPanel, ID_SEARCH, _T("🔍 Найти"));
        m_searchBtn->SetBackgroundColour(wxColour(0, 120, 215));
        m_searchBtn->SetForegroundColour(*wxWHITE);
        filterSizer->Add(m_searchBtn, 0, wxRIGHT, 5);

        m_refreshBtn = new wxButton(filterPanel, ID_REFRESH, _T("🔄 Обновить"));
        m_refreshBtn->SetBackgroundColour(wxColour(80, 80, 90));
        m_refreshBtn->SetForegroundColour(*wxWHITE);
        filterSizer->Add(m_refreshBtn, 0, wxRIGHT, 5);

        m_clearBtn = new wxButton(filterPanel, ID_CLEAR, _T("🗑️ Сбросить"));
        m_clearBtn->SetBackgroundColour(wxColour(100, 60, 60));
        m_clearBtn->SetForegroundColour(*wxWHITE);
        filterSizer->Add(m_clearBtn, 0, wxRIGHT, 5);

        m_exportBtn = new wxButton(filterPanel, ID_EXPORT, _T("📤 Экспорт"));
        m_exportBtn->SetBackgroundColour(wxColour(0, 120, 215));
        m_exportBtn->SetForegroundColour(*wxWHITE);
        filterSizer->Add(m_exportBtn, 0);

        filterPanel->SetSizer(filterSizer);
        mainSizer->Add(filterPanel, 0, wxEXPAND | wxALL, 5);

        // ========== ТАБЛИЦА ЛОГОВ ==========
        m_logList = new wxListCtrl(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
        m_logList->SetBackgroundColour(wxColour(45, 45, 55));
        m_logList->SetForegroundColour(*wxWHITE);
        m_logList->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

        // Колонки
        m_logList->AppendColumn(_T("Время"), wxLIST_FORMAT_LEFT, 160);
        m_logList->AppendColumn(_T("ФИО"), wxLIST_FORMAT_LEFT, 200);
        m_logList->AppendColumn(_T("Камера"), wxLIST_FORMAT_LEFT, 80);
        m_logList->AppendColumn(_T("Результат"), wxLIST_FORMAT_LEFT, 100);
        m_logList->AppendColumn(_T("Уверенность"), wxLIST_FORMAT_LEFT, 80);
        m_logList->AppendColumn(_T("Детали"), wxLIST_FORMAT_LEFT, 250);

        mainSizer->Add(m_logList, 1, wxEXPAND | wxALL, 5);

        // ========== СТАТУС ==========
        m_statusText = new wxStaticText(mainPanel, wxID_ANY, _T("Готов"));
        m_statusText->SetForegroundColour(wxColour(150, 150, 160));
        mainSizer->Add(m_statusText, 0, wxALIGN_CENTER | wxALL, 5);

        mainPanel->SetSizer(mainSizer);

        loadLogs();

        Centre();
    }

    LogsWindow::~LogsWindow() {}

    wxString LogsWindow::getPersonName(int personId) {
        if (personId == 0) return _T("Неизвестный");

        auto it = m_personNames.find(personId);
        if (it != m_personNames.end()) {
            return wxString::FromUTF8(it->second);
        }

        if (m_dbQueries) {
            auto person = m_dbQueries->getPersonById(personId);
            if (person.has_value()) {
                wxString name = wxString::FromUTF8(person->fullName);
                m_personNames[personId] = person->fullName;
                return name;
            }
        }

        return wxString::Format(_T("ID:%d"), personId);
    }

    void LogsWindow::loadLogs(const std::string& fromDate, const std::string& toDate, const std::string& search) {
        if (!m_dbQueries) {
            m_statusText->SetLabel(_T("❌ База данных не доступна"));
            return;
        }

        m_statusText->SetLabel(_T("Загрузка..."));
        m_logList->DeleteAllItems();

        std::string from, to;

        if (fromDate.empty()) {
            from = m_dateFrom->GetValue().Format("%Y-%m-%d").ToStdString() + " 00:00:00";
            to = m_dateTo->GetValue().Format("%Y-%m-%d").ToStdString() + " 23:59:59";
        }
        else {
            from = fromDate + " 00:00:00";
            to = toDate + " 23:59:59";
        }

        // Используем метод getLogsByDateRange
        auto logsResult = m_dbQueries->getLogsByDateRange(from, to, 1000);

        if (!logsResult.has_value()) {
            m_statusText->SetLabel(wxString::Format(_T("❌ %s"), logsResult.error()));
            return;
        }

        // Получаем ссылку на вектор
        const auto& logs = logsResult.value();

        // Вектор для отфильтрованных логов
        std::vector<bigiate::db::AccessLog> filteredLogs;

        if (!search.empty()) {
            std::string searchLower = search;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

            // Обычный цикл for (без auto& для совместимости)
            for (size_t i = 0; i < logs.size(); ++i) {
                const auto& log = logs[i];
                wxString name = getPersonName(log.personId);
                std::string nameLower = name.ToStdString();
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                if (nameLower.find(searchLower) != std::string::npos) {
                    filteredLogs.push_back(log);
                }
            }
        }
        else {
            filteredLogs = logs;
        }

        // Отображаем логи
        for (size_t i = 0; i < filteredLogs.size(); ++i) {
            const auto& log = filteredLogs[i];

            wxString time = wxString::FromUTF8(log.accessTime);
            wxString name = getPersonName(log.personId);
            wxString camera = wxString::Format(_T("Камера %d"), log.cameraId);
            wxString result = log.accessGranted ? _T("✅ РАЗРЕШЕН") : _T("❌ ЗАПРЕЩЕН");
            wxString confidence = wxString::Format(_T("%.1f%%"), log.similarityScore * 100);
            wxString details = wxString::FromUTF8(log.accessReason);

            long idx = m_logList->InsertItem(m_logList->GetItemCount(), time);
            m_logList->SetItem(idx, 1, name);
            m_logList->SetItem(idx, 2, camera);
            m_logList->SetItem(idx, 3, result);
            m_logList->SetItem(idx, 4, confidence);
            m_logList->SetItem(idx, 5, details);

            if (log.accessGranted) {
                m_logList->SetItemTextColour(idx, wxColour(0, 200, 0));
            }
            else {
                m_logList->SetItemTextColour(idx, wxColour(200, 0, 0));
            }
        }

        m_statusText->SetLabel(wxString::Format(_T("Найдено записей: %d"), m_logList->GetItemCount()));
    }


    void LogsWindow::onSearch(wxCommandEvent& WXUNUSED(event)) {
        std::string from = m_dateFrom->GetValue().Format("%Y-%m-%d").ToStdString();
        std::string to = m_dateTo->GetValue().Format("%Y-%m-%d").ToStdString();
        std::string search = m_searchBox->GetValue().ToStdString();
        loadLogs(from, to, search);
    }

    void LogsWindow::onRefresh(wxCommandEvent& WXUNUSED(event)) {
        wxCommandEvent dummyEvent;
        onSearch(dummyEvent);
    }

    // ui/src/LogsWindow.cpp

    void LogsWindow::onDateChanged(wxDateEvent& WXUNUSED(event)) {
        // Вызываем поиск при изменении даты
        wxCommandEvent dummyEvent;
        onSearch(dummyEvent);
    }

    void LogsWindow::onClear(wxCommandEvent& WXUNUSED(event)) {
        m_searchBox->SetValue("");

        wxDateTime today = wxDateTime::Now();
        wxDateTime weekAgo = today;
        weekAgo.Subtract(wxDateSpan::Days(7));

        m_dateFrom->SetValue(weekAgo);
        m_dateTo->SetValue(today);

        wxCommandEvent dummyEvent;
        onSearch(dummyEvent);
    }

    void LogsWindow::onExport(wxCommandEvent& WXUNUSED(event)) {
        wxFileDialog dialog(this, _T("Сохранить лог"), "", "log.csv",
            _T("CSV файлы (*.csv)|*.csv|Текстовые файлы (*.txt)|*.txt"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (dialog.ShowModal() == wxID_OK) {
            wxString path = dialog.GetPath();

            std::ofstream file(path.ToStdString());
            if (file.is_open()) {
                file << "Время,ФИО,Камера,Результат,Уверенность,Детали\n";

                for (int i = 0; i < m_logList->GetItemCount(); ++i) {
                    file << m_logList->GetItemText(i).ToStdString() << ","
                        << m_logList->GetItemText(i, 1).ToStdString() << ","
                        << m_logList->GetItemText(i, 2).ToStdString() << ","
                        << m_logList->GetItemText(i, 3).ToStdString() << ","
                        << m_logList->GetItemText(i, 4).ToStdString() << ","
                        << m_logList->GetItemText(i, 5).ToStdString() << "\n";
                }

                file.close();
                wxMessageBox(_T("Лог сохранён!"), _T("Успех"), wxOK | wxICON_INFORMATION);
            }
            else {
                wxMessageBox(_T("Не удалось сохранить файл"), _T("Ошибка"), wxOK | wxICON_ERROR);
            }
        }
    }

} // namespace bigiate::ui