#pragma once

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/statbmp.h>
#include <vector>
#include <span>
#include <expected>
#include "../../configs/include/types.h"

namespace bigiate::ui {

    // Структура для хранения информации о камере в GUI
    struct CameraWidget {
        int id;
        wxPanel* panel;
        wxStaticText* title;
        wxPanel* videoArea;
        wxStaticText* status;
        wxStaticBitmap* videoBitmap{ nullptr };

        void setOnline(bool online);
        void updateFrame(const wxImage& frame);
        void setTitle(const wxString& name);
    };

    class CameraPanel : public wxPanel {
    public:
        explicit CameraPanel(wxWindow* parent);
        ~CameraPanel() override = default;

        CameraPanel(const CameraPanel&) = delete;
        CameraPanel& operator=(const CameraPanel&) = delete;

        [[nodiscard]] std::expected<void, wxString> createCameras(const std::vector<config::CameraConfig>& cameras);

        void updateCameraStatus(int cameraId, bool online);
        void updateCameraFrame(int cameraId, const wxImage& frame);

        [[nodiscard]] size_t getCameraCount() const noexcept { return m_cameras.size(); }
        void clearCameras() noexcept;

    private:
        struct GridSize { int rows; int cols; };
        [[nodiscard]] static GridSize calculateGridSize(size_t cameraCount) noexcept;
        void createEmptyPlaceholder();
        [[nodiscard]] std::expected<CameraWidget, wxString> createCameraWidget(const config::CameraConfig& cfg, int index);
        void relayout();

        wxGridSizer* m_grid{ nullptr };
        std::vector<CameraWidget> m_cameras;

        // Цветовая схема
        static const wxColour BG_COLOR;
        static const wxColour PANEL_COLOR;
        static const wxColour VIDEO_COLOR;
        static const wxColour ONLINE_COLOR;
        static const wxColour OFFLINE_COLOR;
        static const wxColour TEXT_COLOR;

        wxDECLARE_EVENT_TABLE();
    };

} // namespace bigiate::ui