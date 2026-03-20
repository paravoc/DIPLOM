#pragma once

// Подключение к БД
#define DB_CHECK_CONNECTION(conn) \
    do { \
        if (!conn || PQstatus(conn) != CONNECTION_OK) { \
            return std::unexpected("Database connection failed: " + \
                                   std::string(PQerrorMessage(conn))); \
        } \
    } while(0)

// Выполнение запроса
#define DB_EXECUTE(conn, query, msg) \
    do { \
        auto res = PQexec(conn, query); \
        if (PQresultStatus(res) != PGRES_TUPLES_OK && \
            PQresultStatus(res) != PGRES_COMMAND_OK) { \
            PQclear(res); \
            return std::unexpected(msg + ": " + std::string(PQerrorMessage(conn))); \
        } \
        PQclear(res); \
    } while(0)

// Получение одной строки
#define DB_GET_ROW(res, row, col, type) \
    [&]() -> std::expected<type, std::string> { \
        if (PQgetisnull(res, row, col)) { \
            return std::unexpected("NULL value in column " + std::to_string(col)); \
        } \
        return PQgetvalue(res, row, col); \
    }()

// Параметризованный запрос
#define DB_PARAM_INT(i, val) \
    PQexecParams(conn, query, nParams, NULL, params, NULL, NULL, 0)