// src/config/include/config/loader.h
#pragma once

#include <yaml-cpp/yaml.h>  // <--- ЭТО ДОЛЖНО БЫТЬ ПЕРВЫМ ИЛИ РАНЬШЕ

#include "types.h"
#include "macros.h"

#include <expected>
#include <string>
#include <filesystem>

namespace bigiate::config {

	// 1. Главная функция - загружает всё
	[[nodiscard]] std::expected<LoadResult, std::string>
		LoadConfig(const std::string& config_path);

	// 2. Для тестов - только версию
	[[nodiscard]] std::expected<Version, std::string>
		LoadVersionOnly(const std::string& config_path);

	// 3. Проверка файла
	[[nodiscard]] bool
		ConfigFileExists(const std::string& config_path);

	// 4. Для отладки - распечатать конфиг
	void DumpConfig(const ServerConfig& config);

	// Парсинг отдельных секций
	[[nodiscard]] std::expected<CameraConfig, std::string>
		ParseCamera(const YAML::Node& node);

	[[nodiscard]] std::expected<DatabaseConfig, std::string>
		ParseDatabase(const YAML::Node& node);

	[[nodiscard]] std::expected<BastionConfig, std::string>
		ParseBastion(const YAML::Node& node);

	[[nodiscard]] std::expected<RecognitionConfig, std::string>
		ParseRecognition(const YAML::Node& node);

	[[nodiscard]] std::expected<LoggingConfig, std::string>
		ParseLogging(const YAML::Node& node);

	[[nodiscard]] std::expected<SecurityConfig, std::string>
		ParseSecurity(const YAML::Node& node);

	// Валидация
	[[nodiscard]] std::expected<void, std::string>
		ValidateConfig(const ServerConfig& config);

} // namespace bigiate::config