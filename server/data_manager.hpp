#pragma once

#include <cstdint>
#include <vector>

#include "../common/data/database.hpp"
#include "../common/logger.hpp"

// service for handling server db and keeping sent messages buffer

class DataManager {
   public:
    DataManager(Logger& logger);
    ~DataManager() = default;

    bool add_user(const std::string& nickname, const std::vector<uint8_t>& key);
    bool contain_nickname(const std::string& nickname) noexcept;

    Database& get_db() noexcept;

   private:
    Logger& logger;

    Database db;
};
