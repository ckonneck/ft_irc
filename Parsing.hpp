#ifndef PARSING_HPP
#define PARSING_HPP

#include <string>
#include <vector>
#include "User.hpp"

extern const std::string SERVER_NAME;

struct ParsedCommand {
    std::string             command;
    std::vector<std::string> params;
    std::string             trailing;
};

ParsedCommand parseIrcCommand(const std::string& rawLine);
void           processClientLine(User& user, const std::string& rawLine);

#endif