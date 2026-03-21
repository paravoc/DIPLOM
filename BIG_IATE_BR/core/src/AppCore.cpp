//==============================================================================
// BIG IATE - Core Application Manager Implementation
// AppCore.cpp
//==============================================================================
// Описание: Реализация главного класса управления приложением.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#include "../../core/include/AppCore.h"
#include "../../configs/include/loader.h"
#include "../../ui/include/MainFrame.h"
#include <wx/image.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

namespace bigiate::core {

    //==============================================================================
    // ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ: СОЗДАНИЕ ЗАГЛУШКИ "NO SIGNAL"
    //==============================================================================
    static wxImage createNoSignalImage(int width, int height, int frameCount) {
        wxImage img(width, height);

        // Тёмно-серый фон
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                img.SetRGB(x, y, 40, 40, 50);
            }
        }

        // Мигающая надпись (каждые 30 кадров меняет цвет)
        bool blink = (frameCount / 30) % 2;
        int startX = width / 2 - 80;
        int endX = width / 2 + 80;
        int startY = height / 2 - 15;
        int endY = height / 2 + 15;

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    img.SetRGB(x, y, blink ? 200 : 100, 50, 50);
                }
            }
        }

        return img;
    }

    //==============================================================================
    // ГЛАВНАЯ ФУНКЦИЯ ЗАПУСКА
    //==============================================================================
    bool AppCore::run(const std::string& configPath) {
        std::cout << "========================================" << std::endl;
        std::cout << "=== BIG IATE - СИСТЕМА КОНТРОЛЯ ДОСТУПА ===" << std::endl;
        std::cout << "========================================" << std::endl;

        if (!loadConfig(configPath)) return false;
        if (!initDatabase()) {
            std::cerr << "⚠️ База данных не подключена, но продолжаем..." << std::endl;
        }
        if (!initRecognition()) {
            std::cerr << "⚠️ Распознавание не инициализировано" << std::endl;
        }
        if (!createGUI()) return false;
        if (!startCameras()) {
            std::cerr << "⚠️ Камеры не запущены" << std::endl;
        }

        std::cout << "\n✅ СИСТЕМА ГОТОВА К РАБОТЕ" << std::endl;
        return true;
    }

    //==============================================================================
    // ЗАГРУЗКА КОНФИГУРАЦИИ
    //==============================================================================
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

    //==============================================================================
    // ИНИЦИАЛИЗАЦИЯ РАСПОЗНАВАНИЯ
    //==============================================================================
    bool AppCore::initRecognition() {
        std::cout << "\n🧠 Инициализация распознавания..." << std::endl;

        m_faceRecognizer = std::make_unique<recognition::FaceRecognizer>();

        auto result = m_faceRecognizer->init(
            m_configResult.config.recognition,
            m_dbQueries
        );

        if (!result.has_value()) {
            std::cerr << "❌ " << result.error() << std::endl;
            return false;
        }

        std::cout << "✅ Распознавание готово" << std::endl;
        return true;
    }

    //==============================================================================
    // ИНИЦИАЛИЗАЦИЯ БАЗЫ ДАННЫХ
    //==============================================================================
    bool AppCore::initDatabase() {
        std::cout << "\n📊 Инициализация базы данных..." << std::endl;

        const auto& dbCfg = m_configResult.config.database;
        std::cout << "   Хост: " << dbCfg.host << std::endl;
        std::cout << "   Порт: " << dbCfg.port << std::endl;
        std::cout << "   БД: " << dbCfg.name << std::endl;

        std::string dbPassword = m_secrets.database_password;

        if (dbPassword.empty()) {
            std::cerr << "⚠️ Database password is empty!" << std::endl;
            return false;
        }

        try {
            m_dbPool = std::make_shared<db::DBConnectionPool>(dbCfg, dbPassword, 2, 4);
            m_dbQueries = std::make_shared<db::DBQueries>(m_dbPool);

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

    //==============================================================================
    // СОЗДАНИЕ GUI
    //==============================================================================
    bool AppCore::createGUI() {
        std::cout << "\n🖥️ Создание интерфейса..." << std::endl;

        m_frame = new MainFrame();
        if (!m_frame) {
            std::cerr << "❌ Не удалось создать окно" << std::endl;
            return false;
        }

        // Передаём DBQueries в MainFrame для доступа к логам
        m_frame->setDBQueries(m_dbQueries);
        m_frame->SetCameras(m_configResult.config.cameras);
        m_frame->Show(true);
        m_frame->Raise();

        std::cout << "✅ Окно создано" << std::endl;
        return true;
    }

    //==============================================================================
    // ЗАПУСК КАМЕР
    //==============================================================================
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

    //==============================================================================
    // ОБРАБОТЧИК КАМЕРЫ (основной цикл захвата и распознавания)
    //==============================================================================
    void AppCore::cameraWorker(int cameraId, const config::CameraConfig& cfg) {
        std::cout << "🎥 Поток камеры " << cameraId << " запущен" << std::endl;

        int width = cfg.capture.width;
        int height = cfg.capture.height;
        int fps = cfg.capture.fps;
        int skipFrames = m_configResult.config.recognition.performance.skip_frames;
        int frameCount = 0;

        // Формируем источник с учётом секретов (авторизация)
        std::string source;
        bool useAuth = false;
        std::string username;
        std::string password;

        auto it = m_secrets.cameras.find(cameraId);
        if (it != m_secrets.cameras.end()) {
            const auto& camSecrets = it->second;
            if (camSecrets.username.has_value() && camSecrets.password.has_value()) {
                useAuth = true;
                username = camSecrets.username.value();
                password = camSecrets.password.value();
            }
        }

        if (cfg.connection.protocol == "usb" && cfg.connection.device.has_value()) {
            source = cfg.connection.device.value();
        }
        else {
            if (useAuth) {
                source = cfg.connection.protocol + "://" +
                    username + ":" + password + "@" +
                    cfg.connection.host + ":" +
                    std::to_string(cfg.connection.port) +
                    cfg.connection.path;
            }
            else {
                source = cfg.connection.protocol + "://" +
                    cfg.connection.host + ":" +
                    std::to_string(cfg.connection.port) +
                    cfg.connection.path;
            }
        }

        std::cout << "   Источник: " << source << std::endl;

        cv::VideoCapture cap;
        bool cameraOnline = false;
        int reconnectDelay = 1000;
        int reconnectAttempts = 0;

        // Открываем камеру
        if (cfg.connection.protocol == "usb") {
            int device = 0;
            try { device = std::stoi(source); }
            catch (...) { device = 0; }
            cap.open(device);
            std::cout << "   Открываю USB камеру: device " << device << std::endl;
        }
        else {
            cap.open(source);
            std::cout << "   Открываю RTSP камеру: " << source << std::endl;
        }

        if (cap.isOpened()) {
            cameraOnline = true;
            cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
            cap.set(cv::CAP_PROP_FPS, fps);
            std::cout << "   ✅ Камера " << cameraId << " подключена!" << std::endl;

            wxTheApp->CallAfter([this, cameraId]() {
                if (m_frame) m_frame->UpdateCameraStatus(cameraId, true);
                });
        }
        else {
            std::cerr << "   ❌ Камера " << cameraId << " не подключена" << std::endl;
            wxTheApp->CallAfter([this, cameraId]() {
                if (m_frame) m_frame->UpdateCameraStatus(cameraId, false);
                });
        }

        // Основной цикл обработки кадров
        while (m_running) {
            if (cameraOnline) {
                cv::Mat frame;
                cap >> frame;

                if (frame.empty()) {
                    std::cerr << "⚠️ Камера " << cameraId << " потеряла сигнал" << std::endl;
                    cameraOnline = false;
                    cap.release();
                    reconnectDelay = 1000;
                    reconnectAttempts = 0;

                    wxTheApp->CallAfter([this, cameraId]() {
                        if (m_frame) m_frame->UpdateCameraStatus(cameraId, false);
                        });

                    wxImage dummy = createNoSignalImage(width, height, frameCount);
                    wxTheApp->CallAfter([this, cameraId, dummy]() {
                        if (m_frame) m_frame->UpdateCameraFrame(cameraId, dummy);
                        });

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                frameCount++;

                // ============================================================
                // РАСПОЗНАВАНИЕ ЛИЦ (каждый skipFrames-й кадр)
                // ============================================================
                if (m_faceRecognizer && (frameCount % skipFrames == 0)) {
                    auto results = m_faceRecognizer->recognize(frame, cameraId, 0.75f);

                    for (const auto& result : results) {
                        if (result.match.has_value()) {
                            const auto& match = result.match.value();

                            // Зелёная рамка для своих
                            cv::rectangle(frame, result.detection.bbox, cv::Scalar(0, 255, 0), 2);

                            // Выводим имя и уверенность
                            std::string label = match.fullName + " (" +
                                std::to_string(static_cast<int>(match.similarity * 100)) + "%)";
                            cv::putText(frame, label,
                                cv::Point(result.detection.bbox.x, result.detection.bbox.y - 5),
                                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

                            std::cout << "👤 Камера " << cameraId
                                << ": " << match.fullName
                                << " (" << match.similarity * 100 << "%)" << std::endl;

                            // Запись в журнал
                            if (m_dbQueries) {
                                db::AccessLog log;
                                log.personId = match.personId;
                                log.cameraId = cameraId;
                                log.gateId = "gate_" + std::to_string(cameraId);
                                log.direction = "enter";
                                log.accessGranted = result.accessGranted;
                                log.accessReason = result.reason;
                                log.similarityScore = match.similarity;
                                log.encodingId = match.encodingId;

                                auto logResult = m_dbQueries->addAccessLog(log);
                                if (!logResult.has_value()) {
                                    std::cerr << "⚠️ Не удалось записать лог: " << logResult.error() << std::endl;
                                }
                            }

                        }
                        else {
                            // Красная рамка для неопознанных
                            cv::rectangle(frame, result.detection.bbox, cv::Scalar(0, 0, 255), 2);
                            cv::putText(frame, "Unknown",
                                cv::Point(result.detection.bbox.x, result.detection.bbox.y - 5),
                                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);

                            std::cout << "❓ Камера " << cameraId << ": Неизвестное лицо" << std::endl;

                            // Запись для неизвестного
                            if (m_dbQueries) {
                                db::AccessLog log;
                                log.personId = 0;
                                log.cameraId = cameraId;
                                log.gateId = "gate_" + std::to_string(cameraId);
                                log.direction = "enter";
                                log.accessGranted = false;
                                log.accessReason = "Person not recognized";
                                log.similarityScore = 0.0f;

                                auto logResult = m_dbQueries->addAccessLog(log);
                                if (!logResult.has_value()) {
                                    std::cerr << "⚠️ Не удалось записать лог: " << logResult.error() << std::endl;
                                }
                            }
                        }
                    }
                }

                // Отправка кадра в GUI
                cv::Mat rgb;
                cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
                wxImage wxFrame(rgb.cols, rgb.rows, rgb.data, true);
                wxImage copy = wxFrame.Copy();

                wxTheApp->CallAfter([this, cameraId, copy]() {
                    if (m_frame) m_frame->UpdateCameraFrame(cameraId, copy);
                    });

            }
            else {
                // Камера не онлайн — показываем заглушку и пробуем переподключиться
                std::this_thread::sleep_for(std::chrono::milliseconds(reconnectDelay));
                reconnectAttempts++;

                frameCount++;
                wxImage dummy = createNoSignalImage(width, height, frameCount);
                wxTheApp->CallAfter([this, cameraId, dummy]() {
                    if (m_frame) m_frame->UpdateCameraFrame(cameraId, dummy);
                    });

                if (reconnectAttempts >= 5) {
                    reconnectAttempts = 0;
                    reconnectDelay = std::min(reconnectDelay + 500, 5000);

                    std::cout << "🔄 Камера " << cameraId << ": попытка переподключения..." << std::endl;

                    if (cfg.connection.protocol == "usb") {
                        int device = 0;
                        try { device = std::stoi(source); }
                        catch (...) { device = 0; }
                        cap.open(device);
                    }
                    else {
                        cap.open(source);
                    }

                    if (cap.isOpened()) {
                        cameraOnline = true;
                        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
                        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
                        cap.set(cv::CAP_PROP_FPS, fps);
                        std::cout << "   ✅ Камера " << cameraId << " переподключена!" << std::endl;

                        wxTheApp->CallAfter([this, cameraId]() {
                            if (m_frame) m_frame->UpdateCameraStatus(cameraId, true);
                            });

                        reconnectDelay = 1000;
                    }
                }
            }

            int delay = 1000 / fps;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }

        if (cap.isOpened()) cap.release();
        std::cout << "🛑 Поток камеры " << cameraId << " остановлен" << std::endl;
    }

    //==============================================================================
    // ДЕСТРУКТОР
    //==============================================================================
    AppCore::~AppCore() {
        stop();
    }

    //==============================================================================
    // ОСТАНОВКА ВСЕХ ПОТОКОВ
    //==============================================================================
    void AppCore::stop() {
        if (!m_running) return;

        std::cout << "\n🛑 Остановка системы..." << std::endl;
        m_running = false;

        for (auto& thread : m_cameraThreads) {
            if (thread.joinable()) thread.join();
        }
        m_cameraThreads.clear();

        if (m_dbPool) m_dbPool->close();

        std::cout << "✅ Система остановлена" << std::endl;
    }

} // namespace bigiate::core