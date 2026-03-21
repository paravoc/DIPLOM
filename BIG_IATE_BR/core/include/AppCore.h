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
        // Инициализация распознавания
        bool initRecognition();

        // Управление камерами
        void cameraWorker(int cameraId, const config::CameraConfig& cfg);

        // Вспомогательные методы
        bool loadConfig(const std::string& path);
        bool initDatabase();
        bool startCameras();
        bool createGUI();

        // Данные
        config::LoadResult m_configResult;
        config::Secrets m_secrets;

        // База данных
        std::shared_ptr<db::DBConnectionPool> m_dbPool;
        std::shared_ptr<db::DBQueries> m_dbQueries;

        // Распознавание
        std::unique_ptr<recognition::FaceRecognizer> m_faceRecognizer;

        // Потоки камер
        std::vector<std::thread> m_cameraThreads;
        std::atomic<bool> m_running{ false };

        // GUI
        MainFrame* m_frame{ nullptr };

        AppCore() = default;
        ~AppCore();

        AppCore(const AppCore&) = delete;
        AppCore& operator=(const AppCore&) = delete;
    };

} // namespace bigiate::core