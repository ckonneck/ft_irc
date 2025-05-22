#include "Server.hpp"

void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i)
{
    std::string mBuf(messagebuffer);
    std::cout << "the command is " << mBuf << std::endl;
    User *curr = findUserByFD(fds[i].fd);
    std::cout << "this is tha nicknamuin commando: "<< curr->getNickname() << std::endl;
    std::vector<std::string> mVec = split(mBuf, ' ');
    if (mBuf.find("PING") == 0)
    {
        std::string response = "PONG :localhost\r\n";
        send(fds[i].fd, response.c_str(), response.length(), 0);
    }
    if (mBuf.find("NICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /NICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        std::string oldnick = curr->getNickname();
        std::string newnick = parseNick(mBuf);
        curr->setNickname(newnick);
        curr->HSNick(oldnick, newnick);
        std::cout << "this is tha nicknamuin commando2222: "<< curr->getNickname() << std::endl;
    }
    if (mBuf.find("KICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /KICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("JOIN") == 0 && mVec.size() > 1)
    {
        std::cout << "found /JOIN on position 0" << std::endl;
        
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        std::string chanName = sanitize(mVec[1]);
        if (chanName[0] != '#')
        {
            std::cout <<"  Invalid channel name " << std::endl;
            return;
        }
        Chatroom* chan = NULL;
        if(g_chatrooms.find(chanName) == g_chatrooms.end())
        {
            chan = new Chatroom(chanName);
            // std::cout << "debug lalala"<< chanName << std::endl;
            //         for (size_t j = 0; j < chanName.size(); ++j) {
            //             std::cout << "target[" << j << "] = '" << chanName[j] << "' (0x" 
            //             << std::hex << (int)(unsigned char)chanName[j] << ")" << std::endl;
            // }
            g_chatrooms[chanName] = chan;
        }
        else
        {
            std::cout << "debug lalala222: " << chanName << std::endl;
            chan = g_chatrooms[chanName];
        }

        chan->addUser(curr);
        std::string msg = ":" + curr->getNickname() + " JOIN :" + chanName + "\r\n";
        chan->broadcast(msg, NULL); // NULL = broadcast to all
        join_channel(fds[i].fd, curr->getNickname(), chan->getName());
    }
    if (mBuf.find("PRIVMSG") == 0)
    {
        std::cout << "WE IN HERE" << std::endl;
        if (mVec.size() < 3) return;
        std::string target = mVec[1];

         std::cout << "WE IN HERE2" << std::endl;
        size_t msg_start = mBuf.find(" :", 0);
        std::string actual_message = (msg_start != std::string::npos)
        ? mBuf.substr(msg_start + 2) : "";
        std::cout << "target is: " << target << std::endl;
        std::cout << "target 0 is: " << target[0] << std::endl;
        for (size_t j = 0; j < target.size(); ++j) {
    std::cout << "target[" << j << "] = '" << target[j] << "' (0x" 
              << std::hex << (int)(unsigned char)target[j] << ")" << std::endl;
}

        if (target[0] == '#')
        {
            std::cout << "WE IN HERE2.5" << std::endl;
            for (std::map<std::string, Chatroom*>::iterator it = g_chatrooms.begin(); it != g_chatrooms.end(); ++it)
            {
                std::cout << "Channel nameIT: " << it->first << std::endl;
            }

            // if (g_chatrooms.find(target) == g_chatrooms.end())
            // {
            //     std::cout << "WE IN HERE2.7" << std::endl;

            //     send_to_client(fds[i].fd, "403 " + target + " :No such channel\r\n");
            //     return;
            // }
            std::cout << "WE IN HERE3" << std::endl;
            Chatroom* chan = g_chatrooms[target];
            std::string fullMsg = ":" + curr->getNickname() + " PRIVMSG " + target + " :" + actual_message + "\r\n";
            chan->broadcast(fullMsg, curr);
    } else {
        // Private message to a user
    }
    }
    if (mBuf.find("INVITE") == 0 && mVec.size() > 1)
    {
        std::cout << "found /INVITE on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("TOPIC") == 0 && mVec.size() > 1)
    {
        std::cout << "found /TOPIC on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        //to pass full topic, should use the full vector minus the first word
        //which will be /TOPIC
    }
    if (mBuf.find("QUIT") == 0 && mVec.size() >= 1)
    {
        removeUser(findUserByFD(fds[i].fd));
        std::cout << "got rid of " <<  fds[i].fd << std::endl;
    }
    
}