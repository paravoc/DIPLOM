#pragma once
#include <expected>
#include <vector>
#include"../../configs/include/types.h"
#include "DBModels.h"

namespace bigiate::db {

    class DBQueries {
    public:
        static std::expected<void, std::string> init(const DatabaseConfig& cfg);
        static void close();

        // Люди
        static std::expected<std::vector<Person>, std::string> getAllPersons();
        static std::expected<Person, std::string> getPersonById(int id);
        static std::expected<int, std::string> addPerson(const Person& person);
        static std::expected<void, std::string> updatePerson(int id, const Person& person);

        // Эмбеддинги
        static std::expected<std::vector<FaceEncoding>, std::string> getFaceEncodings(int personId);
        static std::expected<void, std::string> addFaceEncoding(const FaceEncoding& encoding);
        static std::expected<void, std::string> markEncodingInactive(int id);

        // Поиск
        static std::expected<std::vector<RecognitionResult>, std::string>
            findPersonByEmbedding(const std::vector<float>& embedding, float threshold = 0.75f);

        // Журнал
        static std::expected<long, std::string> addAccessLog(const AccessLog& log);
        static std::expected<std::vector<AccessLog>, std::string> getRecentLogs(int limit = 100);
        static std::expected<std::vector<AccessLog>, std::string> getLogsByPerson(int personId, int limit = 100);
    };

} // namespace bigiate::db