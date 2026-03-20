#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "ui/include/MainFrame.h"
#include "configs/include/loader.h"
#include "database/include/DBConnectionPool.h"
#include "database/include/DBQueries.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace config = bigiate::config;
namespace db = bigiate::db;

// Класс приложения
class BigIateApp : public wxApp {
private:
    config::LoadResult m_configResult;
    std::shared_ptr<db::DBConnectionPool> m_dbPool;
    std::unique_ptr<db::DBQueries> m_dbQueries;
    std::vector<std::thread> m_cameraThreads;
    bool m_running = true;

public:
    virtual bool OnInit() override {
        // Настройка локали (для русского языка)
        wxLocale locale;
        locale.Init(wxLANGUAGE_RUSSIAN);

        // Загружаем конфиг
        std::string config_file = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";

        wxPrintf("Загрузка конфига: %s\n", config_file);

        auto result = config::LoadConfig(config_file);

        if (!result.has_value()) {
            wxMessageBox(
                wxString::Format("Ошибка загрузки конфига:\n%s", result.error()),
                "Ошибка",
                wxOK | wxICON_ERROR
            );
            return false;
        }
        else {
            m_configResult = *result;
            wxPrintf("Конфиг загружен: версия %s, камер: %zu\n",
                m_configResult.config.version.toString(),
                m_configResult.config.cameras.size());
        }

        // ============================================================
        // ИНИЦИАЛИЗАЦИЯ БАЗЫ ДАННЫХ
        // ============================================================

        // Получаем пароль из секретов (временно для теста)
        std::string dbPassword = "postgres"; // В реальном коде: m_configResult.secrets.database_password

        // Создаём пул соединений
        m_dbPool = std::make_shared<db::DBConnectionPool>(
            m_configResult.config.database,
            dbPassword,
            2,   // min connections
            4    // max connections (по числу турникетов)
        );

        // Создаём объект для запросов
        m_dbQueries = std::make_unique<db::DBQueries>(m_dbPool);

        // Тестируем подключение
        auto test = m_dbQueries->testConnection();
        if (!test.has_value()) {
            wxPrintf("❌ Ошибка подключения к БД: %s\n", test.error());
            // Можно продолжить без БД или выйти
        }
        else {
            wxPrintf("✅ Подключение к базе данных установлено!\n");

            // Получаем статистику
            auto stats = m_dbQueries->getStats();
            if (stats.has_value()) {
                wxPrintf("📊 Статистика БД: %d человек, %d активных\n",
                    stats->totalPersons, stats->activePersons);
            }
        }

        // ============================================================
        // ЗАПУСК ПОТОКОВ ДЛЯ КАМЕР
        // ============================================================

        // Запускаем поток для каждой активной камеры
        for (const auto& cam : m_configResult.config.cameras) {
            if (!cam.enabled) continue;

            m_cameraThreads.emplace_back([this, cam]() {
                this->cameraWorker(cam);
                });

            wxPrintf("📷 Запущен поток для камеры %d: %s\n", cam.id, cam.name);
        }

        // ============================================================
        // СОЗДАНИЕ GUI
        // ============================================================

        // Создаём главное окно и передаём конфиг
        MainFrame* frame = new MainFrame();
        frame->Show(true);

        return true;
    }

    virtual int OnExit() override {
        // Останавливаем потоки
        m_running = false;

        // Ждём завершения всех потоков
        for (auto& thread : m_cameraThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Закрываем соединения с БД
        if (m_dbPool) {
            m_dbPool->close();
        }

        return wxApp::OnExit();
    }

private:
    // Функция для обработки камеры (заглушка)
    void cameraWorker(const config::CameraConfig& cam) {
        wxPrintf("🎥 Поток камеры %d запущен\n", cam.id);

        // TODO: Здесь будет реальная работа с камерой и распознаванием
        while (m_running) {
            // Симуляция работы
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // В реальном коде здесь будет:
            // 1. Захват кадра с камеры
            // 2. Детекция лиц
            // 3. Извлечение эмбеддингов
            // 4. Поиск в БД через m_dbQueries->findPersonByEmbedding()
            // 5. Запись в журнал через m_dbQueries->addAccessLog()
            // 6. Отправка события в GUI
        }

        wxPrintf("🛑 Поток камеры %d остановлен\n", cam.id);
    }
};

wxIMPLEMENT_APP(BigIateApp);
#endif