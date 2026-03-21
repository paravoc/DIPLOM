// core/include/SecretsMacros.h
#pragma once

#include <iostream>
#include <string>
#include <expected>

// Макрос для безопасного ввода пароля (без эха)
#ifdef _WIN32
#include <conio.h>
#define SECURE_INPUT(prompt, var) \
    do { \
        std::cout << prompt; \
        var.clear(); \
        char ch; \
        while ((ch = _getch()) != '\r') { \
            if (ch == '\b') { \
                if (!var.empty()) { \
                    var.pop_back(); \
                    std::cout << "\b \b"; \
                } \
            } else { \
                var += ch; \
                std::cout << '*'; \
            } \
        } \
        std::cout << std::endl; \
    } while(0)
#else
#include <termios.h>
#include <unistd.h>
#define SECURE_INPUT(prompt, var) \
    do { \
        std::cout << prompt; \
        termios oldt, newt; \
        tcgetattr(STDIN_FILENO, &oldt); \
        newt = oldt; \
        newt.c_lflag &= ~ECHO; \
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); \
        std::getline(std::cin, var); \
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); \
        std::cout << std::endl; \
    } while(0)
#endif

// Макрос для проверки существования файла
#define CHECK_SECRETS_FILE(path) \
    do { \
        if (!std::filesystem::exists(path)) { \
            return std::unexpected("Secrets file not found: " + path); \
        } \
    } while(0)

// Макрос для безопасной обработки результата
#define TRY_SECRETS(expr) \
    ({ \
        auto _result = (expr); \
        if (!_result.has_value()) { \
            return std::unexpected(_result.error()); \
        } \
        std::move(_result.value()); \
    })

// Макрос для создания шаблона
#define CREATE_SECRETS_TEMPLATE(path) \
    do { \
        std::ofstream file(path); \
        if (!file.is_open()) { \
            return std::unexpected("Cannot create template: " + path); \
        } \
        file << "# BIG IATE - Secrets Template\n"; \
        file << "# Fill in the passwords and run encrypt_secrets.exe\n\n"; \
        file << "database_password: \"\"\n\n"; \
        file << "cameras:\n"; \
        file << "  1:\n"; \
        file << "    username: \"\"\n"; \
        file << "    password: \"\"\n"; \
        file << "  2:\n"; \
        file << "    username: \"\"\n"; \
        file << "    password: \"\"\n\n"; \
        file << "bastion_password: \"\"\n"; \
        file.close(); \
    } while(0)