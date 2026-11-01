#include "database.hpp"

#include <sqlite3.h>

#include <filesystem>
#include <fstream>
#include <iostream>

Database::Database() {
    std::string file_name = "data";
    std::string path = engo::filesystem::get_main_dir(1) + file_name;

    if (!std::filesystem::exists(path)) {
        std::ofstream file(path);
        file.close();
    }

    int res = sqlite3_open(path.c_str(), &src);
    if (res) throw std::runtime_error("failed to open the db: " + path);
}

Database::~Database() {
    if (src) {
        sqlite3_close(src);
    }
}

bool Database::execute(const std::string& query) {
    if (query.empty()) return false;

    char* err_msg = NULL;
    int   rc = sqlite3_exec(src, query.c_str(), 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

std::unique_ptr<DB_stmt> Database::prepare(const std::string& query) {
    if (query.empty()) return nullptr;

    std::unique_ptr<DB_stmt> stmt = std::make_unique<DB_stmt>();
    if (sqlite3_prepare_v2(src, query.c_str(), -1, &stmt->get(), nullptr) !=
        SQLITE_OK)
        return nullptr;

    return stmt;
}
