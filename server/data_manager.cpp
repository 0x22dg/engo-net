#pragma once

#include "data_manager.hpp"

DataManager::DataManager(Logger& logger) : logger(logger) {
    const char* query =
        "CREATE TABLE IF NOT EXISTS users(nickname TEXT UNIQUE, pub_key TEXT "
        "UNIQUE)";

    if (!db.execute(query))
        throw std::runtime_error("failed to init server db table");

    logger.log(Logger::INFO, "data manager has been init");
}

bool DataManager::add_user(const std::string&          nickname,
                           const std::vector<uint8_t>& key) {
    const char* query = "INSERT INTO users(nickname, pub_key) VALUES (?);";

    auto stmt = db.prepare(query);
    if (!stmt) {
        logger.log(Logger::ERROR, "data manager: preparing stmt");
        return false;
    }

    stmt->bind(1, nickname);
    stmt->bind(2, std::string(key.begin(), key.end()));

    if (sqlite3_step(stmt->get()) != SQLITE_DONE) {
        logger.log(Logger::ERROR,
                   "data manager: stmt step for adding the user");
        return false;
    }

    return true;
}

bool DataManager::contain_nickname(const std::string& nickname) noexcept {
    const char* query = "SELECT 1 FROM users WHERE nickname = ? LIMIT 1;";

    auto stmt = db.prepare(query);
    if (!stmt) return false;

    sqlite3_bind_text(stmt->get(), 1, nickname.c_str(), -1, SQLITE_TRANSIENT);

    return sqlite3_step(stmt->get()) == SQLITE_ROW;
}

Database& DataManager::get_db() noexcept { return db; }
