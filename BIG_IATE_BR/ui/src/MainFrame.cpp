#include "../include/MainFrame.h"
#include "../include/CameraPanel.h"
#include "../include/LogPanel.h"
#include "../include/StatsPanel.h"
#include <iostream>

#define _T(str) wxString::FromUTF8(str)

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_CLOSE(MainFrame::OnClose)
EVT_TIMER(wxID_ANY, MainFrame::OnUpdateTime)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, _T("BIG IATE - Система контроля доступа"),
        wxDefaultPosition, wxSize(1400, 800)) {
    std::cout << "🔧 MainFrame конструктор НАЧАЛ" << std::endl;
    SetBackgroundColour(wxColour(25, 25, 35));
    SetupUI();

    m_timer = new wxTimer(this);
    m_timer->Start(1000);

    Centre();
    std::cout << "🔧 MainFrame конструктор ЗАВЕРШЁН" << std::endl;
}

MainFrame::~MainFrame() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void MainFrame::SetupUI() {
    std::cout << "📐 SetupUI начат" << std::endl;

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
    m_cameraPanel = new bigiate::ui::CameraPanel(m_splitter);

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
    std::cout << "📐 SetupUI завершён" << std::endl;
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

void MainFrame::SetCameras(const std::vector<bigiate::config::CameraConfig>& cameras) {
    if (m_cameraPanel) {
        auto result = m_cameraPanel->createCameras(cameras);
        if (!result.has_value()) {
            std::cerr << "❌ Ошибка создания камер: " << result.error() << std::endl;
        }
        else {
            std::cout << "✅ Камеры созданы: " << m_cameraPanel->getCameraCount() << std::endl;
        }
    }
}

void MainFrame::UpdateCameraFrame(int cameraId, const wxImage& frame) {
    if (m_cameraPanel) {
        m_cameraPanel->updateCameraFrame(cameraId, frame);
    }
}

void MainFrame::UpdateCameraStatus(int cameraId, bool online) {
    if (m_cameraPanel) {
        m_cameraPanel->updateCameraStatus(cameraId, online);
    }
}