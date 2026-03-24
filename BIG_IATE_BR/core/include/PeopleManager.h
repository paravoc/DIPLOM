#pragma once

#include <string>
#include <expected>
#include <vector>
#include <optional>
#include <memory>
#include <opencv2/opencv.hpp>
#include "../../configs/include/types.h"

namespace bigiate::db {
    class DBQueries;
}

namespace bigiate::recognition {
    class FaceRecognizer;
}

namespace bigiate::core {

    // Структура для информации о человеке
    struct PersonInfo {
        int id;
        std::string fullName;
        std::string personType;
        std::string birthDate;
        char gender;
        std::string phone;
        std::string email;
        std::string address;
        bool isActive;
        bool isBlocked;
        std::string blockReason;
        std::string blockUntil;
        int encodingCount;
    };

    class PeopleManager {
    public:
        PeopleManager();
        ~PeopleManager();

        PeopleManager(const PeopleManager&) = delete;
        PeopleManager& operator=(const PeopleManager&) = delete;

        // Инициализация
        [[nodiscard]] std::expected<void, std::string> init(const std::string& configPath);

        // Добавить человека из одного фото
        [[nodiscard]] std::expected<int, std::string> addPersonFromPhoto(
            const std::string& imagePath,
            const std::string& fullName,
            const std::string& personType,
            const std::string& birthDate = "",
            char gender = 'M',
            const std::string& phone = "",
            const std::string& email = "",
            const std::string& address = ""
        );

        // Добавить человека из нескольких фото
        [[nodiscard]] std::expected<int, std::string> addPersonFromPhotos(
            const std::vector<std::string>& imagePaths,
            const std::string& fullName,
            const std::string& personType,
            const std::string& birthDate = "",
            char gender = 'M'
        );

        // Добавить фото к существующему человеку
        [[nodiscard]] std::expected<int, std::string> addPhotoToPerson(
            int personId,
            const std::string& imagePath,
            bool setAsPrimary = false
        );

        // Показать всех людей
        [[nodiscard]] std::expected<void, std::string> listAllPersons();

        // Получить информацию о человеке
        [[nodiscard]] std::expected<PersonInfo, std::string> getPersonInfo(int personId);

        // Обновить данные человека
        [[nodiscard]] std::expected<void, std::string> updatePerson(
            int personId,
            const std::string& fullName = "",
            const std::string& personType = "",
            const std::string& birthDate = "",
            std::optional<char> gender = std::nullopt,
            const std::string& phone = "",
            const std::string& email = "",
            const std::string& address = ""
        );

        // Заблокировать/разблокировать
        [[nodiscard]] std::expected<void, std::string> setPersonBlocked(
            int personId,
            bool blocked,
            const std::string& reason = "",
            const std::string& until = ""
        );

        // Удалить человека (мягкое удаление)
        [[nodiscard]] std::expected<void, std::string> deletePerson(int personId);

        // Закрыть соединения
        void close();

        bool isInitialized() const { return m_initialized; }

    private:
        // Вспомогательные функции (объявлены, но реализация в .cpp)
        [[nodiscard]] std::expected<cv::Mat, std::string> loadImage(const std::string& path);
        [[nodiscard]] std::expected<std::vector<float>, std::string> extractEmbedding(const cv::Mat& image);

        // Удаление из БД
        [[nodiscard]] std::expected<void, std::string> hardDeletePerson(int personId);

        struct Impl;
        std::unique_ptr<Impl> m_pimpl;
        bool m_initialized{ false };
    };

} // namespace bigiate::core