// Parsing.cpp

#include "Parsing.hpp"    // your declarations
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


extern std::map<std::string,Chatroom*> g_chatrooms;
extern std::string servername;

void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i)
{
    std::string raw(messagebuffer);
    std::vector<std::string> tokens = split(raw, ' ');
    if (tokens.empty()) return;

    int fd = fds[i].fd;
    User *curr = findUserByFD(fd);

    const std::string &cmd = tokens[0];
    if      (cmd == "PING")     handlePing(fd);
    else if (cmd == "NICK" && tokens.size() > 1)
                                 handleNick(curr, tokens, raw);
  else if (cmd == "KICK" && tokens.size() > 2)
{
    std::string channelName = tokens[1];
    std::string targetNick  = tokens[2];

    // grab optional reason after the first " :"
    std::string reason;
    size_t pos = raw.find(" :");
    if (pos != std::string::npos)
        reason = raw.substr(pos + 2);

    handleKick(curr, channelName, targetNick, reason);
}

    else if (cmd == "JOIN" && tokens.size() > 1)
                                 handleJoin(curr, fd, tokens[1]);
    else if (cmd == "PRIVMSG")
                                 handlePrivmsg(curr, fd, tokens, raw);
    else if (cmd == "INVITE" && tokens.size() > 1)
                                 handleInvite(curr, tokens[1]);
    else if (cmd == "TOPIC" && tokens.size() > 1)
                                 handleTopic(curr, raw);
    // else if (cmd == "QUIT")
    //                              handleQuit(fd);            //whoaaaaaaaaaaaaaaawe doing double deletion with this oneeeeee.. check serverloooooop
}


void handlePing(int fd)
{
    const std::string resp = "PONG :localhost\r\n";
    send(fd, resp.c_str(), resp.length(), 0);
}

void handleNick(User* curr,
                const std::vector<std::string>& tokens,
                const std::string& raw)
{
    // suppress unused warning for tokens
    (void)tokens;
    std::string oldnick = curr->getNickname();
    std::string newnick = parseNick(raw);
    if (findUserByNickname(newnick) != NULL)
    {
        std::string err = ":localhost 433 * " + newnick + " :Nickname is already in use\r\n";
        send(findUserByNickname(oldnick)->getFD(), err.c_str(), err.length(), 0);
        return;
    }
    curr->setNickname(newnick);
    curr->HSNick(oldnick, newnick);
}



void handleKick(User* requester,
                const std::string& channelName,
                const std::string& targetNick,
                const std::string& reason)
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
        requester->sendMsg(msg);
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
        requester->sendMsg(msg);
        return;
    }

    // 3) Not an operator? 482 ERR_CHANOPRIVSNEEDED
    if (!chan->isOperator(requester))
    {
        std::string msg = ":" + servername +
            " 482 " + requester->getNickname() +
            " " + channelName +
            " :You're not channel operator\r\n";
        requester->sendMsg(msg);
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
        requester->sendMsg(msg);
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
    chan->broadcast(kickLine, NULL);

    // 7) Send it to the kicked user
    victim->sendMsg(kickLine);
    
    // 8) Finally remove them
    chan->removeUser(victim);
}




void handleJoin(User* curr, int fd, const std::string& chanArg)
{
    std::string chanName = sanitize(chanArg);
    if (chanName.empty() || chanName[0] != '#')
    {
        std::cout << "  Invalid channel name " << chanArg << std::endl;
        return;
    }

    Chatroom *chan = NULL;
    std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(chanName);
    if (it == g_chatrooms.end())
    {
        chan = new Chatroom(chanName);
        g_chatrooms[chanName] = chan;
        chan->addOperator(curr);
        std::cout << "User: " << curr->getNickname() << " is Operator of " << chan->getName() << std::endl; 
    }
    else
    {
        chan = it->second;
    }

    chan->addUser(curr);
    std::string msg = ":" + curr->getNickname() + " JOIN :" + chanName + "\r\n";
    chan->broadcast(msg, NULL);
    join_channel(fd, curr->getNickname(), chanName);
}

void handlePrivmsg(User* curr,
                   int fd,
                   const std::vector<std::string>& tokens,
                   const std::string& raw)
{
    if (tokens.size() < 3) return;

    std::string target = tokens[1];
    size_t pos = raw.find(" :");
    std::string text = (pos != std::string::npos)
                       ? raw.substr(pos + 2)
                       : "";

    if (!target.empty() && target[0] == '#')
    {
        std::map<std::string, Chatroom*>::iterator it = g_chatrooms.find(target);
        if (it != g_chatrooms.end())
        {
            Chatroom* chan = it->second;
            std::string full = ":" + curr->getNickname()
                             + " PRIVMSG " + target
                             + " :" + text + "\r\n";
            chan->broadcast(full, curr);
        }
        else
        {
            send_to_client(fd, "403 " + target + " :No such channel\r\n");
        }
    }
    else
    {
        // TODO: private user‐to‐user messaging
    }
}

void handleInvite(User* curr, const std::string& target)
{
    // suppress unused warning for curr
    (void)curr;

    std::cout << "INVITE requested for " << target << std::endl;
    // TODO: implement INVITE logic
}

void handleTopic(User* curr, const std::string& raw)
{
    // suppress unused warning for curr
    (void)curr;

    std::cout << "TOPIC command raw line: " << raw << std::endl;
    // TODO: parse and apply topic
}

void handleQuit(int fd)
{
    User* u = findUserByFD(fd);
    removeUser(u);
    std::cout << "got rid of FD " << fd << std::endl;
}
