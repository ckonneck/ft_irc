#include "Parsing.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>

const std::string SERVER_NAME = "irc.42.fr";

ParsedCommand parseIrcCommand(const std::string& rawLine) {
    ParsedCommand result;
    std::string line = rawLine;
    // Strip CR and LF at the end of the line
    while (!line.empty()) {
        char c = line[line.size() - 1];
        if (c == '\r' || c == '\n') {
            line.erase(line.size() - 1, 1);
        } else {
            break;
        }
    }
    // Extract trailing part after " :"
    size_t pos = line.find(" :");
    if (pos != std::string::npos) {
        result.trailing = line.substr(pos + 2);
        line = line.substr(0, pos);
    }
    // Tokenize command and parameters
    std::istringstream iss(line);
    if (iss >> result.command) {
        std::string param;
        while (iss >> param) {
            result.params.push_back(param);
        }
        std::transform(result.command.begin(), result.command.end(),
                       result.command.begin(), ::toupper);
    }
    return result;
}

static void sendReply(User& user, const std::string& code, const std::string& msg) {
    std::ostringstream oss;
    oss << ":" << SERVER_NAME << " " << code
        << " " << user.getNickname() << " " << msg << "\r\n";
    std::string out = oss.str();
    if (::send(user.getFD(), out.c_str(), out.size(), 0) == -1) {
        std::cerr << "sendReply error: " << strerror(errno) << std::endl;
    }
}

static void sendError(User& user, const std::string& code, const std::string& msg) {
    std::ostringstream oss;
    oss << ":" << SERVER_NAME << " " << code
        << " " << user.getNickname() << " " << msg << "\r\n";
    std::string out = oss.str();
    if (::send(user.getFD(), out.c_str(), out.size(), 0) == -1) {
        std::cerr << "sendError error: " << strerror(errno) << std::endl;
    }
}

static void handleClientConnectParsing(User& user, const ParsedCommand& cmd) {
    if (cmd.command == "PASS") {
        if (cmd.params.empty()) { sendError(user, "461", "PASS :Not enough parameters"); return; }
        user.tempPass = cmd.params[0]; user.hasPass = true;
    } else if (cmd.command == "NICK") {
        if (cmd.params.empty()) { sendError(user, "431", ":No nickname given"); return; }
        user.tempNick = cmd.params[0]; user.hasNick = true;
    } else if (cmd.command == "USER") {
        if (cmd.params.size() < 4) { sendError(user, "461", "USER :Not enough parameters"); return; }
        user.tempUser = cmd.params[0];
        user.tempRealname = cmd.trailing;
        user.hasUser = true;
    }

    if (user.hasPass && user.hasNick && user.hasUser && !user.isRegistered()) {
        user.setRealName(user.tempRealname);
        user.setRegistered(true);
        g_mappa.push_back(&user);
        sendReply(user, "001", "Welcome to " + SERVER_NAME + ", " + user.getNickname());
        sendReply(user, "002", "Your host is " + SERVER_NAME);
        sendReply(user, "003", "This server was created for 42 ft_irc project");
        sendReply(user, "004", "Server modes are +ix");
    }
}

static void handleCommand(const ParsedCommand& /*cmd*/, User& /*user*/) {
    // TODO: implement JOIN, PRIVMSG, PING, etc.
}

void processClientLine(User& user, const std::string& rawLine) {
    ParsedCommand cmd = parseIrcCommand(rawLine);
    if (!user.isRegistered())
        handleClientConnectParsing(user, cmd);
    else
        handleCommand(cmd, user);
}
