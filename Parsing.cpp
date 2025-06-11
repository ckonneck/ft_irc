#include "Parsing.hpp"
#include "Chatroom.hpp"
#include "User.hpp"
#include "Chatroom.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include <algorithm>
#include "User.hpp"
#include "Chatroom.hpp"
#include <cstdlib> 
#include <sstream>  


extern std::string servername;

void commandParsing(const std::string &messagebuffer, std::vector<pollfd> &fds, size_t i)
{
    std::string raw(messagebuffer);
    std::vector<std::string> tokens = split(raw, ' ');
    if (tokens.empty()) return;

    int fd = fds[i].fd;
    User *curr = findUserByFD(fd);

    const std::string &cmd = tokens[0];
    if (cmd == "PING")
    {
        //std::cout << "tokens1 is: " << tokens[1] << std::endl;
        curr->appendToSendBuffer("PONG :" + tokens[1] +"\r\n");
        //handlePing(fd, raw);
    }
    else if (cmd == "NICK" && tokens.size() > 1)
        handleNick(curr, raw, fds);
    else if (cmd == "KICK" && tokens.size() > 2)
    {
        std::string channelName = tokens[1];
        std::string targetNick  = tokens[2];

        // grab optional reason after the first " :"
        std::string reason;
        size_t pos = raw.find(" :");
        if (pos != std::string::npos)
            reason = raw.substr(pos + 2);

        handleKick(curr, channelName, targetNick, reason, fds);
    }
    else if (cmd == "JOIN" && tokens.size() > 1)
        handleJoin(curr, fd, tokens, fds);
    else if (cmd == "PRIVMSG")
        handlePrivmsg(curr, fd, tokens, raw, fds);
    else if (cmd == "INVITE" && tokens.size() > 2)
        handleInvite(curr, tokens[1], tokens[2]);
    else if (cmd == "TOPIC" && tokens.size() > 1)
        handleTopic(curr, raw, tokens, fds);
    else if (cmd == "MODE" && tokens.size() > 2) {
        const std::string &target = tokens[1];
        const std::string &flags  = tokens[2];

    if (!target.empty() && target[0] == '#') {
        // channel mode
        handleMode(curr, target, flags, tokens, fds);
    }
    else if (target == curr->getNickname()) {
        // user mode
        handleUserMode(curr, flags, fds);
    }
    else {
        // neither channel nor self → no such nick
        std::ostringstream err;
        err << ":" << servername
            << " 401 " << curr->getNickname()
            << " "       << target
            << " :No such nick\r\n";
        curr->appendToSendBuffer(err.str());
    }
}
    else if (cmd == "PART" && tokens.size() > 1)
        handlePart(curr, tokens[1], fds);
    else if (cmd == "QUIT")
    {
        std::string reason;
        size_t pos = raw.find(" :");
        if (pos != std::string::npos)
            reason = raw.substr(pos + 2);
        handleQuit(curr ,reason, fds);
    }
    else if (cmd == "CAP")
        handleCap(curr, tokens);
    else if (cmd == "WHOIS")
        handleWhois(curr, tokens, fds);
    else if (cmd == "WHO")
        handleWho(curr, tokens, fds);
}

void handleWho(User* curr,
               const std::vector<std::string>& tokens,
               std::vector<pollfd>& fds)
{
    // 1) Missing target → ERR_NONICKNAMEGIVEN (same numeric as WHOIS)
    if (tokens.size() < 2 || tokens[1].empty()) {
        std::string reply = ":" + servername
                          + " 431 " + curr->getNickname()
                          + " :No nickname given\r\n";
        curr->appendToSendBuffer(reply);
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].fd == curr->getFD()) {
                fds[i].events |= POLLOUT;
                break;
            }
        }
        return;
    }

    // 2) Build the mask (we’ll treat it as a literal nick only)
    const std::string mask = tokens[1];

    // 3) For each user in g_mappa, if nickname matches mask exactly, send a 352
    for (std::vector<User*>::const_iterator it = g_mappa.begin();
         it != g_mappa.end(); ++it)
    {
        User* u = *it;
        if (u->getNickname() == mask) {
            // 352 RPL_WHOREPLY
            std::ostringstream who;
            who << ":" << servername
                << " 352 " << curr->getNickname()
                << " "    << mask
                << " "    << u->getUsername()
                << " "    << u->getHostname()
                << " "    << servername   // server field
                << " "    << u->getNickname()
                << " H :0 "             // flags + hops
                << u->getRealname()
                << "\r\n";
            curr->appendToSendBuffer(who.str());
        }
    }

    // 4) Regardless of match or not, end with 315 RPL_ENDOFWHO
    std::ostringstream end;
    end << ":" << servername
        << " 315 " << curr->getNickname()
        << " "    << mask
        << " :End of WHO list\r\n";
    curr->appendToSendBuffer(end.str());

    // 5) Flag POLLOUT once
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == curr->getFD()) {
            fds[i].events |= POLLOUT;
            break;
        }
    }
}


void handlePart(User* curr,
                const std::string& channelName,
                std::vector<pollfd>& fds)
{
    std::cout << "PERPARING TO PART" << std::endl;
    std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(channelName);
    if (it == g_chatrooms.end()) {
        std::string err = std::string(":")
            + servername
            + " 403 "
            + curr->getNickname()
            + " "
            + channelName
            + " :No such channel\r\n";
        curr->appendToSendBuffer(err);
        return;
    }
    Chatroom* chan = it->second;

    if (! chan->isMember(curr)) {
        std::string err = std::string(":")
            + servername
            + " 442 "
            + curr->getNickname()
            + " "
            + channelName
            + " :You're not on that channel\r\n";
        curr->appendToSendBuffer(err);
        return;
    }

    std::string prefix = curr->getNickname() 
                       + std::string("!") 
                       + curr->getUsername() 
                       + std::string("@") 
                       + curr->getHostname();
    std::string partLine = std::string(":")
        + prefix
        + " PART "
        + channelName
        + "\r\n";

    chan->broadcast(partLine, curr, fds);

    curr->removeChatroom(channelName);
    chan->removeUser(curr);

    curr->appendToSendBuffer(partLine);
}


void handleQuit(User* curr,
                const std::string& reason,
                std::vector<pollfd>& fds)
{
    std::string prefix = curr->getNickname() 
                       + std::string("!") 
                       + curr->getUsername() 
                       + std::string("@") 
                       + curr->getHostname();
    std::string partLine = std::string(":")
        + prefix
        + " QUIT "
        + reason
        + "\r\n";
//find in which channels the user is
   std::map<std::string, Chatroom*>& chatrooms = curr->getChatrooms();

    for (std::map<std::string, Chatroom*>::iterator it = chatrooms.begin(); it != chatrooms.end(); ++it)
    {
        Chatroom* room = it->second;
        if (room)
        {
            room->broadcast(partLine, curr, fds);
        }
    }
    curr->leaveAllChatrooms();
    curr->appendToSendBuffer(partLine);
}


void handleCap(User* curr, std::vector<std::string> tokens)
    {
        if (tokens.size() < 2)
            return;
    
        const std::string &subcmd = tokens[1];
    
        if (subcmd == "LS")
        {
            // Send a list of supported capabilities
            std::string caps = "multi-prefix";
            std::string msg = ":" + servername + " CAP * LS :" + caps + "\r\n";
            curr->appendToSendBuffer(msg);
        }
    }


bool isSpaceOrNewline(char c) {
    return std::isspace(static_cast<unsigned char>(c)) || c == '\r' || c == '\n';
}


void handleUserMode(User* user,
                    const std::string& flags,
                    std::vector<pollfd>& fds)
{
    bool adding = true;

    // strip whitespace/newlines
    std::string clean = flags;
    clean.erase(
        std::remove_if(clean.begin(), clean.end(), isSpaceOrNewline),
        clean.end()
    );

    for (std::string::const_iterator it = clean.begin(); it != clean.end(); ++it) {
        char m = *it;
        if      (m == '+')            { adding = true; continue; }
        else if (m == '-')            { adding = false; continue; }
        else if (m == 'i')            { user->setInvisible(adding); }
        else /* unsupported flag */   { /* optionally reply ERR_UMODEUNKNOWNFLAG */ }
    }

    // ACK back to the client
    std::ostringstream reply;
    reply << ":" << servername
          << " MODE " << user->getNickname()
          << " "   << flags
          << "\r\n";
    user->appendToSendBuffer(reply.str());

    // ensure we POLLOUT on this fd
    int fd = user->getFD();
    for (std::vector<pollfd>::iterator p = fds.begin(); p != fds.end(); ++p) {
        if (p->fd == fd) { p->events |= POLLOUT; break; }
    }
}


void handleMode(User* requester,
                const std::string& chanName,
                const std::string& flags,
                const std::vector<std::string>& tokens,
                std::vector<pollfd>& fds)
{
    // 1) Look up channel
    std::map<std::string, Chatroom*>::iterator itChan =
        g_chatrooms.find(chanName);
    if (itChan == g_chatrooms.end()) {
        requester->appendToSendBuffer(":" + servername +
            " 403 " + requester->getNickname() +
            " " + chanName +
            " :No such channel\r\n");
        return;
    }
    Chatroom* chan = itChan->second;

    // 2) Membership check
    if (!chan->isMember(requester)) {
        requester->appendToSendBuffer(":" + servername +
            " 442 " + requester->getNickname() +
            " " + chanName +
            " :You're not on that channel\r\n");
        return;
    }

    if (banQuery(requester, chanName, flags) == true)
        return;

    // 3) Operator check
    if (!chan->isOperator(requester)) {
        requester->appendToSendBuffer(":" + servername +
            " 482 " + requester->getNickname() +
            " " + chanName +
            " :You're not channel1 operator\r\n");
        return;
    }


    // 4) Prepare for per-letter handling
    bool adding = true;
    size_t argIdx = 3;  // first mode‐parameter index
    std::vector<std::string> argList;

    // Remove whitespace/newlines from flags
    std::string cleanFlags = flags;
    cleanFlags.erase(
    std::remove_if(cleanFlags.begin(), cleanFlags.end(), isSpaceOrNewline),
    cleanFlags.end());
    // 5) Loop over each mode character (C++98 iterator style)
    for (std::string::const_iterator it = cleanFlags.begin();
         it != cleanFlags.end(); ++it)
    {
        char m = *it;
        if (m == '+') {
            adding = true;
            continue;
        }
        if (m == '-') {
            adding = false;
            continue;
        }

        if (m == 'i') {
            handleModeInvite(chan, adding, requester, chanName, fds);
        }
        else if (m == 't') {
            handleModeTopic(chan, adding, requester, chanName, fds);
        }
        else if (m == 'k') {
            handleModeKey(chan, adding, requester, chanName,
                          tokens, argIdx, fds, argList);
        }
        else if (m == 'l') {
            handleModeLimit(chan, adding, requester, chanName,
                            tokens, argIdx, fds, argList);
        }
        else if (m == 'o') {
            handleModeOperator(chan, adding, requester, chanName,
                               tokens, argIdx, fds, argList);
        }
        else {
            handleModeUnknown(requester, m);
        }
    }

    // 6) Broadcast the final "MODE" line with any parameters
    std::ostringstream oss;
    oss << ":" << requester->getNickname()
        << "!" << requester->getUsername()
        << "@" << requester->getHostname()
        << " MODE " << chanName
        << " " << flags;
    for (size_t i = 0; i < argList.size(); ++i) {
        oss << " " << argList[i];
    }
    oss << "\r\n";
    chan->broadcast(oss.str(), NULL, fds);
}

bool banQuery(User *requester, const std::string &chanName, const std::string &flags)
{
    std::string cleanFlags = flags;
    cleanFlags.erase(
    std::remove_if(cleanFlags.begin(), cleanFlags.end(), isSpaceOrNewline),
    cleanFlags.end());
    for (std::string::const_iterator it = cleanFlags.begin();
         it != cleanFlags.end(); ++it)
    {
        char m = *it;
        if (m == 'b') {
            handleBanList(requester, chanName);
            return true;
        }
    }
    return false;
}

void handleBanList(User *requester, const std::string &chanName)
{
    std::ostringstream oss; //test again
    oss << ":" << "localhost"
        << " 368 " << requester->getNickname()
        << " " << chanName
        << " :End of Channel Ban List"
        << "\r\n";
    std::cout << oss.str() << std::endl;
    requester->appendToSendBuffer(oss.str());

}


void handleModeInvite(Chatroom* chan, bool adding, User* requester,
                      const std::string& chanName, std::vector<pollfd>& fds)
{
    chan->setInviteOnly(adding);
    std::ostringstream msg;
    msg << ":" << servername << " NOTICE " << chanName
        << " :Channel is now "
        << (adding ? "invite-only" : "open")
        << ", set by " << requester->getNickname()
        << "\r\n";
    chan->broadcast(msg.str(), NULL, fds);
}


void handleModeTopic(Chatroom* chan, bool adding, User* requester,
                     const std::string& chanName, std::vector<pollfd>& fds)
{
    chan->setTopicOnlyOps(adding);
    std::ostringstream msg;
    if (adding) {
        msg << ":" << servername << " NOTICE " << chanName
            << " :Only ops can change topic now, set by "
            << requester->getNickname() << "\r\n";
    } else {
        msg << ":" << servername << " NOTICE " << chanName
            << " :Everyone can change topic now, set by "
            << requester->getNickname() << "\r\n";
    }
    chan->broadcast(msg.str(), NULL, fds);
}


void handleModeKey(Chatroom* chan, bool adding, User* requester,
                   const std::string& chanName,
                   const std::vector<std::string>& tokens,
                   size_t& argIdx, std::vector<pollfd>& fds,
                   std::vector<std::string>& argList)
{
    if (adding) {
        if (tokens.size() <= argIdx) {
            // Not enough params
            requester->appendToSendBuffer(":" + servername +
                " 461 " + requester->getNickname() +
                " MODE :Not enough parameters\r\n");
        } else {
            const std::string& keyParam = tokens[argIdx];
            chan->setKey(keyParam);
            std::ostringstream msg;
            msg << ":" << servername << " NOTICE " << chanName
                << " :Channel key set by " << requester->getNickname()
                << "\r\n";
            chan->broadcast(msg.str(), NULL, fds);
            argList.push_back(keyParam);
            argIdx++;
        }
    } else {
        // Removing key
        chan->unsetKey();
        std::ostringstream msg;
        msg << ":" << servername << " NOTICE " << chanName
            << " :Channel key removed by " << requester->getNickname()
            << "\r\n";
        chan->broadcast(msg.str(), NULL, fds);
    }
}

void handleModeLimit(Chatroom* chan, bool adding, User* requester,
                     const std::string& chanName,
                     const std::vector<std::string>& tokens,
                     size_t& argIdx, std::vector<pollfd>& fds,
                     std::vector<std::string>& argList)
{
    if (adding) {
        if (tokens.size() <= argIdx) {
            // Not enough params
            requester->appendToSendBuffer(":" + servername +
                " 461 " + requester->getNickname() +
                " MODE :Not enough parameters\r\n");
        } else {
            int lim = std::atoi(tokens[argIdx].c_str());
            if (lim <= 0) {
                // Invalid limit
                requester->appendToSendBuffer(":" + servername +
                    " 461 " + requester->getNickname() +
                    " MODE :Invalid user limit\r\n");
            } else {
                chan->setLimit(lim);
                std::ostringstream msg;
                msg << ":" << servername << " NOTICE " << chanName
                    << " :User limit set to " << lim
                    << " by " << requester->getNickname()
                    << "\r\n";
                chan->broadcast(msg.str(), NULL, fds);
                argList.push_back(tokens[argIdx]);
                argIdx++;
            }
        }
    } else {
        // Removing limit
        chan->unsetLimit();
        std::ostringstream msg;
        msg << ":" << servername << " NOTICE " << chanName
            << " :User limit removed by " << requester->getNickname()
            << "\r\n";
        chan->broadcast(msg.str(), NULL, fds);
    }
}

void handleModeOperator(Chatroom* chan, bool adding, User* requester,
                        const std::string& chanName,
                        const std::vector<std::string>& tokens,
                        size_t& argIdx, std::vector<pollfd>& fds,
                        std::vector<std::string>& argList)
{
    if (tokens.size() <= argIdx) {
        // Missing nickname parameter
        requester->appendToSendBuffer(":" + servername +
            " 461 " + requester->getNickname() +
            " MODE :Not enough parameters\r\n");
    } else {
        const std::string& nickArg = tokens[argIdx];
        std::string cleanNick = sanitize(nickArg);
        argList.push_back(cleanNick);
        argIdx++;

        User* globalU = findUserByNickname(cleanNick);
        if (!globalU) {
            requester->appendToSendBuffer(":" + servername +
                " 401 " + requester->getNickname() +
                " " + nickArg +
                " :No such nick/channel\r\n");
        } else {
            User* memberU = chan->findUserByNick(cleanNick);
            if (!memberU) {
                requester->appendToSendBuffer(":" + servername +
                    " 441 " + requester->getNickname() +
                    " " + nickArg +
                    " " + chanName +
                    " :They aren't on that channel\r\n");
            } else {
                if (adding) {
                    if (!chan->isOperator(memberU)) {
                        chan->addOperator(memberU);
                        std::ostringstream msg;
                        msg << ":" << servername << " NOTICE " << chanName
                            << " :" << cleanNick
                            << " is now a channel operator (set by "
                            << requester->getNickname() << ")\r\n";
                        chan->broadcast(msg.str(), NULL, fds);
                    }
                } else {
                    if (chan->isOperator(memberU)) {
                        chan->removeOperator(memberU);
                        std::ostringstream msg;
                        msg << ":" << servername << " NOTICE " << chanName
                            << " :" << cleanNick
                            << " is no longer a channel operator (unset by "
                            << requester->getNickname() << ")\r\n";
                        chan->broadcast(msg.str(), NULL, fds);
                    }
                }
            }
        }
    }
}

void handleModeUnknown(User* requester, char m)
{
    std::string modeChar(1, m);
    requester->appendToSendBuffer(":" + servername +
        " 472 " + requester->getNickname() +
        " " + modeChar +
        " :is unknown mode char to me\r\n");
}


void handlePing(int fd, const std::string& raw)
{
    // Extract everything after the first space (the token)
    size_t pos = raw.find(' ');
    std::string token = (pos != std::string::npos)
                      ? raw.substr(pos + 1)
                      : servername;

    // Strip any trailing CR/LF
    token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
    token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());

    User *us = findUserByFD(fd);
    us->appendToSendBuffer("PONG " + token + "\r\n");
}

void handleNick(User* curr, const std::string& raw, std::vector<pollfd> &fds)
{
    std::cout << "we nickhandling" << std::endl;
    std::string oldnick = curr->getNickname();
    std::string newnick = parseNick(raw);

    if (newnick.size() > MAX_NICK_LEN)
    {
        std::string truncated = newnick.substr(0, MAX_NICK_LEN);
        // Inform the user
        std::ostringstream notice;
        notice << ":" << servername
               << " NOTICE " << newnick
               << " :Your nickname has been truncated to " << truncated
               << "\r\n";
        curr->appendToSendBuffer(notice.str());
        newnick = truncated;
    }

    User *check = findUserByNickname(newnick);
    std::cout << "we before" << std::endl;
    if (findUserByNickname(newnick) != NULL && check->getFD() != curr->getFD())
    {std::cout << "we in" << std::endl;
        std::ostringstream oss;
        oss << "NICK " << uwuTasticNick();
        std::string temp = oss.str();
        std::string err = ":localhost 433 * " + newnick + " :Nickname is already in use\r\n";
        //curr->appendToSendBuffer(err);
        curr->appendToSendBuffer("Either the nickname was already taken, or you tried to steal someone else's nickname.\r\n");
        curr->appendToSendBuffer("Doesn't matter. you get a BETTER ONE NOW\r\n");
        curr->setNickname(parseNick(temp));
        curr->HSNick(oldnick, parseNick(temp), fds);
        
        return;
    }
    std::cout << "we under" << std::endl;
     curr->setNickname(newnick);
    if (! uniqueNick(curr)) {
        curr->setNickname(oldnick);
        return;
    }
    std::cout << "we shaking hands" << std::endl;
    curr->HSNick(oldnick, newnick, fds);
}

void handleWhois(User* curr,
                 const std::vector<std::string>& tokens,
                 std::vector<pollfd>& fds)
{
    // 431 ERR_NONICKNAMEGIVEN: client sent "WHOIS" with no nick
    if (tokens.size() < 2 || tokens[1].empty()) {
        std::string reply = ":" + servername
                          + " 431 " + curr->getNickname()
                          + " :No nickname given\r\n";
        curr->appendToSendBuffer(reply);
        // flag write
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].fd == curr->getFD()) {
                fds[i].events |= POLLOUT;
                break;
            }
        }
        return;
    }

    // Extract the target nick
    const std::string targetNick = tokens[1];
    User* target = findUserByNickname(targetNick);
    if (! target) {
        // 401 ERR_NOSUCHNICK
        std::string err = ":" + servername
                        + " 401 " + curr->getNickname()
                        + " "   + targetNick
                        + " :No such nick\r\n";
        curr->appendToSendBuffer(err);
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].fd == curr->getFD()) {
                fds[i].events |= POLLOUT;
                break;
            }
        }
        return;
    }

    // 311 RPL_WHOISUSER
    std::ostringstream whoisUser;
    whoisUser << ":" << servername
              << " 311 " << curr->getNickname()
              << " "    << target->getNickname()
              << " "    << target->getUsername()
              << " "    << target->getHostname()
              << " * :" << target->getRealname()
              << "\r\n";
    curr->appendToSendBuffer(whoisUser.str());

    // 318 RPL_ENDOFWHOIS
    std::ostringstream endOfWhois;
    endOfWhois << ":" << servername
               << " 318 " << curr->getNickname()
               << " "    << target->getNickname()
               << " :End of /WHOIS list\r\n";
    curr->appendToSendBuffer(endOfWhois.str());

    // flag write once more
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == curr->getFD()) {
            fds[i].events |= POLLOUT;
            break;
        }
    }
}

void handleKick(User* requester,
                const std::string& channelName,
                const std::string& targetNick,
                const std::string& reason, std::vector<pollfd> &fds)
{
    // 1) find channel in the global map
    std::map<std::string,Chatroom*>::iterator mit
        = g_chatrooms.find(channelName);
    if (mit == g_chatrooms.end())
    {
        // 403 = ERR_NOSUCHCHANNEL
        std::string msg = ":" + servername +
            " 403 " + requester->getNickname() +
            " " + channelName +
            " :No such channel\r\n";
        requester->appendToSendBuffer(msg);
        return;
    }
    Chatroom* chan = mit->second;

    // 2) Not on channel? 442 ERR_NOTONCHANNEL
    if (!chan->isMember(requester))
    {
        std::string msg = ":" + servername +
            " 442 " + requester->getNickname() +
            " " + channelName +
            " :You're not on that channel\r\n";
        requester->appendToSendBuffer(msg);
        return;
    }

    // 3) Not an operator? 482 ERR_CHANOPRIVSNEEDED
    if (!chan->isOperator(requester))
    {
        std::string msg = ":" + servername +
            " 482 " + requester->getNickname() +
            " " + channelName +
            " :You're not channel2 operator\r\n";
        requester->appendToSendBuffer(msg);
        return;
    }

    // 4) Look up the victim
    User* victim = chan->findUserByNick(targetNick);
    if (victim == NULL)
    {
        // 441 ERR_USERNOTINCHANNEL
        std::string msg = ":" + servername +
            " 441 " + requester->getNickname() +
            " " + targetNick +
            " " + channelName +
            " :They aren't on that channel\r\n";
        requester->appendToSendBuffer(msg);
        return;
    }

    // 5) Build the KICK line
    std::string textReason = reason.empty() ? targetNick : reason;
    std::string kickLine = ":" + requester->getNickname() +
        "!" + requester->getUsername() +
        "@" + requester->getHostname() +
        " KICK " + channelName +
        " " + targetNick +
        " :" + textReason + "\r\n";

    // 6) Broadcast to everyone in the channel
    chan->broadcast(kickLine, NULL, fds);

    // 7) Send it to the kicked user
    victim->appendToSendBuffer(kickLine);
    victim->removeChatroom(channelName);
    // 8) Finally remove them
    chan->removeUser(victim);
}




void handleJoin(User* curr,
                int fd,
                const std::vector<std::string>& tokens,
                std::vector<pollfd>& fds)
{
    if (! uniqueNick(curr)) {
        return;
    }
    std::string chanName = sanitize(tokens[1]);
    if (chanName.empty() || chanName[0] != '#') {
        std::cout << "  Invalid channel name " << tokens[1] << std::endl;
        return;
    }

    Chatroom* chan = NULL;
    std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(chanName);
    if (it == g_chatrooms.end()) {
        chan = new Chatroom(chanName);
        g_chatrooms[chanName] = chan;
        chan->addOperator(curr);
        std::cout << "User: " << curr->getNickname()
                  << " is Operator of " << chan->getName() << std::endl;
    } else {
        chan = it->second;
    }

    if (chan->isInviteOnly() && ! chan->isInvited(curr)) {
        std::string err = ":" + servername
                        + " 473 "      // ERR_INVITEONLYCHAN
                        + curr->getNickname()
                        + " " + chanName
                        + " :Cannot join channel (+i)\r\n";
        curr->appendToSendBuffer(err);
        return;
    }

    if (chan->hasKey()) {
        // tokens.size() must be >= 3 to supply a key
        if (tokens.size() <= 2) {
            // No key supplied → ERR_BADCHANNELKEY
            std::string err = ":" + servername
                            + " 475 "      // ERR_BADCHANNELKEY
                            + curr->getNickname()
                            + " " + chanName
                            + " :Bad Channel Key\r\n";
            curr->appendToSendBuffer(err);
            return;
        }

        const std::string& givenKey = tokens[2];
        if (givenKey != chan->getKey()) {
            // Wrong key → ERR_BADCHANNELKEY
            std::string err = ":" + servername
                            + " 475 "
                            + curr->getNickname()
                            + " " + chanName
                            + " :Bad Channel Key\r\n";
            curr->appendToSendBuffer(err);
            return;
        }
    }
        std::cout << "LIMIT-TEST limit: " << chan->hasLimit() << "members" << std::endl;
        if (chan->hasLimit()) {
             const std::vector<User*>& members = chan->getMembers();
                if (static_cast<int>(members.size()) >= chan->getLimit()) {
                 // Channel is full → ERR_CHANNELISFULL (471)
                     std::string err = ":" + servername
                        + " 471 "
                        + curr->getNickname()
                        + " " + chanName
                        + " :Cannot join channel (+l)\r\n";
                    curr->appendToSendBuffer(err);
            return;
    }
}
    chan->addUser(curr);
    std::string prefix =
    curr->getNickname()
  + "!" + curr->getUsername()
  + "@" + curr->getHostname();

std::string joinMsg = ":" + prefix
                    + " JOIN :" + chanName
                    + "\r\n";

chan->broadcast(joinMsg, NULL, fds);
curr->appendToSendBuffer(joinMsg);
    const std::vector<User*>& members = chan->getMembers();
    std::string nameList;
    for (size_t i = 0; i < members.size(); ++i) {
        if (! nameList.empty())
            nameList += " ";

        // Prefix with @ if the user is an operator (optional)
        if (chan->isOperator(members[i]))
            nameList += "@";
        nameList += members[i]->getNickname();
    }

    std::string rpl_353 = ":" + servername
                        + " 353 "     // RPL_NAMREPLY
                        + curr->getNickname()
                        + " = " + chanName
                        + " :" + nameList
                        + "\r\n";
    std::string rpl_366 = ":" + servername
                        + " 366 "     // RPL_ENDOFNAMES
                        + curr->getNickname()
                        + " " + chanName
                        + " :End of /NAMES list\r\n";

    curr->addNewMemberToChatroom(chan);
    curr->appendToSendBuffer(rpl_353);
    curr->appendToSendBuffer(rpl_366);
    curr->HSTopicQuery(*chan, fds);

    (void) fd;  // unused
}
   //(void) fd;//in case we still need this and its fucked
void handlePrivmsg(User* curr,
                   int /*fd*/,
                   const std::vector<std::string>& tokens,
                   const std::string& raw,
                   std::vector<pollfd>& fds)
{
    // 411 ERR_NORECIPIENT: no target given
    if (tokens.size() < 2) {
        curr->appendToSendBuffer(":" + servername
            + " 411 " + curr->getNickname()
            + " PRIVMSG :No recipient given (PRIVMSG)\r\n");
        return;
    }
    std::string target = tokens[1];

    // 412 ERR_NOTEXTTOSEND: no message given
    size_t pos = raw.find(" :");
    if (pos == std::string::npos || pos + 2 >= raw.size()) {
        curr->appendToSendBuffer(":" + servername
            + " 412 " + curr->getNickname()
            + " :No text to send\r\n");
        return;
    }
    std::string text = raw.substr(pos + 2);

    // Build the standard IRC prefix: nick!user@host
    std::string prefix = curr->getNickname()
                       + "!" + curr->getUsername()
                       + "@" + curr->getHostname();  // :contentReference[oaicite:0]{index=0}

    // Channel message if target starts with '#' or '&'
    if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
        // lookup channel
        std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(target);
        Chatroom* room = (it != g_chatrooms.end() ? it->second : NULL);  // NULL instead of nullptr

        if (!room) {
            // 403 ERR_NOSUCHCHANNEL
            curr->appendToSendBuffer(":" + servername
                + " 403 " + curr->getNickname()
                + " " + target
                + " :No such channel\r\n");
        }
        else if (!room->isMember(curr)) {
            // 404 ERR_CANNOTSENDTOCHAN
            curr->appendToSendBuffer(":" + servername
                + " 404 " + curr->getNickname()
                + " " + target
                + " :Cannot send to channel\r\n");
        }
        else {
            // broadcast to channel
            std::string full = ":" + prefix
                             + " PRIVMSG " + target
                             + " :" + text + "\r\n";
            room->broadcast(full, curr, fds);  // :contentReference[oaicite:1]{index=1}
        }
    }
    else {
        // Direct user-to-user message
        User* u2 = findUserByNickname(target);
        if (!u2) {
            // 401 ERR_NOSUCHNICK
            curr->appendToSendBuffer(":" + servername
                + " 401 " + curr->getNickname()
                + " " + target
                + " :No such nick\r\n");
            return;
        }

        // format the PRIVMSG
        std::string full = ":" + prefix
                         + " PRIVMSG " + target
                         + " :" + text + "\r\n";

        // send to recipient
        u2->appendToSendBuffer(full);
        int target_fd = u2->getFD();
        if (target_fd == curr->getFD())
            return;
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == target_fd) {
            fds[i].events |= POLLOUT;
            break;
        }
    }
    }
}

void handleInvite(User* curr,
                  const std::string& rawTargetNick,
                  const std::string& rawChannelName)
{
    // 1) First sanitize whatever the user typed:
    std::string channelName = sanitize(rawChannelName);

    // 2) If sanitize() gave us an empty string or something not starting with '#',
    //    prepend '#' and sanitize again (to trim whitespace, etc.).
    if (channelName.empty() || channelName[0] != '#')
    {
        channelName = "#" + channelName;
        channelName = sanitize(channelName);
    }

    // 3) If sanitize still failed, reject with ERR_NOSUCHCHANNEL
    if (channelName.empty() || channelName[0] != '#')
    {
        std::string err = ":" + servername
                        + " 403 " + curr->getNickname()
                        + " " + rawChannelName
                        + " :No such channel\r\n";
        curr->appendToSendBuffer(err);
        return;
    }

    // 4) Now look up the channel in g_chatrooms
    std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(channelName);
    if (it == g_chatrooms.end())
    {
        std::string msg = ":" + servername
                        + " 403 " + curr->getNickname()
                        + " " + channelName
                        + " :No such channel\r\n";
        curr->appendToSendBuffer(msg);
        return;
    }
    Chatroom* chan = it->second;

    // 5) Ensure the inviter is on that channel
    if (! chan->isMember(curr))
    {
        std::string msg = ":" + servername
                        + " 442 " + curr->getNickname()
                        + " " + channelName
                        + " :You're not on that channel\r\n";
        curr->appendToSendBuffer(msg);
        return;
    }

    // 6) If +i is set, only an operator may invite
    if (chan->isInviteOnly())
    {
        if (! chan->isOperator(curr))
        {
            std::string msg = ":" + servername
                            + " 482 " + curr->getNickname()
                            + " " + channelName
                            + " :You're not channel3 operator\r\n";
            curr->appendToSendBuffer(msg);
            return;
        }
    }
    // Otherwise (invite-only off), any member may INVITE

    // 7) Check that targetNick exists
    std::string targetNick = rawTargetNick;
    User* target = findUserByNickname(targetNick);
    if (target == NULL)
    {
        std::string msg = ":" + servername
                        + " 401 " + curr->getNickname()
                        + " " + targetNick
                        + " :No such nick/channel\r\n";
        curr->appendToSendBuffer(msg);
        return;
    }

    // 8) If they’re already on that channel, send ERR_USERONCHANNEL
    if (chan->isMember(target))
    {
        std::string msg = ":" + servername
                        + " 443 " + curr->getNickname()
                        + " " + targetNick
                        + " " + channelName
                        + " :They are already on that channel\r\n";
        curr->appendToSendBuffer(msg);
        return;
    }

    // 9) (Optional: check +k here if you need to enforce a key)

    // 10) Record the invitation so they can JOIN even if +i is set
    chan->inviteUser(target);

    // 11) Send RPL_INVITING (341) back to the inviter
    {
        std::string reply = ":" + servername
                          + " 341 " + curr->getNickname()
                          + " " + targetNick
                          + " " + channelName + "\r\n";
        curr->appendToSendBuffer(reply);
    }

    // 12) Send the actual INVITE line to the invitee’s buffer
    {
        std::string inviteMsg;
        inviteMsg.reserve(
            curr->getNickname().length() +
            curr->getUsername().length() +
            curr->getHostname().length() +
            targetNick.length() +
            channelName.length() + 32
        );
        inviteMsg += ":";
        inviteMsg += curr->getNickname();
        inviteMsg += "!";
        inviteMsg += curr->getUsername();//formatting, what the fuck is this shit
        inviteMsg += "@";
        inviteMsg += curr->getHostname();
        inviteMsg += " INVITE ";
        inviteMsg += targetNick;
        inviteMsg += " :";
        inviteMsg += channelName;
        inviteMsg += "\r\n";

        target->appendToSendBuffer(inviteMsg);
    }
}


void handleTopic(User* curr, const std::string& raw, std::vector<std::string> tokens, std::vector<pollfd> &fds)
{
    (void) raw;
    Chatroom* chan = NULL;
    std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(tokens[1]);
    if (it != g_chatrooms.end())
    {
        chan = it->second;   
    }
    else
    {
        curr->appendToSendBuffer("Private messages can't have topics baka.\r\n");
        return;
    }
    //tokens 2 = topictoset
    // tokens 1 = channelname
    if (tokens[2] != "")
    {
        curr->HSSetTopic(tokens ,*chan, fds);
        return;
    }
    curr->HSTopicQuery(*chan, fds);
    
}
