// database/include/DBThreadSafe.h
#pragma once

#include <memory>
#include <mutex>
#include "DBConnectionPool.h"

namespace bigiate::db {

    // RAII-обёртка для соединения (автоматически возвращает в пул)
    class ScopedConnection {
    public:
        ScopedConnection(DBConnectionPool& pool);
        ~ScopedConnection();

        // Запрещаем копирование
        ScopedConnection(const ScopedConnection&) = delete;
        ScopedConnection& operator=(const ScopedConnection&) = delete;

        // Разрешаем перемещение
        ScopedConnection(ScopedConnection&& other) noexcept;
        ScopedConnection& operator=(ScopedConnection&& other) noexcept;

        PGconn* get() const { return m_conn; }
        operator PGconn* () const { return m_conn; }
        bool isValid() const { return m_conn != nullptr; }

    private:
        DBConnectionPool* m_pool = nullptr;
        PGconn* m_conn = nullptr;
    };

    // ============================================================
    // ПОТОКОБЕЗОПАСНЫЙ ВЫПОЛНИТЕЛЬ ЗАПРОСОВ
    // ============================================================

    class ThreadSafeExecutor {
    public:
        explicit ThreadSafeExecutor(std::shared_ptr<DBConnectionPool> pool);
        ~ThreadSafeExecutor() = default;

        // Выполнить запрос без возврата
        [[nodiscard]] std::expected<void, std::string> execute(const std::string& query);

        // Выполнить запрос с возвратом результата
        [[nodiscard]] std::expected<PGresult*, std::string> query(const std::string& query);

        // Выполнить параметризованный запрос
        [[nodiscard]] std::expected<PGresult*, std::string> queryParams(
            const std::string& query,
            int nParams,
            const char* const* params);

    private:
        std::shared_ptr<DBConnectionPool> m_pool;
    };

} // namespace bigiate::db