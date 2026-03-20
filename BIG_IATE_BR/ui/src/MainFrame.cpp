#include "../include/MainFrame.h"
#include "../include/CaneraPanel.h"
#include "../include/LogPanel.h"
#include "../include/StatsPanel.h"
 

#define _T(str) wxString::FromUTF8(str)

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_CLOSE(MainFrame::OnClose)
EVT_TIMER(wxID_ANY, MainFrame::OnUpdateTime)
wxEND_EVENT_TABLE()

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, _T("BIG IATE - Система контроля доступа"),
    wxDefaultPosition, wxSize(1400, 800)) {
    SetBackgroundColour(wxColour(25, 25, 35));
    SetupUI();

    m_timer = new wxTimer(this);
    m_timer->Start(1000);

    Centre();
}

MainFrame::~MainFrame() {
    if (m_timer) m_timer->Stop();
}

void MainFrame::SetupUI() {
    // Меню
    wxMenuBar* menuBar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_EXIT, _T("Выход\tAlt+F4"));
    menuBar->Append(fileMenu, _T("Файл"));
    SetMenuBar(menuBar);

    // Статус бар
    m_statusBar = CreateStatusBar(2);
    SetStatusText(_T("Система готова"), 0);

    // Главный сплиттер
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_splitter->SetMinimumPaneSize(300);
    m_splitter->SetSashGravity(0.6);

    // Левая панель - камеры
    m_cameraPanel = new CameraPanel(m_splitter);

    // Правая панель
    wxPanel* rightPanel = new wxPanel(m_splitter);
    rightPanel->SetBackgroundColour(wxColour(30, 30, 40));

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    // Лог
    m_logPanel = new LogPanel(rightPanel);
    rightSizer->Add(m_logPanel, 1, wxEXPAND | wxALL, 5);

    // Статистика
    m_statsPanel = new StatsPanel(rightPanel);
    rightSizer->Add(m_statsPanel, 0, wxEXPAND | wxALL, 5);

    rightPanel->SetSizer(rightSizer);

    // Разделяем
    m_splitter->SplitVertically(m_cameraPanel, rightPanel, 700);

    // Центральный sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_splitter, 1, wxEXPAND);
    SetSizer(mainSizer);

    UpdateStats();
}

void MainFrame::UpdateStats() {
    if (m_logPanel && m_statsPanel) {
        m_statsPanel->UpdateStats(
            m_logPanel->GetAuthorizedCount(),
            m_logPanel->GetDeniedCount(),
            m_logPanel->GetUnknownCount(),
            m_logPanel->GetSystemCount()
        );
    }
}

void MainFrame::OnUpdateTime(wxTimerEvent& event) {
    SetStatusText(wxDateTime::Now().Format(_T("%H:%M:%S")), 1);
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnClose(wxCloseEvent& event) {
    if (wxMessageBox(_T("Выйти из программы?"), _T("Подтверждение"),
        wxYES_NO | wxICON_QUESTION) == wxYES) {
        event.Skip();
    }
    else {
        event.Veto();
    }
}