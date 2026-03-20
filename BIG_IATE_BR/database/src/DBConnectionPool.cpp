// database/src/DBConnectionPool.cpp
#include "../include/DBConnectionPool.h"
#include <libpq-fe.h>
#include <sstream>
#include <thread>

namespace bigiate::db {

    DBConnectionPool::DBConnectionPool(const config::DatabaseConfig& cfg,
        const std::string& password,
        int minConnections,
        int maxConnections)
        : m_config(cfg)
        , m_password(password)
        , m_minConnections(minConnections)
        , m_maxConnections(maxConnections) {

        // Создаём минимальное количество соединений
        for (int i = 0; i < minConnections; ++i) {
            auto connResult = createConnection();
            if (connResult.has_value()) {
                auto pooled = std::make_unique<PooledConnection>();
                pooled->conn = PGconnPtr(connResult.value());
                pooled->inUse = false;
                pooled->id = m_nextConnectionId++;
                m_connections.push_back(std::move(pooled));
                m_available.push(m_connections.back().get());
            }
        }
    }

    DBConnectionPool::~DBConnectionPool() {
        close();
    }

    std::expected<PGconn*, std::string> DBConnectionPool::createConnection() {
        // Формируем строку подключения
        std::ostringstream oss;
        oss << "host=" << m_config.host
            << " port=" << m_config.port
            << " dbname=" << m_config.name
            << " user=" << m_config.username;

        if (!m_password.empty()) {
            oss << " password=" << m_password;
        }

        std::string connStr = oss.str();
        PGconn* conn = PQconnectdb(connStr.c_str());

        if (PQstatus(conn) != CONNECTION_OK) {
            std::string error = PQerrorMessage(conn);
            PQfinish(conn);
            return std::unexpected("Failed to connect: " + error);
        }

        // Устанавливаем таймаут
        PQsetnonblocking(conn, 0);

        return conn;
    }

    bool DBConnectionPool::isConnectionAlive(PGconn* conn) {
        if (!conn) return false;
        if (PQstatus(conn) != CONNECTION_OK) return false;

        // Пинг-запрос
        PGresult* res = PQexec(conn, "SELECT 1");
        bool alive = (res && PQresultStatus(res) == PGRES_TUPLES_OK);
        PQclear(res);
        return alive;
    }

    std::expected<PGconn*, std::string> DBConnectionPool::acquire() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingRequests++;

        // Ждём доступное соединение
        while (m_available.empty()) {
            // Если можем создать новое соединение
            if (m_connections.size() < (size_t)m_maxConnections) {
                auto connResult = createConnection();
                if (connResult.has_value()) {
                    auto pooled = std::make_unique<PooledConnection>();
                    pooled->conn = PGconnPtr(connResult.value());
                    pooled->inUse = false;
                    pooled->id = m_nextConnectionId++;
                    m_connections.push_back(std::move(pooled));
                    m_available.push(m_connections.back().get());
                    break;
                }
            }

            // Ждём освобождения
            m_cv.wait(lock);

            // Очищаем мёртвые соединения
            cleanupDeadConnections();
        }

        m_pendingRequests--;

        if (m_available.empty()) {
            return std::unexpected("No available database connections");
        }

        PooledConnection* pooled = m_available.front();
        m_available.pop();

        // Проверяем живо ли соединение
        if (!isConnectionAlive(pooled->conn.get())) {
            // Пересоздаём
            auto connResult = createConnection();
            if (!connResult.has_value()) {
                return std::unexpected(connResult.error());
            }
            pooled->conn = PGconnPtr(connResult.value());
        }

        pooled->inUse = true;
        pooled->lastUsed = std::chrono::steady_clock::now();

        return pooled->conn.get();
    }

    void DBConnectionPool::release(PGconn* conn) {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& pooled : m_connections) {
            if (pooled->conn.get() == conn) {
                pooled->inUse = false;
                m_available.push(pooled.get());
                break;
            }
        }

        m_cv.notify_one();
    }

    DBConnectionPool::Stats DBConnectionPool::getStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        Stats stats;
        stats.totalConnections = (int)m_connections.size();
        stats.activeConnections = 0;

        for (const auto& pooled : m_connections) {
            if (pooled->inUse) stats.activeConnections++;
        }

        stats.idleConnections = stats.totalConnections - stats.activeConnections;
        stats.pendingRequests = m_pendingRequests.load();
        return stats;
    }

    void DBConnectionPool::cleanupDeadConnections() {
        // Удаляем мёртвые соединения, которые не используются
        auto it = m_connections.begin();
        while (it != m_connections.end()) {
            if (!(*it)->inUse && !isConnectionAlive((*it)->conn.get())) {
                it = m_connections.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void DBConnectionPool::close() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.clear();
        while (!m_available.empty()) m_available.pop();
    }

} // namespace bigiate::db