#pragma once

#include <string>
#include <expected>
#include <memory>
#include <libpq-fe.h>

#include "../../configs/include/types.h"

namespace bigiate::db {

    // Результат подключения
    struct DBConnectionResult {
        PGconn* conn = nullptr;
        bool success = false;
        std::string error;
    };

    // Класс управления подключением к БД
    class DBConnection {
    public:
        DBConnection();
        ~DBConnection();

        // Запрещаем копирование
        DBConnection(const DBConnection&) = delete;
        DBConnection& operator=(const DBConnection&) = delete;

        // Разрешаем перемещение
        DBConnection(DBConnection&& other) noexcept;
        DBConnection& operator=(DBConnection&& other) noexcept;

        // Инициализация из конфига
        [[nodiscard]] std::expected<void, std::string> init(const config::DatabaseConfig& cfg);

        // Закрытие соединения
        void close();

        // Проверка соединения
        [[nodiscard]] bool isConnected() const;

        // Получение сырого указателя (для низкоуровневых операций)
        [[nodiscard]] PGconn* get() const { return m_conn; }

        // Выполнение запроса без возврата данных
        [[nodiscard]] std::expected<void, std::string> execute(const std::string& query);

        // Выполнение запроса с возвратом результата
        [[nodiscard]] std::expected<PGresult*, std::string> query(const std::string& query);

        // Формирование строки подключения
        [[nodiscard]] static std::string makeConnectionString(const config::DatabaseConfig& cfg);

    private:
        PGconn* m_conn = nullptr;
        config::DatabaseConfig m_config;
    };

} // namespace bigiate::db