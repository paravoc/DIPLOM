#include "../include/PeopleManager.h"
#include "../../configs/include/loader.h"
#include "../../database/include/DBConnectionPool.h"
#include "../../database/include/DBQueries.h"
#include "../../database/include/DBModels.h"
#include "../../recognition/include/FaceRecognizer.h"
#include "../../recognition/include/RecognitionResult.h"
#include "../../core/include/SecretsInitializer.h"
#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <iomanip>
#include <chrono>
#include <sstream>

namespace bigiate::core {

    struct PeopleManager::Impl {
        config::LoadResult configResult;
        std::shared_ptr<db::DBQueries> dbQueries;
        std::shared_ptr<recognition::FaceRecognizer> faceRecognizer;
    };

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // ============================================================
    // КОНСТРУКТОРЫ
    // ============================================================

    PeopleManager::PeopleManager() : m_pimpl(std::make_unique<Impl>()) {}
    PeopleManager::~PeopleManager() = default;

    // ============================================================
    // ИНИЦИАЛИЗАЦИЯ
    // ============================================================

    std::expected<void, std::string> PeopleManager::init(const std::string& configPath) {
        std::cout << "\n=== PEOPLE MANAGER INIT ===" << std::endl;

        // 1. Загружаем конфиг
        auto configResult = config::LoadConfig(configPath);
        if (!configResult.has_value()) {
            return std::unexpected("Failed to load config: " + configResult.error());
        }
        m_pimpl->configResult = std::move(configResult.value());

        // 2. Загружаем секреты
        auto secretsResult = secrets::SecretsInitializer::initialize();
        if (!secretsResult.has_value()) {
            return std::unexpected("Failed to load secrets: " + secretsResult.error());
        }

        // 3. Подключаемся к БД
        const auto& dbCfg = m_pimpl->configResult.config.database;
        std::string dbPassword = secretsResult.value().database_password;

        if (dbPassword.empty()) {
            return std::unexpected("Database password is empty");
        }

        auto dbPool = std::make_shared<db::DBConnectionPool>(dbCfg, dbPassword, 1, 2);
        m_pimpl->dbQueries = std::make_shared<db::DBQueries>(dbPool);

        auto testResult = m_pimpl->dbQueries->testConnection();
        if (!testResult.has_value()) {
            return std::unexpected("Database connection failed: " + testResult.error());
        }
        std::cout << "✅ Database connected" << std::endl;

        // 4. Инициализируем распознавание
        m_pimpl->faceRecognizer = std::make_shared<recognition::FaceRecognizer>();  // <-- make_shared
        auto initResult = m_pimpl->faceRecognizer->init(
            m_pimpl->configResult.config.recognition,
            m_pimpl->dbQueries
        );

        if (!initResult.has_value()) {
            return std::unexpected("Face recognizer init failed: " + initResult.error());
        }
        std::cout << "✅ Face recognition initialized" << std::endl;

        m_initialized = true;
        return {};
    }

    // ============================================================
    // ВСПОМОГАТЕЛЬНЫЕ
    // ============================================================
    // В конец файла PeopleManager.cpp добавьте:

    std::shared_ptr<recognition::FaceRecognizer> PeopleManager::getRecognizer() {
        if (!m_initialized) {
            return nullptr;
        }
        return m_pimpl->faceRecognizer;
    }

    std::vector<config::CameraConfig> PeopleManager::getCameras() const {
        if (!m_initialized) {
            return {};
        }
        return m_pimpl->configResult.config.cameras;
    }

    std::expected<cv::Mat, std::string> PeopleManager::loadImage(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            return std::unexpected("File not found: " + path);
        }

        cv::Mat image = cv::imread(path);
        if (image.empty()) {
            return std::unexpected("Failed to load image: " + path);
        }
        return image;
    }

    std::expected<std::vector<float>, std::string> PeopleManager::extractEmbedding(const cv::Mat& image) {
        if (!m_pimpl->faceRecognizer) {
            return std::unexpected("Face recognizer not initialized");
        }

        // Получаем детектор и экстрактор из FaceRecognizer
        auto* detector = m_pimpl->faceRecognizer->getDetector();
        auto* extractor = m_pimpl->faceRecognizer->getExtractor();

        if (!detector || !extractor) {
            return std::unexpected("Detector or extractor not available");
        }

        // Детектируем лицо
        auto detections = detector->detect(image, 0);
        if (detections.empty()) {
            return std::unexpected("No face detected in image");
        }

        // Если несколько лиц, выбираем самое большое
        recognition::Detection* bestDetection = &detections[0];
        for (auto& det : detections) {
            if (det.bbox.area() > bestDetection->bbox.area()) {
                bestDetection = &det;
            }
        }

        // Проверяем размер лица
        int faceSize = std::min(bestDetection->bbox.width, bestDetection->bbox.height);
        if (faceSize < 60) {
            return std::unexpected("Face too small (" + std::to_string(faceSize) + "px)");
        }

        // Извлекаем эмбеддинг
        auto embResult = extractor->extract(bestDetection->faceROI);
        if (!embResult.has_value()) {
            return std::unexpected(embResult.error());
        }

        // Нормализуем эмбеддинг
        std::vector<float> embedding = embResult.value().vector;
        float norm = 0.0f;
        for (float v : embedding) norm += v * v;
        norm = std::sqrt(norm);
        if (norm > 0.0001f) {
            for (float& v : embedding) v /= norm;
        }

        return embedding;
    }
    std::expected<void, std::string> PeopleManager::hardDeletePerson(int personId) {
        db::Person p;
        p.id = personId;
        p.isActive = false;
        return m_pimpl->dbQueries->updatePerson(p);
    }

    // ============================================================
    // ДОБАВЛЕНИЕ ИЗ ОДНОГО ФОТО
    // ============================================================

    std::expected<int, std::string> PeopleManager::addPersonFromPhoto(
        const std::string& imagePath,
        const std::string& fullName,
        const std::string& personType,
        const std::string& birthDate,
        char gender,
        const std::string& phone,
        const std::string& email,
        const std::string& address) {

        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        std::cout << "\n📸 Processing: " << imagePath << std::endl;

        // 1. Загружаем изображение
        auto imageResult = loadImage(imagePath);
        if (!imageResult.has_value()) {
            return std::unexpected(imageResult.error());
        }

        cv::Mat image = imageResult.value();

        // 2. Извлекаем реальный эмбеддинг
        auto embeddingResult = extractEmbedding(image);
        if (!embeddingResult.has_value()) {
            return std::unexpected(embeddingResult.error());
        }
        std::vector<float> embedding = embeddingResult.value();

        std::cout << "✅ Embedding extracted (size: " << embedding.size() << ")" << std::endl;

        if (embedding.empty()) {
            return std::unexpected("Failed to extract embedding");
        }

        std::cout << "✅ Embedding extracted (size: " << embedding.size() << ")" << std::endl;

        // 3. Добавляем человека в БД
        db::Person person;
        person.fullName = fullName;
        person.personType = personType;
        person.birthDate = birthDate;
        person.gender = gender;
        person.phone = phone;
        person.email = email;
        person.address = address;
        person.isActive = true;
        person.isBlocked = false;

        auto personIdResult = m_pimpl->dbQueries->addPerson(person);
        if (!personIdResult.has_value()) {
            return std::unexpected("Failed to add person: " + personIdResult.error());
        }
        int personId = personIdResult.value();

        // 4. Добавляем эмбеддинг
        db::FaceEncoding encoding;
        encoding.personId = personId;
        encoding.embedding = embedding;
        encoding.sourceImagePath = imagePath;
        encoding.captureDate = getCurrentTimestamp();
        encoding.isPrimary = true;
        encoding.isCurrent = true;

        auto encodingIdResult = m_pimpl->dbQueries->addFaceEncoding(encoding);
        if (!encodingIdResult.has_value()) {
            return std::unexpected("Failed to add encoding: " + encodingIdResult.error());
        }

        std::cout << "\n✅ Person added! ID: " << personId << std::endl;
        return personId;
    }

    // ============================================================
    // ДОБАВЛЕНИЕ ИЗ НЕСКОЛЬКИХ ФОТО
    // ============================================================

    std::expected<int, std::string> PeopleManager::addPersonFromPhotos(
        const std::vector<std::string>& imagePaths,
        const std::string& fullName,
        const std::string& personType,
        const std::string& birthDate,
        char gender) {

        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        if (imagePaths.empty()) {
            return std::unexpected("No images provided");
        }

        std::cout << "\n📸 Processing " << imagePaths.size() << " photos..." << std::endl;

        // Сначала добавляем человека
        db::Person person;
        person.fullName = fullName;
        person.personType = personType;
        person.birthDate = birthDate;
        person.gender = gender;
        person.isActive = true;
        person.isBlocked = false;

        auto personIdResult = m_pimpl->dbQueries->addPerson(person);
        if (!personIdResult.has_value()) {
            return std::unexpected("Failed to add person: " + personIdResult.error());
        }
        int personId = personIdResult.value();

        // Обрабатываем каждое фото
        int successCount = 0;

        for (const auto& path : imagePaths) {
            std::cout << "  - " << path << std::endl;

            auto imageResult = loadImage(path);
            if (!imageResult.has_value()) {
                std::cerr << "    ⚠️ " << imageResult.error() << std::endl;
                continue;
            }

            cv::Mat image = imageResult.value();

            // Извлекаем реальный эмбеддинг
            auto embeddingResult = extractEmbedding(image);
            if (!embeddingResult.has_value()) {
                std::cerr << "    ⚠️ " << embeddingResult.error() << std::endl;
                continue;
            }
            std::vector<float> embedding = embeddingResult.value();

            // Сохраняем эмбеддинг с ВСЕМИ полями
            db::FaceEncoding encoding;
            encoding.personId = personId;
            encoding.embedding = embedding;
            encoding.sourceImagePath = path;
            encoding.captureDate = getCurrentTimestamp();
            encoding.qualityScore = 1.0f;           // <-- ДОБАВЛЕНО
            encoding.faceSize = 0;                  // <-- ДОБАВЛЕНО
            encoding.isPrimary = (successCount == 0);
            encoding.isCurrent = true;
            encoding.expirationDate = "";           // <-- ДОБАВЛЕНО
            encoding.notes = "";                    // <-- ДОБАВЛЕНО

            auto encodingIdResult = m_pimpl->dbQueries->addFaceEncoding(encoding);
            if (!encodingIdResult.has_value()) {
                std::cerr << "    ⚠️ Failed to save: " << encodingIdResult.error() << std::endl;
                continue;
            }

            std::cout << "    ✅ Encoding saved (ID: " << encodingIdResult.value() << ")" << std::endl;
            successCount++;
        }

        if (successCount == 0) {
            return std::unexpected("No valid faces extracted");
        }

        std::cout << "\n✅ Person added! ID: " << personId
            << " (encodings: " << successCount << "/" << imagePaths.size() << ")" << std::endl;
        return personId;
    }

    // ============================================================
    // ДОБАВИТЬ ФОТО К СУЩЕСТВУЮЩЕМУ
    // ============================================================

    std::expected<int, std::string> PeopleManager::addPhotoToPerson(
        int personId,
        const std::string& imagePath,
        bool setAsPrimary) {

        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        // Проверяем существование человека
        auto personResult = m_pimpl->dbQueries->getPersonById(personId);
        if (!personResult.has_value()) {
            return std::unexpected("Person not found");
        }

        std::cout << "\n📸 Adding photo for " << personResult.value().fullName << std::endl;

        auto imageResult = loadImage(imagePath);
        if (!imageResult.has_value()) {
            return std::unexpected(imageResult.error());
        }

        cv::Mat image = imageResult.value();

        // Извлекаем реальный эмбеддинг
        auto embeddingResult = extractEmbedding(image);
        if (!embeddingResult.has_value()) {
            return std::unexpected(embeddingResult.error());
        }
        std::vector<float> embedding = embeddingResult.value();

        // Добавляем эмбеддинг с ВСЕМИ полями
        db::FaceEncoding encoding;
        encoding.personId = personId;
        encoding.embedding = embedding;
        encoding.sourceImagePath = imagePath;
        encoding.captureDate = getCurrentTimestamp();
        encoding.qualityScore = 1.0f;           // <-- ДОБАВЛЕНО
        encoding.faceSize = 0;                  // <-- ДОБАВЛЕНО
        encoding.isPrimary = setAsPrimary;
        encoding.isCurrent = true;
        encoding.expirationDate = "";           // <-- ДОБАВЛЕНО
        encoding.notes = "";                    // <-- ДОБАВЛЕНО

        auto encodingIdResult = m_pimpl->dbQueries->addFaceEncoding(encoding);
        if (!encodingIdResult.has_value()) {
            return std::unexpected("Failed to add encoding: " + encodingIdResult.error());
        }

        if (setAsPrimary) {
            m_pimpl->dbQueries->setPrimaryEncoding(personId, encodingIdResult.value());
        }

        std::cout << "✅ Photo added! Encoding ID: " << encodingIdResult.value() << std::endl;
        return encodingIdResult.value();
    }
    // ============================================================
    // ПОКАЗАТЬ ВСЕХ
    // ============================================================

    std::expected<void, std::string> PeopleManager::listAllPersons() {
        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        auto personsResult = m_pimpl->dbQueries->getAllPersons(true);
        if (!personsResult.has_value()) {
            return std::unexpected(personsResult.error());
        }

        const auto& persons = personsResult.value();

        std::cout << "\n========================================" << std::endl;
        std::cout << "PERSONS IN DATABASE (" << persons.size() << ")" << std::endl;
        std::cout << "========================================" << std::endl;

        for (const auto& p : persons) {
            std::cout << "ID: " << p.id
                << " | " << p.fullName
                << " | " << p.personType
                << " | Active: " << (p.isActive ? "Yes" : "No")
                << " | Blocked: " << (p.isBlocked ? "Yes" : "No") << std::endl;
        }

        return {};
    }

    // ============================================================
    // ИНФОРМАЦИЯ О ЧЕЛОВЕКЕ
    // ============================================================

    std::expected<PersonInfo, std::string> PeopleManager::getPersonInfo(int personId) {
        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        auto personResult = m_pimpl->dbQueries->getPersonById(personId);
        if (!personResult.has_value()) {
            return std::unexpected(personResult.error());
        }

        const auto& p = personResult.value();

        // Получаем количество эмбеддингов
        auto encodingsResult = m_pimpl->dbQueries->getFaceEncodingsByPerson(personId);
        int encodingCount = encodingsResult.has_value() ? static_cast<int>(encodingsResult.value().size()) : 0;

        PersonInfo info;
        info.id = p.id;
        info.fullName = p.fullName;
        info.personType = p.personType;
        info.birthDate = p.birthDate;
        info.gender = p.gender;
        info.phone = p.phone;
        info.email = p.email;
        info.address = p.address;
        info.isActive = p.isActive;
        info.isBlocked = p.isBlocked;
        info.blockReason = p.blockReason;
        info.blockUntil = p.blockUntil;
        info.encodingCount = encodingCount;

        return info;
    }

    // ============================================================
    // ОБНОВИТЬ
    // ============================================================

    std::expected<void, std::string> PeopleManager::updatePerson(
        int personId,
        const std::string& fullName,
        const std::string& personType,
        const std::string& birthDate,
        std::optional<char> gender,
        const std::string& phone,
        const std::string& email,
        const std::string& address) {

        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        auto personResult = m_pimpl->dbQueries->getPersonById(personId);
        if (!personResult.has_value()) {
            return std::unexpected(personResult.error());
        }

        db::Person p = personResult.value();
        if (!fullName.empty()) p.fullName = fullName;
        if (!personType.empty()) p.personType = personType;
        if (!birthDate.empty()) p.birthDate = birthDate;
        if (gender.has_value()) p.gender = gender.value();
        if (!phone.empty()) p.phone = phone;
        if (!email.empty()) p.email = email;
        if (!address.empty()) p.address = address;

        return m_pimpl->dbQueries->updatePerson(p);
    }

    // ============================================================
    // БЛОКИРОВКА
    // ============================================================

    std::expected<void, std::string> PeopleManager::setPersonBlocked(
        int personId,
        bool blocked,
        const std::string& reason,
        const std::string& until) {

        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        if (blocked) {
            return m_pimpl->dbQueries->blockPerson(personId, reason, until);
        }
        else {
            return m_pimpl->dbQueries->unblockPerson(personId);
        }
    }

    // ============================================================
    // УДАЛИТЬ
    // ============================================================

    std::expected<void, std::string> PeopleManager::deletePerson(int personId) {
        if (!m_initialized) {
            return std::unexpected("PeopleManager not initialized");
        }

        return hardDeletePerson(personId);
    }

    // ============================================================
    // ЗАКРЫТЬ
    // ============================================================

    void PeopleManager::close() {
        if (m_pimpl->dbQueries) {
            m_pimpl->dbQueries.reset();
        }
        if (m_pimpl->faceRecognizer) {
            m_pimpl->faceRecognizer.reset();
        }
        m_initialized = false;
    }

} // namespace bigiate::core