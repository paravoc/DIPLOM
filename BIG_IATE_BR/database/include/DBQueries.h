// database/include/DBQueries.h
#pragma once

#include <expected>
#include <vector>
#include <memory>
#include "DBModels.h"
#include "DBThreadSafe.h"

namespace bigiate::db {

    class DBQueries {
    public:
        explicit DBQueries(std::shared_ptr<DBConnectionPool> pool);
        ~DBQueries() = default;

        // ========== ЛЮДИ ==========
        [[nodiscard]] std::expected<std::vector<Person>, std::string> getAllPersons(bool onlyActive = true);
        [[nodiscard]] std::expected<Person, std::string> getPersonById(int id);
        [[nodiscard]] std::expected<int, std::string> addPerson(const Person& person);
        [[nodiscard]] std::expected<void, std::string> updatePerson(const Person& person);
        [[nodiscard]] std::expected<void, std::string> blockPerson(int id, const std::string& reason, const std::string& until = "");
        [[nodiscard]] std::expected<void, std::string> unblockPerson(int id);

        // ========== ЭМБЕДДИНГИ ==========
        [[nodiscard]] std::expected<int, std::string> addFaceEncoding(const FaceEncoding& encoding);
        [[nodiscard]] std::expected<std::vector<FaceEncoding>, std::string> getFaceEncodingsByPerson(int personId);
        [[nodiscard]] std::expected<void, std::string> setPrimaryEncoding(int personId, int encodingId);
        [[nodiscard]] std::expected<void, std::string> markEncodingObsolete(int personId);

        // ========== ПОИСК (PGVECTOR) ==========
        [[nodiscard]] std::expected<std::vector<MatchResult>, std::string> findPersonByEmbedding(
            const std::vector<float>& embedding,
            float threshold = 0.75f,
            int limit = 5
        );

        // ========== ЖУРНАЛ ==========
        [[nodiscard]] std::expected<long, std::string> addAccessLog(const AccessLog& log);
        [[nodiscard]] std::expected<std::vector<AccessLog>, std::string> getRecentLogs(int limit = 100);
        [[nodiscard]] std::expected<std::vector<AccessLog>, std::string> getLogsByPerson(int personId, int limit = 100);

        [[nodiscard]] std::expected<std::vector<AccessLog>, std::string> getLogsByDateRange(
            const std::string& from,
            const std::string& to,
            int limit = 1000
        );

        // ========== СТАТИСТИКА ==========
        struct Stats {
            int totalPersons = 0;
            int activePersons = 0;
            int blockedPersons = 0;
            int totalAccessToday = 0;
            int grantedToday = 0;
            int deniedToday = 0;
        };
        [[nodiscard]] std::expected<Stats, std::string> getStats();

        // ========== ТЕСТ ==========
        [[nodiscard]] std::expected<void, std::string> testConnection();

    private:
        [[nodiscard]] std::expected<void, std::string> execute(const std::string& query);
        std::shared_ptr<ThreadSafeExecutor> m_executor;

        // Парсеры
        Person parsePerson(PGresult* res, int row);
        FaceEncoding parseFaceEncoding(PGresult* res, int row);
        AccessLog parseAccessLog(PGresult* res, int row);
        MatchResult parseMatchResult(PGresult* res, int row);

        // Вспомогательные
        std::string vectorToString(const std::vector<float>& vec);
        std::vector<float> stringToVector(const std::string& str);
        std::string escapeString(const std::string& str);
    };

} // namespace bigiate::db