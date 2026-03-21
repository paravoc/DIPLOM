#include "../include/CameraPanel.h"   // ← исправлено: Canera → Camera
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <ranges>
#include <cmath>

namespace bigiate::ui {

    // ============================================================
    // ОПРЕДЕЛЕНИЕ ЦВЕТОВ
    // ============================================================

    const wxColour CameraPanel::BG_COLOR{ 25, 25, 35 };
    const wxColour CameraPanel::PANEL_COLOR{ 40, 40, 50 };
    const wxColour CameraPanel::VIDEO_COLOR{ 20, 20, 30 };
    const wxColour CameraPanel::ONLINE_COLOR{ 0, 200, 0 };
    const wxColour CameraPanel::OFFLINE_COLOR{ 200, 0, 0 };
    const wxColour CameraPanel::TEXT_COLOR{ 240, 240, 245 };

    // ============================================================
    // ТАБЛИЦА СОБЫТИЙ (пустая, но нужна для wxDECLARE_EVENT_TABLE)
    // ============================================================

    wxBEGIN_EVENT_TABLE(CameraPanel, wxPanel)
        // здесь будут события, если нужны
        wxEND_EVENT_TABLE()

        // ============================================================
        // ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
        // ============================================================

        CameraPanel::GridSize CameraPanel::calculateGridSize(size_t cameraCount) noexcept {
        // Маппинг количества камер на сетку
        if (cameraCount <= 1) return { 1, 1 };
        if (cameraCount <= 2) return { 2, 1 };
        if (cameraCount <= 3) return { 3, 1 };
        if (cameraCount <= 4) return { 2, 2 };
        if (cameraCount <= 9) return { 3, 3 };
        if (cameraCount <= 16) return { 4, 4 };

        // Для большего количества камер — квадратная сетка
        int size = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(cameraCount))));
        return { size, size };
    }

    // ============================================================
    // КОНСТРУКТОР
    // ============================================================

    CameraPanel::CameraPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
        SetBackgroundColour(BG_COLOR);
        SetDoubleBuffered(true);
    }

    // ============================================================
    // СОЗДАНИЕ КАМЕР
    // ============================================================

    std::expected<void, wxString> CameraPanel::createCameras(const std::vector<config::CameraConfig>& cameras){        clearCameras();

        // Фильтруем только включённые камеры
        std::vector<config::CameraConfig> enabledCameras;
        for (const auto& cam : cameras) {
            if (cam.enabled) {
                enabledCameras.push_back(cam);
            }
        }

        size_t count = enabledCameras.size();

        if (count == 0) {
            createEmptyPlaceholder();
            return {};
        }

        // Определяем сетку
        auto [rows, cols] = calculateGridSize(count);
        m_grid = new wxGridSizer(rows, cols, 10, 10);

        // Создаём камеры
        for (size_t i = 0; i < count; ++i) {
            auto result = createCameraWidget(enabledCameras[i], static_cast<int>(i));
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }
            m_cameras.push_back(std::move(result.value()));
        }

        // Заполняем пустые ячейки
        size_t totalCells = rows * cols;
        for (size_t i = count; i < totalCells; ++i) {
            auto* empty = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            empty->SetBackgroundColour(BG_COLOR);
            m_grid->Add(empty, 1, wxEXPAND | wxALL, 5);
        }

        relayout();
        return {};
    }

    // ============================================================
    // СОЗДАНИЕ ОДНОЙ КАМЕРЫ
    // ============================================================

    std::expected<CameraWidget, wxString> CameraPanel::createCameraWidget(
        const config::CameraConfig& cfg, int /*index*/) {

        CameraWidget widget;
        widget.id = cfg.id;

        // Создаём панель камеры
        widget.panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        widget.panel->SetBackgroundColour(PANEL_COLOR);

        auto* panelSizer = new wxBoxSizer(wxVERTICAL);


        wxString name = wxString::FromUTF8(cfg.name.c_str());
        widget.title = new wxStaticText(widget.panel, wxID_ANY,
            wxString::Format(_T("%s (ID: %d)"), name, cfg.id));

        widget.title->SetForegroundColour(TEXT_COLOR);
        widget.title->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        panelSizer->Add(widget.title, 0, wxALIGN_CENTER | wxTOP, 10);

        // Область видео
        widget.videoArea = new wxPanel(widget.panel, wxID_ANY, wxDefaultPosition, wxSize(280, 180));
        widget.videoArea->SetBackgroundColour(VIDEO_COLOR);
        widget.videoArea->SetMinSize(wxSize(280, 180));
        panelSizer->Add(widget.videoArea, 0, wxALIGN_CENTER | wxALL, 5);

        // Статус
        widget.status = new wxStaticText(widget.panel, wxID_ANY, _T("⏺ Подключение..."));
        widget.status->SetForegroundColour(wxColour(255, 200, 0));
        panelSizer->Add(widget.status, 0, wxALIGN_CENTER | wxBOTTOM, 10);

        widget.panel->SetSizer(panelSizer);
        m_grid->Add(widget.panel, 1, wxEXPAND | wxALL, 5);

        return widget;
    }

    // ============================================================
    // ПУСТАЯ ЗАГЛУШКА
    // ============================================================

    void CameraPanel::createEmptyPlaceholder() {
        auto* sizer = new wxBoxSizer(wxVERTICAL);
        auto* text = new wxStaticText(this, wxID_ANY,
            _T("Нет активных камер\nДобавьте камеры в конфиг"));
        text->SetForegroundColour(TEXT_COLOR);
        text->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        sizer->Add(text, 1, wxALIGN_CENTER);
        SetSizer(sizer);
    }

    // ============================================================
    // ОБНОВЛЕНИЕ СТАТУСА
    // ============================================================

    void CameraPanel::updateCameraStatus(int cameraId, bool online) {
        for (auto& cam : m_cameras) {
            if (cam.id == cameraId) {
                const wxString statusText = online ? _T("✅ Online") : _T("🔴 Offline");
                const wxColour statusColor = online ? ONLINE_COLOR : OFFLINE_COLOR;
                cam.status->SetLabel(statusText);
                cam.status->SetForegroundColour(statusColor);
                break;
            }
        }
    }

    // ============================================================
    // ОБНОВЛЕНИЕ КАДРА
    // ============================================================

    void CameraPanel::updateCameraFrame(int cameraId, const wxImage& frame) {
        for (auto& cam : m_cameras) {
            if (cam.id == cameraId) {
                wxSize videoSize = cam.videoArea->GetClientSize();
                if (videoSize.GetWidth() <= 0 || videoSize.GetHeight() <= 0) {
                    videoSize.Set(280, 180);
                }

                wxImage scaled = frame.Scale(videoSize.GetWidth(), videoSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
                wxBitmap bitmap(scaled);

                if (!cam.videoBitmap) {
                    cam.videoBitmap = new wxStaticBitmap(cam.videoArea, wxID_ANY, bitmap);
                    auto* sizer = new wxBoxSizer(wxVERTICAL);
                    sizer->Add(cam.videoBitmap, 1, wxEXPAND);
                    cam.videoArea->SetSizer(sizer);
                }
                else {
                    cam.videoBitmap->SetBitmap(bitmap);
                }

                cam.videoArea->Refresh();
                break;
            }
        }
    }

    // ============================================================
    // ОЧИСТКА
    // ============================================================

    void CameraPanel::clearCameras() noexcept {
        m_cameras.clear();
        if (m_grid) {
            m_grid->Clear(true);
            delete m_grid;
            m_grid = nullptr;
        }

        // Удаляем все дочерние окна
        wxWindowList children = GetChildren();
        for (wxWindow* child : children) {
            child->Destroy();
        }
    }

    // ============================================================
    // ОБНОВЛЕНИЕ МАКЕТА
    // ============================================================

    void CameraPanel::relayout() {
        auto* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Заголовок с количеством камер
        auto* header = new wxStaticText(this, wxID_ANY,
            wxString::Format(_T("ВИДЕОНАБЛЮДЕНИЕ (%zu камер)"), m_cameras.size()));
        header->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        header->SetForegroundColour(TEXT_COLOR);
        mainSizer->Add(header, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

        mainSizer->Add(m_grid, 1, wxEXPAND | wxALL, 10);

        SetSizer(mainSizer);
        Layout();
    }

    // ============================================================
    // МЕТОДЫ CameraWidget
    // ============================================================

    void CameraWidget::setOnline(bool online) {
        const wxString text = online ? _T("✅ Online") : _T("🔴 Offline");
        const wxColour color = online ? wxColour(0, 200, 0) : wxColour(200, 0, 0);
        status->SetLabel(text);
        status->SetForegroundColour(color);
    }

    void CameraWidget::updateFrame(const wxImage& frame) {
        wxSize videoSize = videoArea->GetClientSize();
        if (videoSize.GetWidth() <= 0) videoSize.Set(280, 180);
        if (videoSize.GetHeight() <= 0) videoSize.Set(180, 180);

        wxImage scaled = frame.Scale(videoSize.GetWidth(), videoSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
        wxBitmap bitmap(scaled);

        if (!videoBitmap) {
            videoBitmap = new wxStaticBitmap(videoArea, wxID_ANY, bitmap);
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(videoBitmap, 1, wxEXPAND);
            videoArea->SetSizer(sizer);
        }
        else {
            videoBitmap->SetBitmap(bitmap);
        }
        videoArea->Refresh();
    }

    void CameraWidget::setTitle(const wxString& name) {
        title->SetLabel(wxString::Format("%s (ID: %d)", name, id));
    }

} // namespace bigiate::ui