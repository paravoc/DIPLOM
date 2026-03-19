#include "../include/MainFrame.h"
#include <wx/aboutdlg.h>
#include <wx/listctrl.h>     // для wxListCtrl
#include <wx/timer.h>        // для wxTimer
#include <wx/splitter.h>     // для wxSplitterWindow
#include <wx/stattext.h>     // для wxStaticText
#include <wx/sizer.h>        // для wxBoxSizer

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_Settings, MainFrame::OnSettings)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 700)) {

    // Устанавливаем иконку (позже)
    SetIcon(wxIcon("main_icon"));

    // Создаём меню
    m_menuBar = new wxMenuBar();

    // Меню "Файл"
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_Settings, "Настройки...\tCtrl+S", "Настройки системы");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Выход\tAlt+F4", "Выход из программы");

    // Меню "Справка"
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "О программе\tF1", "Информация о программе");

    m_menuBar->Append(fileMenu, "&Файл");
    m_menuBar->Append(helpMenu, "&Справка");
    SetMenuBar(m_menuBar);

    // Создаём статус-бар
    m_statusBar = CreateStatusBar(3);
    SetStatusText("Готов к работе", 0);

    // Обновляем время каждую секунду
    wxTimer* timer = new wxTimer(this);
    timer->Start(1000);

    // Главный сплиттер (камеры + лог)
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D);

    // Панель камер (сетка 2x2)
    m_cameraPanel = new wxPanel(m_mainSplitter, wxID_ANY);
    wxGridSizer* cameraGrid = new wxGridSizer(2, 2, 5, 5);

    // Временно создаём заглушки для камер
    for (int i = 1; i <= 4; i++) {
        wxPanel* camPanel = new wxPanel(m_cameraPanel, wxID_ANY, wxDefaultPosition, wxSize(400, 225));
        camPanel->SetBackgroundColour(wxColour(30, 30, 30)); // Тёмно-серый

        wxBoxSizer* camSizer = new wxBoxSizer(wxVERTICAL);

        // Название камеры
        wxStaticText* camTitle = new wxStaticText(camPanel, wxID_ANY,
            wxString::Format("Камера %d", i),
            wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        camTitle->SetForegroundColour(*wxWHITE);
        camTitle->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

        // Заглушка видео (чёрный прямоугольник)
        wxPanel* video = new wxPanel(camPanel, wxID_ANY, wxDefaultPosition, wxSize(380, 180));
        video->SetBackgroundColour(*wxBLACK);

        // Статус
        wxStaticText* status = new wxStaticText(camPanel, wxID_ANY, "⏺ Online",
            wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        status->SetForegroundColour(wxColour(0, 255, 0)); // Зелёный

        camSizer->Add(camTitle, 0, wxALIGN_CENTER | wxTOP, 5);
        camSizer->Add(video, 0, wxALIGN_CENTER | wxALL, 5);
        camSizer->Add(status, 0, wxALIGN_CENTER | wxBOTTOM, 5);

        camPanel->SetSizer(camSizer);
        cameraGrid->Add(camPanel, 1, wxEXPAND | wxALL, 5);
    }

    m_cameraPanel->SetSizer(cameraGrid);

    // Панель лога событий
    m_logPanel = new wxPanel(m_mainSplitter, wxID_ANY);
    wxBoxSizer* logSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* logTitle = new wxStaticText(m_logPanel, wxID_ANY, "ПОСЛЕДНИЕ СОБЫТИЯ");
    logTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    // Таблица событий
    wxListCtrl* logList = new wxListCtrl(m_logPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);

    // Добавляем колонки
    logList->AppendColumn("Время", wxLIST_FORMAT_LEFT, 80);
    logList->AppendColumn("Событие", wxLIST_FORMAT_LEFT, 250);
    logList->AppendColumn("Камера", wxLIST_FORMAT_LEFT, 100);
    logList->AppendColumn("Результат", wxLIST_FORMAT_LEFT, 100);

    // Тестовые данные
    long index = logList->InsertItem(0, "15:47:32");
    logList->SetItem(index, 1, "Иванов П.");
    logList->SetItem(index, 2, "Камера 1");
    logList->SetItem(index, 3, "РАЗРЕШЕН");

    index = logList->InsertItem(1, "15:46:12");
    logList->SetItem(index, 1, "Петров С.");
    logList->SetItem(index, 2, "Камера 1");
    logList->SetItem(index, 3, "ЗАПРЕЩЕН");

    index = logList->InsertItem(2, "15:45:03");
    logList->SetItem(index, 1, "Сидоров А.");
    logList->SetItem(index, 2, "Камера 2");
    logList->SetItem(index, 3, "ВЫХОД");

    index = logList->InsertItem(3, "15:43:21");
    logList->SetItem(index, 1, "СИСТЕМА");
    logList->SetItem(index, 2, "Камера 3");
    logList->SetItem(index, 3, "ОТКЛЮЧЕНА");

    logSizer->Add(logTitle, 0, wxALL, 5);
    logSizer->Add(logList, 1, wxEXPAND | wxALL, 5);

    m_logPanel->SetSizer(logSizer);

    // Разделяем главное окно (70% камеры, 30% лог)
    m_mainSplitter->SplitHorizontally(m_cameraPanel, m_logPanel, 450);
    m_mainSplitter->SetMinimumPaneSize(200);

    // Центральный sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND);
    SetSizer(mainSizer);

    // Центрируем окно
    Centre();
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnSettings(wxCommandEvent& event) {
    wxMessageBox("Окно настроек будет здесь", "Настройки", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName("BIG IATE");
    info.SetVersion("1.0");
    info.SetDescription("Система контроля доступа с распознаванием лиц");
    info.SetCopyright("(C) 2026");
    wxAboutBox(info);
}

void MainFrame::OnClose(wxCloseEvent& event) {
    if (wxMessageBox("Выйти из программы?", "Подтверждение",
        wxYES_NO | wxICON_QUESTION) == wxYES) {
        event.Skip();
    }
    else {
        event.Veto();
    }
}