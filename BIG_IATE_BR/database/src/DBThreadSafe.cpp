// database/src/DBThreadSafe.cpp
#include "../include/DBThreadSafe.h"

namespace bigiate::db {

    // ============================================================
    // ScopedConnection
    // ============================================================

    ScopedConnection::ScopedConnection(DBConnectionPool& pool) : m_pool(&pool) {
        auto connResult = pool.acquire();
        if (connResult.has_value()) {
            m_conn = connResult.value();
        }
    }

    ScopedConnection::~ScopedConnection() {
        if (m_pool && m_conn) {
            m_pool->release(m_conn);
        }
    }

    ScopedConnection::ScopedConnection(ScopedConnection&& other) noexcept
        : m_pool(other.m_pool), m_conn(other.m_conn) {
        other.m_pool = nullptr;
        other.m_conn = nullptr;
    }

    ScopedConnection& ScopedConnection::operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            if (m_pool && m_conn) {
                m_pool->release(m_conn);
            }
            m_pool = other.m_pool;
            m_conn = other.m_conn;
            other.m_pool = nullptr;
            other.m_conn = nullptr;
        }
        return *this;
    }

    // ============================================================
    // ThreadSafeExecutor
    // ============================================================

    ThreadSafeExecutor::ThreadSafeExecutor(std::shared_ptr<DBConnectionPool> pool)
        : m_pool(std::move(pool)) {
    }

    std::expected<void, std::string> ThreadSafeExecutor::execute(const std::string& query) {
        if (!m_pool) {
            return std::unexpected("Database pool not initialized");
        }

        ScopedConnection conn(*m_pool);
        if (!conn.isValid()) {
            return std::unexpected("Failed to acquire database connection");
        }

        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            return std::unexpected("Query failed: " + error);
        }
        PQclear(res);

        return {};
    }

    std::expected<PGresult*, std::string> ThreadSafeExecutor::query(const std::string& query) {
        if (!m_pool) {
            return std::unexpected("Database pool not initialized");
        }

        ScopedConnection conn(*m_pool);
        if (!conn.isValid()) {
            return std::unexpected("Failed to acquire database connection");
        }

        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            return std::unexpected("Query failed: " + error);
        }

        return res;
    }

    std::expected<PGresult*, std::string> ThreadSafeExecutor::queryParams(
        const std::string& query,
        int nParams,
        const char* const* params) {

        if (!m_pool) {
            return std::unexpected("Database pool not initialized");
        }

        ScopedConnection conn(*m_pool);
        if (!conn.isValid()) {
            return std::unexpected("Failed to acquire database connection");
        }

        PGresult* res = PQexecParams(conn, query.c_str(), nParams, nullptr, params, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            return std::unexpected("Query failed: " + error);
        }

        return res;
    }

} // namespace bigiate::db