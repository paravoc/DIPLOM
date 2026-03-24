//==============================================================================
// BIG IATE - Configuration Loader
// loader.h
//==============================================================================
// Описание: Загрузка и парсинг YAML-конфигурации.
//           Основная функция LoadConfig() загружает весь конфиг и секреты.
//
// Автор: paravoc
// Дата: 21.03.2026
// Версия: 1.0.0
//==============================================================================

#pragma once

#include <yaml-cpp/yaml.h>
#include <expected>
#include <string>
#include <filesystem>

#include "types.h"
#include "macros.h"

namespace bigiate::config {

	//==============================================================================
	// ГЛАВНЫЕ ФУНКЦИИ
	//==============================================================================

	// Загружает весь конфиг (версия, камеры, БД, распознавание, логи, безопасность)
	[[nodiscard]] std::expected<LoadResult, std::string>
		LoadConfig(const std::string& config_path);

	// Загружает только версию (для быстрой проверки)
	[[nodiscard]] std::expected<Version, std::string>
		LoadVersionOnly(const std::string& config_path);

	// Проверяет существование файла
	[[nodiscard]] bool
		ConfigFileExists(const std::string& config_path);


	//==============================================================================
	// ПАРСИНГ ОТДЕЛЬНЫХ СЕКЦИЙ
	//==============================================================================

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

	//==============================================================================
	// ВАЛИДАЦИЯ
	//==============================================================================

	[[nodiscard]] std::expected<void, std::string>
		ValidateConfig(const ServerConfig& config);

} // namespace bigiate::config