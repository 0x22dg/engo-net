#include "command_parser.hpp"

#include <sstream>
#include <unordered_map>

#include "command_type.hpp"

CommandParser::CommandParser() {}

Command CommandParser::parse(const std::string& input) {
    if (input.empty()) return {UNKNOWN, {}};

    Command res{};

    std::istringstream ss(input);
    std::string        arg;

    bool b = false;
    while (ss >> arg) {
        if (!b) {
            res.type = convert_type(arg);
            if (res.type == UNKNOWN) return res;

            b = 1;
            continue;
        }

        res.args.push_back(arg);
    }

    return res;
}

CommandType CommandParser::convert_type(const std::string& cmd) const {
    static std::unordered_map<std::string, CommandType> map = {
        {"exit", EXIT}, {"help", HELP},       {"start", START},
        {"stop", STOP}, {"connect", CONNECT}, {"profile", PROFILE}};

    auto it = map.find(cmd);
    if (it != map.end()) {
        return it->second;
    }

    return UNKNOWN;
}
