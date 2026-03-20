#include "../../core/include/AppCore.h"
#include "../../configs/include/loader.h"  // ← нужно для LoadConfig!
#include "../../ui/include/MainFrame.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace bigiate::core {

    // ============================================================
    // СИНГЛТОН
    // ============================================================

    AppCore& AppCore::instance() {
        static AppCore core;
        return core;
    }

    AppCore::~AppCore() {
        stop();
    }

    // ============================================================
    // ГЛАВНАЯ ФУНКЦИЯ
    // ============================================================

    bool AppCore::run(const std::string& configPath) {
        std::cout << "========================================" << std::endl;
        std::cout << "=== BIG IATE - СИСТЕМА КОНТРОЛЯ ДОСТУПА ===" << std::endl;
        std::cout << "========================================" << std::endl;

        // 1. Загрузка конфига
        if (!loadConfig(configPath)) return false;

        // 2. Инициализация базы данных
        if (!initDatabase()) {
            std::cerr << "⚠️ База данных не подключена, но продолжаем..." << std::endl;
        }

        // 3. Создание GUI
        if (!createGUI()) return false;

        // 4. Запуск камер (после GUI)
        if (!startCameras()) {
            std::cerr << "⚠️ Камеры не запущены" << std::endl;
        }

        std::cout << "\n✅ СИСТЕМА ГОТОВА К РАБОТЕ" << std::endl;
        return true;
    }

    // ============================================================
    // ЗАГРУЗКА КОНФИГА
    // ============================================================

    bool AppCore::loadConfig(const std::string& path) {
        std::cout << "📁 Загрузка конфига: " << path << std::endl;

        auto result = config::LoadConfig(path);
        if (!result.has_value()) {
            std::cerr << "❌ Ошибка: " << result.error() << std::endl;
            return false;
        }

        m_configResult = *result;
        std::cout << "✅ Версия: " << m_configResult.config.version.toString() << std::endl;
        std::cout << "   Камер: " << m_configResult.config.cameras.size() << std::endl;

        return true;
    }

    // ============================================================
    // ИНИЦИАЛИЗАЦИЯ БД
    // ============================================================

    bool AppCore::initDatabase() {
        std::cout << "\n📊 Инициализация базы данных..." << std::endl;

        const auto& dbCfg = m_configResult.config.database;
        std::cout << "   Хост: " << dbCfg.host << std::endl;
        std::cout << "   Порт: " << dbCfg.port << std::endl;
        std::cout << "   БД: " << dbCfg.name << std::endl;

        std::string dbPassword = "1234";

        try {
            m_dbPool = std::make_shared<db::DBConnectionPool>(dbCfg, dbPassword, 2, 4);
            m_dbQueries = std::make_unique<db::DBQueries>(m_dbPool);

            auto test = m_dbQueries->testConnection();
            if (test.has_value()) {
                std::cout << "✅ Подключено!" << std::endl;

                auto stats = m_dbQueries->getStats();
                if (stats.has_value()) {
                    std::cout << "   👥 Людей в БД: " << stats->totalPersons << std::endl;
                }
                return true;
            }
            else {
                std::cerr << "❌ Ошибка: " << test.error() << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "❌ Исключение: " << e.what() << std::endl;
            return false;
        }
    }

    // ============================================================
    // СОЗДАНИЕ GUI
    // ============================================================

    bool AppCore::createGUI() {
        std::cout << "\n🖥️ Создание интерфейса..." << std::endl;

        m_frame = new MainFrame();
        if (!m_frame) {
            std::cerr << "❌ Не удалось создать окно" << std::endl;
            return false;
        }

        // Передаём камеры в GUI
        m_frame->SetCameras(m_configResult.config.cameras);

        m_frame->Show(true);
        m_frame->Raise();

        std::cout << "✅ Окно создано" << std::endl;
        return true;
    }

    // ============================================================
    // ЗАПУСК КАМЕР
    // ============================================================

    bool AppCore::startCameras() {
        std::cout << "\n📷 Запуск камер..." << std::endl;

        m_running = true;
        int started = 0;

        for (const auto& cam : m_configResult.config.cameras) {
            if (!cam.enabled) {
                std::cout << "   Камера " << cam.id << " (" << cam.name << ") - отключена" << std::endl;
                continue;
            }

            m_cameraThreads.emplace_back([this, cam]() {
                this->cameraWorker(cam.id, cam);
                });

            std::cout << "   ✅ Камера " << cam.id << " запущена" << std::endl;
            started++;
        }

        std::cout << "   Запущено: " << started << "/" << m_configResult.config.cameras.size() << std::endl;
        return started > 0;
    }

    // ============================================================
    // ОБРАБОТЧИК КАМЕРЫ (заглушка)
    // ============================================================

    void AppCore::cameraWorker(int cameraId, const config::CameraConfig& cfg) {
        std::cout << "🎥 Поток камеры " << cameraId << " запущен" << std::endl;

        int width = cfg.capture.width;
        int height = cfg.capture.height;
        int frameCount = 0;

        while (m_running) {
            frameCount++;

            // Создаём тестовое изображение
            wxImage img(width, height);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    unsigned char r = (x + frameCount) % 256;
                    unsigned char g = (y + frameCount) % 256;
                    unsigned char b = (frameCount) % 256;
                    img.SetRGB(x, y, r, g, b);
                }
            }

            // Отправляем в GUI
            wxTheApp->CallAfter([this, cameraId, img]() {
                if (m_frame) {
                    m_frame->UpdateCameraFrame(cameraId, img);
                }
                });

            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / cfg.capture.fps));
        }

        std::cout << "🛑 Поток камеры " << cameraId << " остановлен" << std::endl;
    }

    // ============================================================
    // ОСТАНОВКА
    // ============================================================

    void AppCore::stop() {
        if (!m_running) return;

        std::cout << "\n🛑 Остановка системы..." << std::endl;
        m_running = false;

        for (auto& thread : m_cameraThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        m_cameraThreads.clear();

        if (m_dbPool) {
            m_dbPool->close();
        }

        std::cout << "✅ Система остановлена" << std::endl;
    }

} // namespace bigiate::core