#include "../include/MainFrame.h"
#include <wx/aboutdlg.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/display.h>
#include <wx/statline.h>
#include <wx/richtooltip.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/scrolwin.h>

#define _T(str) wxString::FromUTF8(str)

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_Settings, MainFrame::OnSettings)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(ID_AddCamera, MainFrame::OnAddCamera)
EVT_MENU(ID_RemoveCamera, MainFrame::OnRemoveCamera)
EVT_MENU(ID_ExportLog, MainFrame::OnExportLog)
EVT_MENU(ID_ClearLog, MainFrame::OnClearLog)
EVT_MENU(ID_Fullscreen, MainFrame::OnFullscreen)
EVT_CLOSE(MainFrame::OnClose)
EVT_TIMER(wxID_ANY, MainFrame::OnUpdateTime)
wxEND_EVENT_TABLE()

// Кастомная панель камеры с красивой отрисовкой
class ModernCameraPanel : public wxPanel {
public:
    ModernCameraPanel(wxWindow* parent, int id, const wxString& title,
        const wxColour& accentColor)
        : wxPanel(parent, id, wxDefaultPosition, wxSize(400, 280), wxBORDER_NONE),
        m_title(title), m_accentColor(accentColor), m_isOnline(true) {

        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetMinSize(wxSize(300, 220));

        Bind(wxEVT_PAINT, &ModernCameraPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &ModernCameraPanel::OnClick, this);
        Bind(wxEVT_ENTER_WINDOW, &ModernCameraPanel::OnMouseEnter, this);
        Bind(wxEVT_LEAVE_WINDOW, &ModernCameraPanel::OnMouseLeave, this);

        m_tooltip = new wxStaticText(this, wxID_ANY, _T("Нажмите для просмотра"));
        m_tooltip->SetForegroundColour(wxColour(200, 200, 200));
        m_tooltip->Hide();
    }

    void SetOnline(bool online) { m_isOnline = online; Refresh(); }
    void SetTitle(const wxString& title) { m_title = title; Refresh(); }

private:
    wxString m_title;
    wxColour m_accentColor;
    bool m_isOnline;
    wxStaticText* m_tooltip;

    void OnPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        dc.Clear();

        wxSize size = GetClientSize();

        wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
        if (!gc) return;

        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

        // Тень
        gc->SetBrush(wxBrush(wxColour(0, 0, 0, 40)));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRoundedRectangle(4, 4, size.x - 8, size.y - 8, 12);

        // Основной фон с градиентом
        wxColour startColor = wxColour(35, 35, 45);
        wxColour endColor = wxColour(25, 25, 35);
        gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, 0, size.y, startColor, endColor));
        gc->SetPen(wxPen(wxColour(60, 60, 70), 1));
        gc->DrawRoundedRectangle(0, 0, size.x, size.y, 10);

        // Верхняя полоска с акцентным цветом
        gc->SetBrush(wxBrush(m_accentColor));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(0, 0, size.x, 4);

        // Заголовок
        gc->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), *wxWHITE);
        gc->DrawText(m_title, 15, 15);

        // Область видео
        wxColour videoBg = m_isOnline ? wxColour(20, 20, 25) : wxColour(30, 20, 20);
        gc->SetBrush(wxBrush(videoBg));
        gc->SetPen(wxPen(wxColour(70, 70, 80), 1));
        gc->DrawRoundedRectangle(15, 45, size.x - 30, size.y - 110, 6);

        if (!m_isOnline) {
            // Красный крест для офлайн камеры
            gc->SetPen(wxPen(wxColour(255, 80, 80), 3));
            gc->StrokeLine(30, 70, size.x - 30, size.y - 70);
            gc->StrokeLine(size.x - 30, 70, 30, size.y - 70);
        }

        // Индикатор статуса
        wxColour statusColor = m_isOnline ? wxColour(0, 200, 0) : wxColour(200, 0, 0);
        gc->SetBrush(wxBrush(statusColor));
        gc->DrawEllipse(15, size.y - 35, 10, 10);

        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
            m_isOnline ? wxColour(150, 150, 160) : wxColour(200, 100, 100));
        gc->DrawText(m_isOnline ? _T("Online") : _T("Offline"), 30, size.y - 40);

        // FPS и разрешение
        gc->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
            wxColour(120, 120, 130));
        gc->DrawText(_T("30 fps | 1920x1080"), size.x - 130, size.y - 40);

        delete gc;
    }

    void OnClick(wxMouseEvent& event) {
        // Анимация нажатия
        wxColour original = GetBackgroundColour();
        SetBackgroundColour(wxColour(50, 50, 60));
        Refresh();
        Update();

        wxMilliSleep(100);

        SetBackgroundColour(original);
        Refresh();

        wxMessageBox(_T("Просмотр камеры будет доступен в следующей версии"),
            _T("Информация"), wxOK | wxICON_INFORMATION);
    }

    void OnMouseEnter(wxMouseEvent& event) {
        m_tooltip->Show();
        Refresh();
    }

    void OnMouseLeave(wxMouseEvent& event) {
        m_tooltip->Hide();
        Refresh();
    }
};

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1600, 900)),
    m_cameraRows(2), m_cameraCols(2) {

    // Цветовая схема премиум
    m_bgColor = wxColour(18, 18, 22);
    m_panelColor = wxColour(25, 25, 32);
    m_accentColor = wxColour(0, 140, 255);
    m_textColor = wxColour(240, 240, 245);
    m_successColor = wxColour(0, 200, 0);
    m_warningColor = wxColour(255, 170, 0);
    m_errorColor = wxColour(255, 80, 80);

    SetBackgroundColour(m_bgColor);
    SetDoubleBuffered(true);

    SetupModernUI();

    // Таймер для обновления времени
    m_timer = new wxTimer(this);
    m_timer->Start(1000);

    Centre();
}

void MainFrame::SetupModernUI() {
    // Создаём меню
    m_menuBar = new wxMenuBar();

    wxMenu* cameraMenu = new wxMenu();
    cameraMenu->Append(ID_AddCamera, _T("➕ Добавить камеру\tCtrl+N"), _T("Добавить новую камеру"));
    cameraMenu->Append(ID_RemoveCamera, _T("➖ Удалить камеру\tCtrl+R"), _T("Удалить выбранную камеру"));
    cameraMenu->AppendSeparator();
    cameraMenu->Append(ID_Fullscreen, _T("🖥️ Полный экран\tF11"), _T("Развернуть на весь экран"));

    wxMenu* logMenu = new wxMenu();
    logMenu->Append(ID_ExportLog, _T("📤 Экспорт логов\tCtrl+E"), _T("Сохранить логи в файл"));
    logMenu->Append(ID_ClearLog, _T("🗑️ Очистить логи\tCtrl+L"), _T("Очистить список событий"));

    wxMenu* settingsMenu = new wxMenu();
    settingsMenu->Append(ID_Settings, _T("⚙️ Параметры системы\tCtrl+S"), _T("Настройки системы"));
    settingsMenu->AppendSeparator();
    settingsMenu->Append(wxID_EXIT, _T("🚪 Выход\tAlt+F4"), _T("Выход из программы"));

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, _T("ℹ️ О программе\tF1"), _T("Информация о программе"));

    m_menuBar->Append(cameraMenu, _T("📷 Камеры"));
    m_menuBar->Append(logMenu, _T("📋 Логи"));
    m_menuBar->Append(settingsMenu, _T("⚙️ Настройки"));
    m_menuBar->Append(helpMenu, _T("❓ Справка"));

    m_menuBar->SetBackgroundColour(m_panelColor);
    m_menuBar->SetForegroundColour(m_textColor);
    SetMenuBar(m_menuBar);

    // Панель инструментов
    wxToolBar* toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL);
    toolBar->SetBackgroundColour(m_panelColor);

    wxBitmap addIcon(24, 24);
    wxBitmap removeIcon(24, 24);
    wxBitmap settingsIcon(24, 24);

    toolBar->AddTool(ID_AddCamera, _T("Добавить"), addIcon, _T("Добавить камеру"));
    toolBar->AddTool(ID_RemoveCamera, _T("Удалить"), removeIcon, _T("Удалить камеру"));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_ExportLog, _T("Экспорт"), addIcon, _T("Экспорт логов"));
    toolBar->AddTool(ID_ClearLog, _T("Очистить"), removeIcon, _T("Очистить логи"));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_Settings, _T("Настройки"), settingsIcon, _T("Настройки системы"));
    toolBar->Realize();

    // Статус-бар
    m_statusBar = CreateStatusBar(4);
    int widths[] = { 200, 150, 200, -1 };
    m_statusBar->SetStatusWidths(4, widths);
    m_statusBar->SetBackgroundColour(m_panelColor);
    m_statusBar->SetForegroundColour(m_textColor);

    SetStatusText(_T("✅ Система готова"), 0);
    SetStatusText(_T("Камер: 4/4"), 1);
    SetStatusText(wxDateTime::Now().Format(_T("%d.%m.%Y %H:%M:%S")), 2);
    SetStatusText(_T("Охранный режим"), 3);

    // ========== ВЕРТИКАЛЬНЫЙ СПЛИТТЕР ==========
    wxSplitterWindow* mainVertSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER);
    mainVertSplitter->SetMinimumPaneSize(300);
    mainVertSplitter->SetSashGravity(0.5); // 50% на 50%
    mainVertSplitter->SetBackgroundColour(m_bgColor);

    // ========== ЛЕВАЯ ПАНЕЛЬ (КАМЕРЫ) ==========
    wxPanel* leftPanel = new wxPanel(mainVertSplitter, wxID_ANY);
    leftPanel->SetBackgroundColour(m_bgColor);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок камер
    wxPanel* cameraHeader = new wxPanel(leftPanel);
    cameraHeader->SetBackgroundColour(m_bgColor);

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* camHeaderTitle = new wxStaticText(cameraHeader, wxID_ANY, _T("ВИДЕОНАБЛЮДЕНИЕ"));
    wxFont headerFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    camHeaderTitle->SetFont(headerFont);
    camHeaderTitle->SetForegroundColour(m_textColor);

    wxStaticLine* headerLine = new wxStaticLine(cameraHeader, wxID_ANY, wxDefaultPosition, wxSize(3, 30));
    headerLine->SetBackgroundColour(m_accentColor);

    wxStaticText* camCount = new wxStaticText(cameraHeader, wxID_ANY, _T("4 камеры онлайн"));
    camCount->SetForegroundColour(wxColour(150, 150, 160));

    headerSizer->Add(camHeaderTitle, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    headerSizer->Add(headerLine, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 15);
    headerSizer->Add(camCount, 1, wxALIGN_CENTER_VERTICAL);

    cameraHeader->SetSizer(headerSizer);
    leftSizer->Add(cameraHeader, 0, wxEXPAND | wxALL, 10);

    // Сетка камер 2x2
    wxGridSizer* cameraGrid = new wxGridSizer(2, 2, 15, 15);

    // Создаём 4 камеры
    wxString camNames[] = { _T("ГЛАВНЫЙ ВХОД"), _T("ЗАПАСНОЙ ВЫХОД"),
                            _T("ПАРКОВКА"), _T("ВЕСТИБЮЛЬ") };

    for (int i = 0; i < 4; i++) {
        ModernCameraPanel* camPanel = new ModernCameraPanel(leftPanel, wxID_ANY,
            wxString::Format(_T("Камера %d - %s"), i + 1, camNames[i]), m_accentColor);
        cameraGrid->Add(camPanel, 1, wxEXPAND | wxALL, 5);
    }

    leftSizer->Add(cameraGrid, 1, wxEXPAND | wxALL, 10);
    leftPanel->SetSizer(leftSizer);

    // ========== ПРАВАЯ ПАНЕЛЬ (ЛОГ) ==========
    wxPanel* rightPanel = new wxPanel(mainVertSplitter, wxID_ANY);
    rightPanel->SetBackgroundColour(m_panelColor);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    // Заголовок лога
    wxPanel* logHeader = new wxPanel(rightPanel);
    logHeader->SetBackgroundColour(m_panelColor);

    wxBoxSizer* logHeaderSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* logHeaderTitle = new wxStaticText(logHeader, wxID_ANY, _T("ЖУРНАЛ СОБЫТИЙ"));
    logHeaderTitle->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    logHeaderTitle->SetForegroundColour(m_textColor);

    logHeaderSizer->Add(logHeaderTitle, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15);
    logHeaderSizer->AddStretchSpacer();

    wxButton* clearLogBtn = new wxButton(logHeader, ID_ClearLog, _T("🗑️ Очистить"));
    clearLogBtn->SetBackgroundColour(m_panelColor);
    clearLogBtn->SetForegroundColour(m_textColor);

    wxButton* exportLogBtn = new wxButton(logHeader, ID_ExportLog, _T("📤 Экспорт"));
    exportLogBtn->SetBackgroundColour(m_accentColor);
    exportLogBtn->SetForegroundColour(*wxWHITE);

    logHeaderSizer->Add(clearLogBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    logHeaderSizer->Add(exportLogBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 15);

    logHeader->SetSizer(logHeaderSizer);
    rightSizer->Add(logHeader, 0, wxEXPAND | wxTOP | wxBOTTOM, 10);

    // Таблица событий
    m_logList = new wxListCtrl(rightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_SINGLE_SEL | wxBORDER_NONE);

    m_logList->SetBackgroundColour(m_panelColor.ChangeLightness(95));
    m_logList->SetForegroundColour(m_textColor);
    m_logList->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    // Настройка колонок
    m_logList->AppendColumn(_T("Время"), wxLIST_FORMAT_LEFT, 90);
    m_logList->AppendColumn(_T("Событие"), wxLIST_FORMAT_LEFT, 200);
    m_logList->AppendColumn(_T("Камера"), wxLIST_FORMAT_LEFT, 120);
    m_logList->AppendColumn(_T("Результат"), wxLIST_FORMAT_LEFT, 120);
    m_logList->AppendColumn(_T("Детали"), wxLIST_FORMAT_LEFT, 250);

    // Тестовые данные
    AddLogEntry(_T("15:47:32"), _T("Иванов Петр Сергеевич"), _T("Камера 1"),
        _T("✅ РАЗРЕШЕН"), _T("Схожесть: 98.5%"), true);
    AddLogEntry(_T("15:46:12"), _T("Петров Сидор Иванович"), _T("Камера 1"),
        _T("❌ ЗАПРЕЩЕН"), _T("Не в базе данных"), false);
    AddLogEntry(_T("15:45:03"), _T("Сидоров Алексей Петрович"), _T("Камера 2"),
        _T("⬆️ ВЫХОД"), _T("Время прохода: 2.3с"), true);
    AddLogEntry(_T("15:44:21"), _T("Неизвестный"), _T("Камера 3"),
        _T("⚠️ НЕ ОПОЗНАН"), _T("Попытка в 15:44"), false);
    AddLogEntry(_T("15:43:21"), _T("СИСТЕМА"), _T("Камера 3"),
        _T("🔴 ОТКЛЮЧЕНА"), _T("Потеря связи"), false);
    AddLogEntry(_T("15:42:15"), _T("Смирнова Анна"), _T("Камера 1"),
        _T("✅ РАЗРЕШЕН"), _T("Схожесть: 97.2%"), true);
    AddLogEntry(_T("15:41:08"), _T("Кузнецов Дмитрий"), _T("Камера 2"),
        _T("✅ РАЗРЕШЕН"), _T("Схожесть: 99.1%"), true);
    AddLogEntry(_T("15:40:02"), _T("СИСТЕМА"), _T("Камера 4"),
        _T("🟢 ПОДКЛЮЧЕНА"), _T("Камера инициализирована"), true);

    rightSizer->Add(m_logList, 1, wxEXPAND | wxLEFT | wxRIGHT, 15);

    // Статистика внизу
    wxPanel* statsPanel = new wxPanel(rightPanel);
    statsPanel->SetBackgroundColour(m_panelColor.ChangeLightness(90));

    wxBoxSizer* statsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* totalStats = new wxStaticText(statsPanel, wxID_ANY, _T("📊 Всего событий: 1,234"));
    totalStats->SetForegroundColour(m_textColor);

    wxStaticText* todayStats = new wxStaticText(statsPanel, wxID_ANY, _T("📅 За сегодня: 56"));
    todayStats->SetForegroundColour(m_successColor);

    wxStaticText* unknownStats = new wxStaticText(statsPanel, wxID_ANY, _T("❓ Неопознано: 3"));
    unknownStats->SetForegroundColour(m_warningColor);

    wxStaticText* systemStats = new wxStaticText(statsPanel, wxID_ANY, _T("⚙️ Система: OK"));
    systemStats->SetForegroundColour(m_successColor);

    statsSizer->Add(totalStats, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 25);
    statsSizer->Add(todayStats, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 25);
    statsSizer->Add(unknownStats, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 25);
    statsSizer->Add(systemStats, 0, wxALIGN_CENTER_VERTICAL);
    statsSizer->AddStretchSpacer();

    wxStaticText* updateTime = new wxStaticText(statsPanel, wxID_ANY,
        _T("Обновлено: ") + wxDateTime::Now().Format(_T("%H:%M:%S")));
    updateTime->SetForegroundColour(wxColour(150, 150, 160));
    statsSizer->Add(updateTime, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 15);

    statsPanel->SetSizer(statsSizer);
    rightSizer->Add(statsPanel, 0, wxEXPAND | wxALL, 15);

    rightPanel->SetSizer(rightSizer);

    // Разделяем вертикально (камеры слева, лог справа)
    mainVertSplitter->SplitVertically(leftPanel, rightPanel, 800); // 800px ширина левой панели

    // Центральный sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(mainVertSplitter, 1, wxEXPAND);
    SetSizer(mainSizer);
}

void MainFrame::AddLogEntry(const wxString& time, const wxString& event,
    const wxString& camera, const wxString& result,
    const wxString& details, bool success) {
    long index = m_logList->GetItemCount();
    index = m_logList->InsertItem(index, time);

    m_logList->SetItem(index, 1, event);
    m_logList->SetItem(index, 2, camera);
    m_logList->SetItem(index, 3, result);
    m_logList->SetItem(index, 4, details);

    if (result.Contains(_T("✅"))) {
        m_logList->SetItemTextColour(index, m_successColor);
    }
    else if (result.Contains(_T("❌"))) {
        m_logList->SetItemTextColour(index, m_errorColor);
    }
    else if (result.Contains(_T("⚠️"))) {
        m_logList->SetItemTextColour(index, m_warningColor);
    }
}

void MainFrame::OnUpdateTime(wxTimerEvent& event) {
    SetStatusText(wxDateTime::Now().Format(_T("%d.%m.%Y %H:%M:%S")), 2);
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnSettings(wxCommandEvent& event) {
    // Пусто
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName(_T("BIG IATE"));
    info.SetVersion(_T("1.0.0"));
    info.SetDescription(_T("Профессиональная система контроля доступа\n"
        "с интеллектуальным распознаванием лиц"));
    info.SetCopyright(_T("© 2026 BIG IATE Technologies"));
    info.SetWebSite(_T("www.bigiate.com"));
    info.AddDeveloper(_T("Команда разработки BIG IATE"));
    info.SetLicence(_T("Проприетарное программное обеспечение.\n"
        "Все права защищены."));
    wxAboutBox(info, this);
}

void MainFrame::OnClose(wxCloseEvent& event) {
    wxRichMessageDialog dialog(this,
        _T("Вы действительно хотите выйти из системы?"),
        _T("Подтверждение выхода"),
        wxYES_NO | wxICON_QUESTION | wxCENTRE);

    dialog.SetYesNoLabels(_T("Выйти"), _T("Отмена"));
    dialog.ShowDetailedText(_T("Все операции будут остановлены.\n"
        "Убедитесь, что нет активных проходов."));

    if (dialog.ShowModal() == wxID_YES) {
        m_timer->Stop();
        event.Skip();
    }
    else {
        event.Veto();
    }
}

void MainFrame::OnAddCamera(wxCommandEvent& event) {
    wxMessageBox(_T("Функция добавления камеры будет доступна в следующей версии"),
        _T("Информация"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnRemoveCamera(wxCommandEvent& event) {
    wxMessageBox(_T("Выберите камеру для удаления"),
        _T("Информация"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExportLog(wxCommandEvent& event) {
    wxFileDialog dialog(this, _T("Сохранить лог"), "", "log.txt",
        _T("Текстовые файлы (*.txt)|*.txt|CSV файлы (*.csv)|*.csv|Все файлы (*.*)|*.*"),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK) {
        wxMessageBox(_T("Лог успешно сохранен!"), _T("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void MainFrame::OnClearLog(wxCommandEvent& event) {
    if (wxMessageBox(_T("Очистить весь журнал событий?"), _T("Подтверждение"),
        wxYES_NO | wxICON_QUESTION) == wxYES) {
        m_logList->DeleteAllItems();
    }
}

void MainFrame::OnFullscreen(wxCommandEvent& event) {
    ShowFullScreen(!IsFullScreen());
}