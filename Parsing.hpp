#ifndef PARSING_HPP
#define PARSING_HPP

#include <string>
#include <vector>
#include <sys/poll.h>   // for pollfd
#include "Server.hpp"

// Forward declarations of project types
class User;
class Chatroom;

// IRC command representation
struct ParsedCommand {
    std::string command;               // The IRC command (e.g., "PRIVMSG", "JOIN")
    std::vector<std::string> params;   // Parameters before the trailing message
    std::string trailing;              // The trailing message after ':'
};

// Parse a raw IRC line into its components
ParsedCommand parseIrcCommand(const std::string& rawLine);

// Send formatted replies and errors
//void sendReply(Client& client, const std::string& code, const std::string& msg);
//void sendError(Client& client, const std::string& code, const std::string& msg);

// Handle a fully registered client's commands
void handleCommand(const ParsedCommand& cmd, User& client);

// Handle commands during initial client registration (PASS, NICK, USER)
//void handleClientConnectParsing(Client& client, const ParsedCommand& cmd);

// Parse and dispatch raw buffers for connected clients using pollfds
void commandParsing(char* messageBuffer, std::vector<pollfd>& fds, size_t index);

void handlePing(int fd);
void handleNick(User* curr, const std::string& raw);
void handleKick(User* requester,
                const std::string& channelName,
                const std::string& targetNick,
                const std::string& reason);
void handleJoin(User* curr, int fd, const std::string& chanArg);
void handlePrivmsg(User* curr, int fd, const std::vector<std::string>& tokens, const std::string& raw);
void handleInvite(User* curr,
                  const std::string& targetNick,
                  const std::string& channelName);
void handleTopic(User* curr, const std::string& raw);
void handleQuit(int fd);
bool uniqueNick(User* usr);

#endif // PARSING_HPP
