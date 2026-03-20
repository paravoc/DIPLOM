// database/include/DBMacros.h
#pragma once

#include <libpq-fe.h>
#include <string>
#include <memory>
#include <expected>
#include <mutex>

namespace bigiate::db {

    // ============================================================
    // RAII ОБЁРТКА ДЛЯ PGresult
    // ============================================================
    struct PGresultDeleter {
        void operator()(PGresult* res) const {
            if (res) PQclear(res);
        }
    };
    using PGresultPtr = std::unique_ptr<PGresult, PGresultDeleter>;

    // ============================================================
    // RAII ОБЁРТКА ДЛЯ PGconn (для пула)
    // ============================================================
    struct PGconnDeleter {
        void operator()(PGconn* conn) const {
            if (conn) PQfinish(conn);
        }
    };
    using PGconnPtr = std::unique_ptr<PGconn, PGconnDeleter>;

    // ============================================================
    // МАКРОСЫ ДЛЯ РАБОТЫ С БД (потокобезопасные)
    // ============================================================

    // Проверка соединения (используется внутри методов)
#define DB_CHECK_CONN(conn) \
    do { \
        if (!conn || PQstatus(conn) != CONNECTION_OK) { \
            return std::unexpected("Database not connected: " + \
                                   std::string(PQerrorMessage(conn))); \
        } \
    } while(0)

// Выполнение запроса без возврата (с блокировкой)
#define DB_EXEC_LOCKED(conn, query, errMsg, mutex) \
    do { \
        std::lock_guard<std::mutex> lock(mutex); \
        auto* res = PQexec(conn, query); \
        if (PQresultStatus(res) != PGRES_COMMAND_OK) { \
            std::string err = errMsg + ": " + std::string(PQerrorMessage(conn)); \
            PQclear(res); \
            return std::unexpected(err); \
        } \
        PQclear(res); \
    } while(0)

// Выполнение запроса с возвратом (с блокировкой)
#define DB_QUERY_LOCKED(conn, query, errMsg, mutex) \
    [&]() -> std::expected<PGresult*, std::string> { \
        std::lock_guard<std::mutex> lock(mutex); \
        auto* res = PQexec(conn, query); \
        if (PQresultStatus(res) != PGRES_TUPLES_OK) { \
            std::string err = errMsg + ": " + std::string(PQerrorMessage(conn)); \
            PQclear(res); \
            return std::unexpected(err); \
        } \
        return res; \
    }()

// Получение строки
#define DB_GET_STRING(res, row, col, def) \
    (PQgetisnull(res, row, col) ? def : std::string(PQgetvalue(res, row, col)))

// Получение целого числа
#define DB_GET_INT(res, row, col, def) \
    (PQgetisnull(res, row, col) ? def : std::atoi(PQgetvalue(res, row, col)))

// Получение числа с плавающей точкой
#define DB_GET_FLOAT(res, row, col, def) \
    (PQgetisnull(res, row, col) ? def : std::atof(PQgetvalue(res, row, col)))

// Получение булевого значения
#define DB_GET_BOOL(res, row, col, def) \
    (PQgetisnull(res, row, col) ? def : (std::string(PQgetvalue(res, row, col)) == "t"))

// Экранирование строки (защита от SQL-инъекций)
#define DB_ESCAPE(conn, str) \
    [&]() -> std::string { \
        size_t len = str.length(); \
        char* escaped = new char[len * 2 + 1]; \
        PQescapeStringConn(conn, escaped, str.c_str(), len, nullptr); \
        std::string result(escaped); \
        delete[] escaped; \
        return result; \
    }()

} // namespace bigiate::db