// core/include/AppCore.h
#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include <wx/image.h>
#include "../../configs/include/types.h"
#include "../../database/include/DBConnectionPool.h"
#include "../../database/include/DBQueries.h"
#include "../../recognition/include/FaceRecognizer.h"

class MainFrame;

namespace bigiate::core {

    class AppCore {
    public:
        static AppCore& instance() {
            static AppCore core;
            return core;
        }

        [[nodiscard]] bool run(const std::string& configPath);
        void stop();
        MainFrame* getMainFrame() { return m_frame; }
        void setSecrets(const config::Secrets& secrets) { m_secrets = secrets; }


    private:
        bool initRecognition();
        std::unique_ptr<recognition::FaceRecognizer> m_faceRecognizer;
        config::Secrets m_secrets;
        AppCore() = default;
        ~AppCore();

        AppCore(const AppCore&) = delete;
        AppCore& operator=(const AppCore&) = delete;

        bool loadConfig(const std::string& path);
        bool initDatabase();
        bool startCameras();
        bool createGUI();
        void cameraWorker(int cameraId, const config::CameraConfig& cfg);

        config::LoadResult m_configResult;
        std::shared_ptr<db::DBConnectionPool> m_dbPool;
        std::shared_ptr<db::DBQueries> m_dbQueries;
        std::vector<std::thread> m_cameraThreads;
        std::atomic<bool> m_running{ false };
        MainFrame* m_frame{ nullptr };
    };

} // namespace bigiate::core