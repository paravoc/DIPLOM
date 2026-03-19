// src/config/include/config/loader.h
#pragma once

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

} // namespace bigiate::config