//==============================================================================
// BIG IATE - Core Application Manager
// AppCore.h
//==============================================================================
// Описание: Главный класс управления приложением. Реализует паттерн Singleton.
//           Управляет загрузкой конфигурации, инициализацией БД, распознаванием,
//           камерами и GUI. Координирует работу всех модулей.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

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

    //==============================================================================
    // ГЛАВНЫЙ КЛАСС УПРАВЛЕНИЯ ПРИЛОЖЕНИЕМ
    //==============================================================================
    class AppCore {
    public:
        // Получение единственного экземпляра (Singleton)
        static AppCore& instance() {
            static AppCore core;
            return core;
        }

        // Запуск системы (главная точка входа)
        [[nodiscard]] bool run(const std::string& configPath);

        // Остановка всех потоков и освобождение ресурсов
        void stop();

        // Получение указателя на главное окно
        MainFrame* getMainFrame() { return m_frame; }

        // Установка секретов (пароли, загруженные из зашифрованного файла)
        void setSecrets(const config::Secrets& secrets) { m_secrets = secrets; }

    private:
        //==========================================================================
        // ИНИЦИАЛИЗАЦИЯ МОДУЛЕЙ
        //==========================================================================

        // Загрузка и парсинг конфигурационного файла
        bool loadConfig(const std::string& path);

        // Инициализация подключения к базе данных
        bool initDatabase();

        // Инициализация модуля распознавания лиц
        bool initRecognition();

        // Создание и отображение графического интерфейса
        bool createGUI();

        // Запуск потоков для всех камер
        bool startCameras();

        //==========================================================================
        // РАБОТА С КАМЕРАМИ
        //==========================================================================

        // Поток обработки одной камеры
        void cameraWorker(int cameraId, const config::CameraConfig& cfg);

        //==========================================================================
        // ДАННЫЕ
        //==========================================================================

        config::LoadResult m_configResult;      // Загруженная конфигурация
        config::Secrets m_secrets;              // Загруженные секреты (пароли)

        // База данных
        std::shared_ptr<db::DBConnectionPool> m_dbPool;   // Пул соединений
        std::shared_ptr<db::DBQueries> m_dbQueries;       // Выполнение запросов

        // Распознавание лиц
        std::unique_ptr<recognition::FaceRecognizer> m_faceRecognizer;

        // Потоки камер
        std::vector<std::thread> m_cameraThreads;
        std::atomic<bool> m_running{ false };             // Флаг работы потоков

        // GUI
        MainFrame* m_frame{ nullptr };

        //==========================================================================
        // КОНСТРУКТОРЫ И ДЕСТРУКТОР
        //==========================================================================

        AppCore() = default;
        ~AppCore();

        // Запрет копирования и перемещения
        AppCore(const AppCore&) = delete;
        AppCore& operator=(const AppCore&) = delete;
    };

} // namespace bigiate::core