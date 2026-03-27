// database/src/DBQueries.cpp
#include "../include/DBQueries.h"
#include "../include/DBMacros.h"
#include <sstream>
#include <iomanip>
#include<iostream>


namespace bigiate::db {

#ifdef _WIN32
#include <windows.h>

    static std::string toUtf8(const std::string& str) {
        // Проверяем, есть ли русские буквы
        bool hasRussian = false;
        for (char c : str) {
            unsigned char uc = static_cast<unsigned char>(c);
            // Диапазон русских букв в Windows-1251: 192-255
            if (uc >= 192 && uc <= 255) {
                hasRussian = true;
                break;
            }
        }

        if (!hasRussian) return str;

        // Конвертируем из Windows-1251 в UTF-8
        int wideSize = MultiByteToWideChar(1251, 0, str.c_str(), -1, nullptr, 0);
        if (wideSize == 0) return str;

        std::wstring wide(wideSize, L'\0');
        MultiByteToWideChar(1251, 0, str.c_str(), -1, &wide[0], wideSize);

        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Size == 0) return str;

        std::string utf8(utf8Size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], utf8Size, nullptr, nullptr);

        return utf8;
    }
#else
    static std::string toUtf8(const std::string& str) {
        return str;  // Linux/Mac — уже UTF-8
    }
#endif

    DBQueries::DBQueries(std::shared_ptr<DBConnectionPool> pool)
        : m_executor(std::make_shared<ThreadSafeExecutor>(std::move(pool))) {
    }

    std::expected<void, std::string> DBQueries::execute(const std::string& query) {
        return m_executor->execute(query);
    }

    // ============================================================
// ОБНОВЛЕНИЕ ЧЕЛОВЕКА
// ============================================================

    static std::vector<float> normalizeEmbedding(const std::vector<float>& embedding) {
        if (embedding.empty()) return embedding;

        // Вычисляем норму (длину вектора)
        float norm = 0.0f;
        for (float val : embedding) {
            norm += val * val;
        }
        norm = std::sqrt(norm);

        if (norm < 0.0001f) return embedding;  // Защита от деления на ноль

        // Нормализуем
        std::vector<float> result;
        result.reserve(embedding.size());
        for (float val : embedding) {
            result.push_back(val / norm);
        }

        return result;
    }




    std::expected<void, std::string> DBQueries::updatePerson(const Person& person) {
        std::string query = "UPDATE persons SET "
            "full_name = '" + escapeString(person.fullName) + "', "
            "person_type = '" + person.personType + "', "
            "birth_date = " + (person.birthDate.empty() ? "NULL" : "'" + person.birthDate + "'") + ", "
            "gender = '" + person.gender + "', "
            "phone = '" + escapeString(person.phone) + "', "
            "email = '" + escapeString(person.email) + "', "
            "address = '" + escapeString(person.address) + "', "
            "is_active = " + (person.isActive ? "true" : "false") + ", "
            "is_blocked = " + (person.isBlocked ? "true" : "false") + ", "
            "block_reason = " + (person.blockReason.empty() ? "NULL" : "'" + escapeString(person.blockReason) + "'") + ", "
            "blocked_until = " + (person.blockUntil.empty() ? "NULL" : "'" + person.blockUntil + "'") + ", "
            "verification_type = '" + person.verificationType + "', "
            "verification_code = " + (person.verificationCode.empty() ? "NULL" : "'" + person.verificationCode + "'") + ", "
            "notes = " + (person.notes.empty() ? "NULL" : "'" + escapeString(person.notes) + "'") + ", "
            "external_id = " + (person.externalId.empty() ? "NULL" : "'" + person.externalId + "'") + ", "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = " + std::to_string(person.id);

        return m_executor->execute(query);
    }

    // ============================================================
    // БЛОКИРОВКА ЧЕЛОВЕКА
    // ============================================================

    std::expected<void, std::string> DBQueries::blockPerson(int id, const std::string& reason, const std::string& until) {
        std::string query = "UPDATE persons SET "
            "is_blocked = true, "
            "block_reason = " + (reason.empty() ? "NULL" : "'" + escapeString(reason) + "'") + ", "
            "blocked_until = " + (until.empty() ? "NULL" : "'" + until + "'") + ", "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = " + std::to_string(id);

        return m_executor->execute(query);
    }

    // ============================================================
    // РАЗБЛОКИРОВКА ЧЕЛОВЕКА
    // ============================================================

    std::expected<void, std::string> DBQueries::unblockPerson(int id) {
        std::string query = "UPDATE persons SET "
            "is_blocked = false, "
            "block_reason = NULL, "
            "blocked_until = NULL, "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = " + std::to_string(id);

        return m_executor->execute(query);
    }

    // ============================================================
    // ПОЛУЧИТЬ ЭМБЕДДИНГИ ЧЕЛОВЕКА
    // ============================================================

    std::expected<std::vector<FaceEncoding>, std::string> DBQueries::getFaceEncodingsByPerson(int personId) {
        std::string query = "SELECT id, person_id, embedding, source_image_path, capture_date, "
            "quality_score, face_size, is_primary, is_current, expiration_date, notes "
            "FROM face_encodings WHERE person_id = " + std::to_string(personId) +
            " ORDER BY is_primary DESC, capture_date DESC";

        auto res = m_executor->query(query);
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

        PGresultPtr result(res.value());
        int rows = PQntuples(result.get());
        std::vector<FaceEncoding> encodings;
        encodings.reserve(rows);

        for (int i = 0; i < rows; ++i) {
            FaceEncoding enc;
            enc.id = DB_GET_INT(result.get(), i, 0, 0);
            enc.personId = DB_GET_INT(result.get(), i, 1, 0);
            std::string embStr = DB_GET_STRING(result.get(), i, 2, "[]");
            enc.embedding = stringToVector(embStr);
            enc.sourceImagePath = DB_GET_STRING(result.get(), i, 3, "");
            enc.captureDate = DB_GET_STRING(result.get(), i, 4, "");
            enc.qualityScore = DB_GET_FLOAT(result.get(), i, 5, 0.0f);
            enc.faceSize = DB_GET_INT(result.get(), i, 6, 0);
            enc.isPrimary = DB_GET_BOOL(result.get(), i, 7, false);
            enc.isCurrent = DB_GET_BOOL(result.get(), i, 8, true);
            enc.expirationDate = DB_GET_STRING(result.get(), i, 9, "");
            enc.notes = DB_GET_STRING(result.get(), i, 10, "");
            encodings.push_back(enc);
        }

        return encodings;
    }

    // ============================================================
    // СДЕЛАТЬ ЭМБЕДДИНГ ОСНОВНЫМ
    // ============================================================

    std::expected<void, std::string> DBQueries::setPrimaryEncoding(int personId, int encodingId) {
        // Начинаем транзакцию
        auto beginResult = m_executor->execute("BEGIN");
        if (!beginResult.has_value()) {
            return std::unexpected(beginResult.error());
        }

        // Снимаем флаг is_primary со всех эмбеддингов этого человека
        std::string resetQuery = "UPDATE face_encodings SET is_primary = false WHERE person_id = " + std::to_string(personId);
        auto resetResult = m_executor->execute(resetQuery);
        if (!resetResult.has_value()) {
            m_executor->execute("ROLLBACK");
            return std::unexpected(resetResult.error());
        }

        // Устанавливаем is_primary для выбранного эмбеддинга
        std::string setQuery = "UPDATE face_encodings SET is_primary = true WHERE id = " + std::to_string(encodingId);
        auto setResult = m_executor->execute(setQuery);
        if (!setResult.has_value()) {
            m_executor->execute("ROLLBACK");
            return std::unexpected(setResult.error());
        }

        // Фиксируем транзакцию
        auto commitResult = m_executor->execute("COMMIT");
        if (!commitResult.has_value()) {
            return std::unexpected(commitResult.error());
        }

        return {};
    }

    // ============================================================
    // ВСПОМОГАТЕЛЬНЫЕ
    // ============================================================

    std::string DBQueries::escapeString(const std::string& str) {
        if (str.empty()) return "";

        std::string result;
        result.reserve(str.size() * 2);

        for (char c : str) {
            switch (c) {
            case '\'': result += "''"; break;
            case '\\': result += "\\\\"; break;
            case '\"': result += "\\\""; break;
            case '\0': break;
            default: result += c; break;
            }
        }
        return result;
    }

    std::expected<int, std::string> DBQueries::addFaceEncoding(const FaceEncoding& encoding) {
        // Нормализуем эмбеддинг
        std::vector<float> normalizedEmbedding = normalizeEmbedding(encoding.embedding);

#ifdef _WIN32
        std::string sourceImagePath = toUtf8(encoding.sourceImagePath);
        std::string notes = toUtf8(encoding.notes);
#else
        std::string sourceImagePath = encoding.sourceImagePath;
        std::string notes = encoding.notes;
#endif

        // Форматируем числа с точкой, а не запятой
        std::string qualityScoreStr = std::to_string(encoding.qualityScore);
        qualityScoreStr[1] = '.';
        std::cout << qualityScoreStr << std::endl;
       
        std::string query = "INSERT INTO face_encodings (person_id, embedding, source_image_path, "
            "capture_date, quality_score, face_size, is_primary, is_current, "
            "expiration_date, notes) VALUES (" +
            std::to_string(encoding.personId) + ", '" +
            vectorToString(normalizedEmbedding) + "'::vector, '" +
            escapeString(sourceImagePath) + "', '" +
            encoding.captureDate + "', " +
            qualityScoreStr + ", " +      // <-- ИСПРАВЛЕНО
            std::to_string(encoding.faceSize) + ", " +
            (encoding.isPrimary ? "true" : "false") + ", " +
            (encoding.isCurrent ? "true" : "false") + ", " +
            (encoding.expirationDate.empty() ? "NULL" : "'" + encoding.expirationDate + "'") + ", " +
            (notes.empty() ? "NULL" : "'" + escapeString(notes) + "'") +
            ") RETURNING id";

        auto res = m_executor->query(query);
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

        PGresultPtr result(res.value());
        if (PQntuples(result.get()) == 0) {
            return std::unexpected("Failed to insert face encoding");
        }
        return DB_GET_INT(result.get(), 0, 0, 0);
    }


    AccessLog DBQueries::parseAccessLog(PGresult* res, int row) {
        AccessLog log;
        log.id = DB_GET_INT(res, row, 0, 0);
        log.personId = DB_GET_INT(res, row, 1, 0);
        log.accessTime = DB_GET_STRING(res, row, 2, "");
        log.cameraId = DB_GET_INT(res, row, 3, 0);
        log.gateId = DB_GET_STRING(res, row, 4, "");
        log.direction = DB_GET_STRING(res, row, 5, "");
        log.accessGranted = DB_GET_BOOL(res, row, 6, false);
        log.accessReason = DB_GET_STRING(res, row, 7, "");
        log.similarityScore = DB_GET_FLOAT(res, row, 8, 0.0f);
        log.encodingId = DB_GET_INT(res, row, 9, 0);
        log.faceImagePath = DB_GET_STRING(res, row, 10, "");
        log.fullFramePath = DB_GET_STRING(res, row, 11, "");
        log.isForced = DB_GET_BOOL(res, row, 12, false);
        log.forcedByPersonId = DB_GET_INT(res, row, 13, 0);
        log.forcedReason = DB_GET_STRING(res, row, 14, "");
        log.notes = DB_GET_STRING(res, row, 15, "");
        return log;
    }

    std::expected<std::vector<AccessLog>, std::string> DBQueries::getLogsByDateRange(
        const std::string& from,
        const std::string& to,
        int limit) {

        std::string query = "SELECT * FROM access_logs "
            "WHERE access_time BETWEEN '" + from + "' AND '" + to + "' "
            "ORDER BY access_time DESC "
            "LIMIT " + std::to_string(limit);

        // Используем m_executor->query() вместо m_conn.query()
        auto res = m_executor->query(query);
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

        PGresultPtr result(res.value());
        int rows = PQntuples(result.get());
        std::vector<AccessLog> logs;
        logs.reserve(rows);

        for (int i = 0; i < rows; ++i) {
            logs.push_back(parseAccessLog(result.get(), i));
        }

        return logs;
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
#ifdef _WIN32
        std::string fullName = toUtf8(person.fullName);
        std::string personType = toUtf8(person.personType);
        std::string phone = toUtf8(person.phone);
        std::string email = toUtf8(person.email);
        std::string address = toUtf8(person.address);
        std::string blockReason = toUtf8(person.blockReason);
        std::string verificationCode = toUtf8(person.verificationCode);
        std::string notes = toUtf8(person.notes);
        std::string externalId = toUtf8(person.externalId);
#else
        std::string fullName = person.fullName;
        std::string personType = person.personType;
        std::string phone = person.phone;
        std::string email = person.email;
        std::string address = person.address;
        std::string blockReason = person.blockReason;
        std::string verificationCode = person.verificationCode;
        std::string notes = person.notes;
        std::string externalId = person.externalId;
#endif

        std::string query = "INSERT INTO persons (full_name, person_type, birth_date, "
            "gender, phone, email, address, is_active, is_blocked, "
            "block_reason, blocked_until, verification_type, "
            "verification_code, notes, external_id) VALUES ('" +
            escapeString(fullName) + "', '" + escapeString(personType) + "', " +
            (person.birthDate.empty() ? "NULL" : "'" + person.birthDate + "'") + ", '" +
            person.gender + "', '" + escapeString(phone) + "', '" +
            escapeString(email) + "', '" + escapeString(address) + "', " +
            (person.isActive ? "true" : "false") + ", " +
            (person.isBlocked ? "true" : "false") + ", " +
            (blockReason.empty() ? "NULL" : "'" + escapeString(blockReason) + "'") + ", " +
            (person.blockUntil.empty() ? "NULL" : "'" + person.blockUntil + "'") + ", '" +
            person.verificationType + "', " +
            (verificationCode.empty() ? "NULL" : "'" + escapeString(verificationCode) + "'") + ", " +
            (notes.empty() ? "NULL" : "'" + escapeString(notes) + "'") + ", " +
            (externalId.empty() ? "NULL" : "'" + escapeString(externalId) + "'") +
            ") RETURNING id";

        auto res = m_executor->query(query);
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

        PGresultPtr result(res.value());
        if (PQntuples(result.get()) == 0) {
            return std::unexpected("Failed to insert person");
        }
        return DB_GET_INT(result.get(), 0, 0, 0);
    }

    // Нормализация вектора (делаем длину = 1)

    // ============================================================
    // ПОИСК ПО ЭМБЕДДИНГУ (PGVECTOR)
    // ============================================================

    std::expected<void, std::string> DBQueries::deleteFaceEncoding(int encodingId) {
        std::string query = "DELETE FROM face_encodings WHERE id = " + std::to_string(encodingId);
        return m_executor->execute(query);
    }

    std::expected<std::vector<MatchResult>, std::string> DBQueries::findPersonByEmbedding(
        const std::vector<float>& embedding,
        float threshold,
        int limit) {

        // Нормализуем эмбеддинг для поиска
        std::vector<float> normalizedEmbedding = normalizeEmbedding(embedding);


        std::cout << "🔍 [SEARCH] Поиск по эмбеддингу, порог: " << threshold << std::endl;
        std::cout << "   Первые 10 значений для поиска: ";
        for (int i = 0; i < (int)normalizedEmbedding.size(); i++) {
            std::cout << normalizedEmbedding[i] << " ";
        }

        std::string query = "SELECT * FROM find_person_by_embedding('" +
            vectorToString(normalizedEmbedding) + "'::vector, " +
            std::to_string(threshold) + ") LIMIT " + std::to_string(limit);

        auto res = m_executor->query(query);
        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

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