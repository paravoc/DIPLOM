#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <string>

// wxWidgets
#include <wx/image.h>  // ← ДОЛЖНО БЫТЬ ПЕРЕД ИСПОЛЬЗОВАНИЕМ wxImage

// Проектные заголовки
#include "../../configs/include/types.h"
#include "../../database/include/DBConnectionPool.h"
#include "../../database/include/DBQueries.h"

// Forward declaration
class MainFrame;

namespace bigiate::core {

    class AppCore {
    public:
        static AppCore& instance();

        [[nodiscard]] bool run(const std::string& configPath);
        void stop();
        MainFrame* getMainFrame() { return m_frame; }

    private:
        AppCore() = default;
        ~AppCore();

        AppCore(const AppCore&) = delete;
        AppCore& operator=(const AppCore&) = delete;

        bool loadConfig(const std::string& path);
        bool initDatabase();
        bool startCameras();
        bool createGUI();
        void cameraWorker(int cameraId, const config::CameraConfig& cfg);
        void updateCameraFrame(int cameraId, const wxImage& frame);  // ← теперь wxImage определён

        config::LoadResult m_configResult;
        std::shared_ptr<db::DBConnectionPool> m_dbPool;
        std::unique_ptr<db::DBQueries> m_dbQueries;
        std::vector<std::thread> m_cameraThreads;
        std::atomic<bool> m_running{ false };
        MainFrame* m_frame{ nullptr };
    };

} // namespace bigiate::core