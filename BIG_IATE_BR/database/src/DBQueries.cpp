// database/src/DBQueries.cpp
#include "../include/DBQueries.h"
#include "../include/DBMacros.h"
#include <sstream>
#include <iomanip>

namespace bigiate::db {

    DBQueries::DBQueries(std::shared_ptr<DBConnectionPool> pool)
        : m_executor(std::make_shared<ThreadSafeExecutor>(std::move(pool))) {
    }

    // ============================================================
    // ВСПОМОГАТЕЛЬНЫЕ
    // ============================================================

    std::string DBQueries::escapeString(const std::string& str) {
        // Простая экранизация (в реальном коде используйте PQescapeStringConn)
        std::string result;
        for (char c : str) {
            if (c == '\'') result += "''";
            else result += c;
        }
        return result;
    }

    std::string DBQueries::vectorToString(const std::vector<float>& vec) {
        if (vec.empty()) return "[]";
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) oss << ",";
            oss << std::fixed << std::setprecision(8) << vec[i];
        }
        oss << "]";
        return oss.str();
    }

    std::vector<float> DBQueries::stringToVector(const std::string& str) {
        std::vector<float> result;
        if (str.empty() || str[0] != '[') return result;
        std::string trimmed = str.substr(1, str.length() - 2);
        std::stringstream ss(trimmed);
        std::string token;
        while (std::getline(ss, token, ',')) {
            if (!token.empty()) {
                try { result.push_back(std::stof(token)); }
                catch (...) {}
            }
        }
        return result;
    }

    // ============================================================
    // ПАРСЕРЫ
    // ============================================================

    Person DBQueries::parsePerson(PGresult* res, int row) {
        Person p;
        p.id = DB_GET_INT(res, row, 0, 0);
        p.fullName = DB_GET_STRING(res, row, 1, "");
        p.personType = DB_GET_STRING(res, row, 2, "");
        p.birthDate = DB_GET_STRING(res, row, 3, "");
        p.gender = DB_GET_STRING(res, row, 4, "M")[0];
        p.phone = DB_GET_STRING(res, row, 5, "");
        p.email = DB_GET_STRING(res, row, 6, "");
        p.address = DB_GET_STRING(res, row, 7, "");
        p.isActive = DB_GET_BOOL(res, row, 8, true);
        p.isBlocked = DB_GET_BOOL(res, row, 9, false);
        p.blockReason = DB_GET_STRING(res, row, 10, "");
        p.blockUntil = DB_GET_STRING(res, row, 11, "");
        p.verificationType = DB_GET_STRING(res, row, 12, "face_only");
        p.verificationCode = DB_GET_STRING(res, row, 13, "");
        p.notes = DB_GET_STRING(res, row, 14, "");
        p.createdAt = DB_GET_STRING(res, row, 15, "");
        p.updatedAt = DB_GET_STRING(res, row, 16, "");
        p.externalId = DB_GET_STRING(res, row, 17, "");
        return p;
    }

    MatchResult DBQueries::parseMatchResult(PGresult* res, int row) {
        MatchResult mr;
        mr.personId = DB_GET_INT(res, row, 0, 0);
        mr.encodingId = DB_GET_INT(res, row, 1, 0);
        mr.similarity = DB_GET_FLOAT(res, row, 2, 0.0f);
        mr.personType = DB_GET_STRING(res, row, 3, "");
        mr.fullName = DB_GET_STRING(res, row, 4, "");
        mr.verificationType = DB_GET_STRING(res, row, 5, "face_only");
        mr.verificationCode = DB_GET_STRING(res, row, 6, "");
        mr.notes = DB_GET_STRING(res, row, 7, "");
        return mr;
    }

    // ============================================================
    // ТЕСТ ПОДКЛЮЧЕНИЯ
    // ============================================================

    std::expected<void, std::string> DBQueries::testConnection() {
        auto res = m_executor->query("SELECT 1");
        if (!res.has_value()) return std::unexpected(res.error());
        PQclear(res.value());
        return {};
    }

    // ============================================================
    // ЛЮДИ
    // ============================================================

    std::expected<std::vector<Person>, std::string> DBQueries::getAllPersons(bool onlyActive) {
        std::string query = "SELECT id, full_name, person_type, birth_date, gender, "
            "phone, email, address, is_active, is_blocked, "
            "block_reason, blocked_until, verification_type, "
            "verification_code, notes, created_at, updated_at, external_id "
            "FROM persons WHERE 1=1";
        if (onlyActive) query += " AND is_active = true AND is_blocked = false";
        query += " ORDER BY full_name";

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        int rows = PQntuples(result.get());
        std::vector<Person> persons;
        persons.reserve(rows);
        for (int i = 0; i < rows; ++i) persons.push_back(parsePerson(result.get(), i));
        return persons;
    }

    std::expected<Person, std::string> DBQueries::getPersonById(int id) {
        std::string query = "SELECT id, full_name, person_type, birth_date, gender, "
            "phone, email, address, is_active, is_blocked, "
            "block_reason, blocked_until, verification_type, "
            "verification_code, notes, created_at, updated_at, external_id "
            "FROM persons WHERE id = " + std::to_string(id);

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        if (PQntuples(result.get()) == 0) {
            return std::unexpected("Person not found with id: " + std::to_string(id));
        }
        return parsePerson(result.get(), 0);
    }

    std::expected<int, std::string> DBQueries::addPerson(const Person& person) {
        std::string query = "INSERT INTO persons (full_name, person_type, birth_date, "
            "gender, phone, email, address, is_active, is_blocked, "
            "block_reason, blocked_until, verification_type, "
            "verification_code, notes, external_id) VALUES ('" +
            escapeString(person.fullName) + "', '" + person.personType + "', " +
            (person.birthDate.empty() ? "NULL" : "'" + person.birthDate + "'") + ", '" +
            person.gender + "', '" + escapeString(person.phone) + "', '" +
            escapeString(person.email) + "', '" + escapeString(person.address) + "', " +
            (person.isActive ? "true" : "false") + ", " +
            (person.isBlocked ? "true" : "false") + ", " +
            (person.blockReason.empty() ? "NULL" : "'" + escapeString(person.blockReason) + "'") + ", " +
            (person.blockUntil.empty() ? "NULL" : "'" + person.blockUntil + "'") + ", '" +
            person.verificationType + "', " +
            (person.verificationCode.empty() ? "NULL" : "'" + person.verificationCode + "'") + ", " +
            (person.notes.empty() ? "NULL" : "'" + escapeString(person.notes) + "'") + ", " +
            (person.externalId.empty() ? "NULL" : "'" + person.externalId + "'") +
            ") RETURNING id";

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        if (PQntuples(result.get()) == 0) {
            return std::unexpected("Failed to insert person");
        }
        return DB_GET_INT(result.get(), 0, 0, 0);
    }

    // ============================================================
    // ПОИСК ПО ЭМБЕДДИНГУ (PGVECTOR)
    // ============================================================

    std::expected<std::vector<MatchResult>, std::string> DBQueries::findPersonByEmbedding(
        const std::vector<float>& embedding, float threshold, int limit) {

        std::string query = "SELECT * FROM find_person_by_embedding('" +
            vectorToString(embedding) + "'::vector, " +
            std::to_string(threshold) + ") LIMIT " + std::to_string(limit);

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        int rows = PQntuples(result.get());
        std::vector<MatchResult> matches;
        matches.reserve(rows);
        for (int i = 0; i < rows; ++i) {
            matches.push_back(parseMatchResult(result.get(), i));
        }
        return matches;
    }

    // ============================================================
    // ЖУРНАЛ
    // ============================================================

    std::expected<long, std::string> DBQueries::addAccessLog(const AccessLog& log) {
        std::string query = "INSERT INTO access_logs (person_id, camera_id, gate_id, "
            "direction, access_granted, access_reason, similarity_score, "
            "face_image_path, is_forced, forced_by_person_id, forced_reason, notes) VALUES (" +
            (log.personId == 0 ? "NULL" : std::to_string(log.personId)) + ", " +
            std::to_string(log.cameraId) + ", '" + log.gateId + "', '" + log.direction + "', " +
            (log.accessGranted ? "true" : "false") + ", '" + escapeString(log.accessReason) + "', " +
            std::to_string(log.similarityScore) + ", '" + log.faceImagePath + "', " +
            (log.isForced ? "true" : "false") + ", " +
            (log.forcedByPersonId == 0 ? "NULL" : std::to_string(log.forcedByPersonId)) + ", " +
            (log.forcedReason.empty() ? "NULL" : "'" + escapeString(log.forcedReason) + "'") + ", " +
            (log.notes.empty() ? "NULL" : "'" + escapeString(log.notes) + "'") +
            ") RETURNING id";

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        if (PQntuples(result.get()) == 0) {
            return std::unexpected("Failed to insert access log");
        }
        return DB_GET_INT(result.get(), 0, 0, 0);
    }

    // ============================================================
    // СТАТИСТИКА
    // ============================================================

    std::expected<DBQueries::Stats, std::string> DBQueries::getStats() {
        std::string query = "SELECT "
            "(SELECT COUNT(*) FROM persons) as total_persons, "
            "(SELECT COUNT(*) FROM persons WHERE is_active = true AND is_blocked = false) as active_persons, "
            "(SELECT COUNT(*) FROM persons WHERE is_blocked = true) as blocked_persons, "
            "(SELECT COUNT(*) FROM access_logs WHERE access_time::date = CURRENT_DATE) as total_today, "
            "(SELECT COUNT(*) FROM access_logs WHERE access_time::date = CURRENT_DATE AND access_granted = true) as granted_today, "
            "(SELECT COUNT(*) FROM access_logs WHERE access_time::date = CURRENT_DATE AND access_granted = false) as denied_today";

        auto res = m_executor->query(query);
        if (!res.has_value()) return std::unexpected(res.error());

        PGresultPtr result(res.value());
        Stats stats;
        stats.totalPersons = DB_GET_INT(result.get(), 0, 0, 0);
        stats.activePersons = DB_GET_INT(result.get(), 0, 1, 0);
        stats.blockedPersons = DB_GET_INT(result.get(), 0, 2, 0);
        stats.totalAccessToday = DB_GET_INT(result.get(), 0, 3, 0);
        stats.grantedToday = DB_GET_INT(result.get(), 0, 4, 0);
        stats.deniedToday = DB_GET_INT(result.get(), 0, 5, 0);
        return stats;
    }

} // namespace bigiate::db