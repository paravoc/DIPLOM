#ifndef USE_CONSOLE
#include <wx/wx.h>
#include "ui/include/MainFrame.h"
#include "configs/include/loader.h"
#include "database/include/DBConnectionPool.h"
#include "database/include/DBQueries.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

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
        // ============================================================
        // ВКЛЮЧЕНИЕ КОНСОЛИ ДЛЯ ОТЛАДКИ
        // ============================================================
#ifdef _WIN32
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
#endif

        std::cout << "========================================" << std::endl;
        std::cout << "=== ПРОГРАММА ЗАПУЩЕНА ===" << std::endl;
        std::cout << "========================================" << std::endl;

        // Настройка локали (для русского языка)
        wxLocale locale;
        locale.Init(wxLANGUAGE_RUSSIAN);
        std::cout << "✅ Локаль установлена" << std::endl;

        // Загружаем конфиг
        std::string config_file = "C:\\Users\\smidr\\source\\repos\\BIG_IATE_BR\\x64\\Debug\\test_config.yaml";

        std::cout << "📁 Загрузка конфига: " << config_file << std::endl;

        auto result = config::LoadConfig(config_file);

        if (!result.has_value()) {
            std::cerr << "❌ Ошибка загрузки конфига: " << result.error() << std::endl;
            wxMessageBox(
                wxString::Format("Ошибка загрузки конфига:\n%s", result.error()),
                "Ошибка",
                wxOK | wxICON_ERROR
            );
            return false;
        }
        else {
            m_configResult = *result;
            std::cout << "✅ Конфиг загружен: версия "
                << m_configResult.config.version.toString()
                << ", камер: " << m_configResult.config.cameras.size() << std::endl;
        }

        // ============================================================
        // ИНИЦИАЛИЗАЦИЯ БАЗЫ ДАННЫХ
        // ============================================================
        std::cout << "\n📊 Инициализация базы данных..." << std::endl;

        std::string dbPassword = "1234"; // пароль из конфига

        std::cout << "  Хост: " << m_configResult.config.database.host << std::endl;
        std::cout << "  Порт: " << m_configResult.config.database.port << std::endl;
        std::cout << "  БД: " << m_configResult.config.database.name << std::endl;
        std::cout << "  Пользователь: " << m_configResult.config.database.username << std::endl;
        std::cout << "  Пароль: " << dbPassword << std::endl;

        // Создаём пул соединений
        try {
            m_dbPool = std::make_shared<db::DBConnectionPool>(
                m_configResult.config.database,
                dbPassword,
                2,   // min connections
                4    // max connections
            );
            std::cout << "✅ Пул соединений создан" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "❌ Ошибка создания пула: " << e.what() << std::endl;
            return false;
        }

        if (!m_dbPool) {
            std::cerr << "❌ Пул соединений = nullptr!" << std::endl;
            return false;
        }

        // Создаём объект для запросов
        try {
            m_dbQueries = std::make_unique<db::DBQueries>(m_dbPool);
            std::cout << "✅ DBQueries создан" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "❌ Ошибка создания DBQueries: " << e.what() << std::endl;
            return false;
        }

        if (!m_dbQueries) {
            std::cerr << "❌ DBQueries = nullptr!" << std::endl;
            return false;
        }

        // Тестируем подключение
        std::cout << "🔍 Проверка testConnection..." << std::endl;
        auto test = m_dbQueries->testConnection();
        if (!test.has_value()) {
            std::cerr << "❌ Ошибка подключения к БД: " << test.error() << std::endl;
            // Можно продолжить без БД или выйти
        }
        else {
            std::cout << "✅ Подключение к базе данных установлено!" << std::endl;

            // Получаем статистику
            std::cout << "🔍 Получение статистики..." << std::endl;
            auto stats = m_dbQueries->getStats();
            if (stats.has_value()) {
                std::cout << "📊 Статистика БД:" << std::endl;
                std::cout << "   Всего людей: " << stats->totalPersons << std::endl;
                std::cout << "   Активных: " << stats->activePersons << std::endl;
                std::cout << "   Заблокированных: " << stats->blockedPersons << std::endl;
                std::cout << "   Проходов сегодня: " << stats->totalAccessToday << std::endl;
                std::cout << "   Разрешено: " << stats->grantedToday << std::endl;
                std::cout << "   Запрещено: " << stats->deniedToday << std::endl;
            }
            else {
                std::cerr << "⚠️ Не удалось получить статистику: " << stats.error() << std::endl;
            }
        }

        // ============================================================
        // ЗАПУСК ПОТОКОВ ДЛЯ КАМЕР
        // ============================================================
        std::cout << "\n📷 Запуск потоков камер..." << std::endl;

        for (const auto& cam : m_configResult.config.cameras) {
            if (!cam.enabled) {
                std::cout << "   Камера " << cam.id << " (" << cam.name << ") - отключена" << std::endl;
                continue;
            }

            m_cameraThreads.emplace_back([this, cam]() {
                this->cameraWorker(cam);
                });

            std::cout << "   ✅ Запущен поток для камеры " << cam.id << ": " << cam.name << std::endl;
        }

        // ============================================================
        // СОЗДАНИЕ GUI
        // ============================================================
        std::cout << "\n🖥️ Создание главного окна..." << std::endl;

        try {
            MainFrame* frame = new MainFrame();
            if (!frame) {
                std::cerr << "❌ Ошибка: frame = nullptr" << std::endl;
                return false;
            }

            std::cout << "✅ Объект MainFrame создан" << std::endl;

            frame->Show(true);
            frame->Raise();
            frame->SetFocus();
            SetTopWindow(frame);

            std::cout << "✅ Окно показано" << std::endl;

            // Принудительное обновление
            frame->Update();
            frame->Refresh();

        }
        catch (const std::exception& e) {
            std::cerr << "❌ Ошибка создания окна: " << e.what() << std::endl;
            return false;
        }

        std::cout << "\n========================================" << std::endl;
        std::cout << "🚀 ПРОГРАММА ГОТОВА К РАБОТЕ" << std::endl;
        std::cout << "========================================\n" << std::endl;

        return true;
    }

    virtual int OnExit() override {
        std::cout << "\n🛑 Остановка программы..." << std::endl;

        // Останавливаем потоки
        m_running = false;

        // Ждём завершения всех потоков
        for (auto& thread : m_cameraThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        std::cout << "✅ Все потоки остановлены" << std::endl;

        // Закрываем соединения с БД
        if (m_dbPool) {
            m_dbPool->close();
            std::cout << "✅ Соединения с БД закрыты" << std::endl;
        }

        std::cout << "👋 Программа завершена" << std::endl;

        return wxApp::OnExit();
    }

private:
    // Функция для обработки камеры (заглушка)
    void cameraWorker(const config::CameraConfig& cam) {
        std::cout << "🎥 Поток камеры " << cam.id << " запущен" << std::endl;

        int frameCount = 0;
        while (m_running) {
            // Симуляция работы
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            frameCount++;
            if (frameCount % 10 == 0) {
                std::cout << "📹 Камера " << cam.id << ": работает (" << frameCount << " кадров)" << std::endl;
            }

            // TODO: Здесь будет реальная работа с камерой и распознаванием
        }

        std::cout << "🛑 Поток камеры " << cam.id << " остановлен (обработано кадров: " << frameCount << ")" << std::endl;
    }
};

wxIMPLEMENT_APP(BigIateApp);
#endif