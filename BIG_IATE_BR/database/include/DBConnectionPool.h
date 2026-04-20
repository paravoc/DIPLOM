#pragma once
// database/include/DBConnectionPool.h
#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <expected>
#include "DBMacros.h"
#include "../../configs/include/types.h"



namespace bigiate::db {

    // Информация о соединении
    struct PooledConnection {
        PGconnPtr conn;
        bool inUse = false;
        std::chrono::steady_clock::time_point lastUsed;
        int id = 0;
    };

    // Пул соединений к БД (потокобезопасный)
    class DBConnectionPool {
    public:
        explicit DBConnectionPool(const config::DatabaseConfig& cfg,
            const std::string& password,
            int minConnections = 2,
            int maxConnections = 10);
        ~DBConnectionPool();

        // Запрещаем копирование
        DBConnectionPool(const DBConnectionPool&) = delete;
        DBConnectionPool& operator=(const DBConnectionPool&) = delete;

        // Получить соединение из пула
        [[nodiscard]] std::expected<PGconn*, std::string> acquire();

        // Вернуть соединение в пул
        void release(PGconn* conn);

        // Получить статистику пула
        struct Stats {
            int totalConnections;
            int activeConnections;
            int idleConnections;
            int pendingRequests;
        };
        [[nodiscard]] Stats getStats() const;

        // Закрыть все соединения
        void close();

    private:
        // Создать новое соединение
        [[nodiscard]] std::expected<PGconn*, std::string> createConnection();

        // Проверить живо ли соединение
        bool isConnectionAlive(PGconn* conn);

        // Очистить мёртвые соединения
        void cleanupDeadConnections();

        config::DatabaseConfig m_config;
        std::string m_password;
        int m_minConnections;
        int m_maxConnections;

        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        std::vector<std::unique_ptr<PooledConnection>> m_connections;
        std::queue<PooledConnection*> m_available;
        std::atomic<int> m_pendingRequests{ 0 };
        int m_nextConnectionId = 0;
    };

} // namespace bigiate::db