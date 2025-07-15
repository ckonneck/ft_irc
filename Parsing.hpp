#ifndef PARSING_HPP
#define PARSING_HPP

#include "Server.hpp"

// Forward declarations of project types
class User;
class Chatroom;
const std::size_t MAX_NICK_LEN = 9;

extern std::string servername;
extern std::string g_serverPassword;

void handlePing(int fd, const std::string& raw);
void handleNick(User* curr, const std::string& raw, std::vector<pollfd> &fds);
void handleKick(User* requester,
                const std::string& channelName,
                const std::string& targetNick,
                const std::string& reason,
                std::vector<pollfd> &fds);
void handleJoin(User* curr,
                int fd,
                const std::vector<std::string>& tokens,
                std::vector<pollfd>& fds);
void handlePrivmsg(User* curr, int fd, const std::vector<std::string>& tokens, const std::string& raw, std::vector<pollfd> &fds);
void handleInvite(User* curr,
                  const std::string& targetNick,
                  const std::string& channelName);
void handleTopic(User* curr, const std::string& raw, std::vector<std::string> tokens, std::vector<pollfd> &fds);
void handleQuit(User* curr,
                const std::string& reason,
                std::vector<pollfd>& fds);
void handleMode(User* requester,
                const std::string& chanName,
                const std::string& flags,
                const std::vector<std::string>& tokens, std::vector<pollfd> &fds);
void handleUserMode(User* user,
                    const std::string& flags,
                    std::vector<pollfd>& fds);
void handleCap(User* curr, std::vector<std::string> tokens);
void handlePart(User* curr, const std::string& channelName, std::vector<pollfd>& fds);
void handleModeInvite(Chatroom* chan, bool adding, User* requester,
                      const std::string& chanName, std::vector<pollfd>& fds);
void handleModeTopic(Chatroom* chan, bool adding, User* requester,
                     const std::string& chanName, std::vector<pollfd>& fds);
void handleModeKey(Chatroom* chan, bool adding, User* requester,
                   const std::string& chanName,
                   const std::vector<std::string>& tokens,
                   size_t& argIdx, std::vector<pollfd>& fds,
                   std::vector<std::string>& argList);
void handleModeLimit(Chatroom* chan, bool adding, User* requester,
                     const std::string& chanName,
                     const std::vector<std::string>& tokens,
                     size_t& argIdx, std::vector<pollfd>& fds,
                     std::vector<std::string>& argList);
void handleModeOperator(Chatroom* chan, bool adding, User* requester,
                        const std::string& chanName,
                        const std::vector<std::string>& tokens,
                        size_t& argIdx, std::vector<pollfd>& fds,
                        std::vector<std::string>& argList);
void handleModeUnknown(User* requester, char m);
void handleWho(User* curr,
               const std::vector<std::string>& tokens,
               std::vector<pollfd>& fds);
void handleWhois(User* curr,
                 const std::vector<std::string>& tokens,
                 std::vector<pollfd>& fds);
void handlePass(User* user, const std::vector<std::string>& tokens, std::vector<pollfd> &fds);
bool uniqueNick(User* usr);
void handleBanList(User *requester, const std::string &chanName);
bool banQuery(User *requester, const std::string &chanName, const std::string &flags);
std::string putAllLowCase(const std::string& input);
std::string uwuTasticNick();
bool isValidPortRange(char *argv);
#endif // PARSING_HPP
