#pragma once

#include <string>
#include <vector>
#include <optional>
#include <expected>
#include <chrono>

namespace bigiate::db {

    // ============================================================
    // СТРУКТУРЫ ДЛЯ БАЗЫ ДАННЫХ
    // ============================================================

    // Человек (из таблицы persons)
    struct Person {
        int id = 0;
        std::string fullName;
        std::string personType;              // student, guest, teacher, staff
        std::string birthDate;               // YYYY-MM-DD
        char gender = 'M';                   // M, F
        std::string phone;
        std::string email;
        std::string address;
        bool isActive = true;
        bool isBlocked = false;
        std::string blockReason;
        std::string blockUntil;              // YYYY-MM-DD
        std::string verificationType = "face_only";  // face_only, finger_code, pin_code, combo
        std::string verificationCode;
        std::string notes;
        std::string createdAt;
        std::string updatedAt;
        std::string externalId;
    };

    // Студент (расширение)
    struct Student {
        int personId = 0;
        std::string studentCardNumber;
        std::string groupName;
        int course = 1;
        std::string faculty;
        bool isExpelled = false;
        std::string expulsionDate;
        std::string notes;
    };

    // Преподаватель
    struct Teacher {
        int personId = 0;
        std::string employeeId;
        std::string department;
        std::string position;
        std::string notes;
    };

    // Сотрудник
    struct Staff {
        int personId = 0;
        std::string employeeId;
        std::string department;
        std::string position;
        std::string staffCategory;           // admin, security, technical, maintenance, other
        bool isSecurity = false;
        bool canOverrideAccess = false;
        std::string notes;
    };

    // Гость
    struct Guest {
        int personId = 0;
        std::string guestCardId;
        int hostPersonId = 0;
        std::string purpose;
        std::string visitReason;
        std::string validFrom;               // YYYY-MM-DD
        std::string validTo;                 // YYYY-MM-DD
        int maxVisits = 1;
        int visitsCount = 0;
        std::vector<std::string> allowedZones;
        bool needsEscort = false;
        int escortPersonId = 0;
        bool isApproved = false;
        int approvedBy = 0;
        std::string approvedAt;
        std::string notes;
    };

    // Эмбеддинг лица
    struct FaceEncoding {
        int id = 0;
        int personId = 0;
        std::vector<float> embedding;        // 512-мерный вектор
        std::string sourceImagePath;
        std::string captureDate;
        float qualityScore = 0.0f;
        int faceSize = 0;
        bool isPrimary = false;
        bool isCurrent = true;
        std::string expirationDate;          // YYYY-MM-DD, пусто = бессрочно
        std::string notes;
    };

    // Запись в журнале проходов
    struct AccessLog {
        long id = 0;
        int personId = 0;                    // 0 = неизвестный
        std::string accessTime;
        int cameraId = 0;
        std::string gateId;
        std::string direction;               // enter, exit
        bool accessGranted = false;
        std::string accessReason;
        float similarityScore = 0.0f;
        int encodingId = 0;
        bool verificationRequired = false;
        std::string verificationMethod;
        std::string detectedVerificationCode;
        std::string expectedVerificationCode;
        std::string faceImagePath;
        std::string fullFramePath;
        std::string unknownFaceImage;
        std::vector<float> unknownFaceEmbedding;
        bool isForced = false;
        int forcedByPersonId = 0;
        std::string forcedReason;
        std::string notes;
    };

    // Результат распознавания
    struct RecognitionResult {
        Person person;
        float similarity = 0.0f;
        bool verificationRequired = false;
        std::string verificationMethod;
        bool isForced = false;
        float bestMatchSimilarity = 0.0f;  
    };

    // Результат поиска по эмбеддингу
    struct MatchResult {
        int personId = 0;
        int encodingId = 0;
        float similarity = 0.0f;
        std::string personType;
        std::string fullName;
        std::string verificationType;
        std::string verificationCode;
        std::string notes;
    };

} // namespace bigiate::db