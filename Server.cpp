#include "Server.hpp"

std::string servername = "server-chan";
std::string g_serverPassword = "";

void cleanupUser(User* u) {
    std::map<std::string,Chatroom*>::iterator it;
    for (it = g_chatrooms.begin(); it != g_chatrooms.end(); ++it) {
        Chatroom* chan = it->second;
        chan->removeUser(u);
        chan->removeOperator(u);
    }
    removeUser(u);
}

void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd)
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        User *user = findUserByFD(fds[i].fd);
        if (fds[i].revents & POLLIN) 
        {
			 // Handle input from server terminal
            if (fds[i].fd == STDIN_FILENO)
            {
                if(serverexit() == true)
                {
                    running = false;
                    break;
                }
                else
                    continue;
            }
            if (fds[i].fd == server_fd)
            {
                User::newclient(server_fd, fds);
                i++;
                continue;
            }
            char buffer[1024];
            ssize_t n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                disconnect(fds, i);
                continue;
            }
            buffer[n] = '\0';
            std::string msg(buffer);
            std::cout << "Received from " << fds[i].fd << ": " << msg;
            
            if (!user)
                continue;
            leParse(user, buffer, fds, i);
			
        }
        if (user && user->hasDataToSend())
            fds[i].events |= POLLOUT;
        if (fds[i].revents & POLLOUT)
        {
            if (!user || !user->hasDataToSend()) continue;
            if (polling(user, fds, i) == 1)
                continue;
        }
    }
	for (size_t j = 0; j < fds.size();)
    {
        int fd = fds[j].fd;
        User *u = (fd >= 0 ? findUserByFD(fd) : NULL);

        if (u && u->isDead())
        {
            // safe to delete now that we're not in the middle of parsing
            cleanupUser(u);
            fds.erase(fds.begin() + j);
        }
        else
        {
            ++j;
        }
    }
}

int polling(User *user, std::vector<pollfd> &fds, size_t &i)
{
    const std::string& msg = user->getSendBuffer();
    ssize_t sent = send(fds[i].fd, msg.c_str(), msg.size(), 0);
    if (sent > 0)
    {
        user->consumeSendBuffer(sent);
        if (!user->hasDataToSend())
        {
            fds[i].events &= ~POLLOUT;
        }
        return 0;
    }
    else if (sent == -1 && errno != EWOULDBLOCK && errno != EAGAIN)
    {
        std::cerr << "Error sending to fd " << fds[i].fd << ": " << strerror(errno) << std::endl;
        close(fds[i].fd);
        fds.erase(fds.begin() + i);
        if (user) cleanupUser(user);
        --i;
        return 1;
    }
    return 0;
}

void leParse(User *user, char *buffer, std::vector<pollfd> &fds, size_t &i)
{
    // stash the fd so we can re-find the User even if we delete it
    int fd = user->getFD();

    // append whatever just came in
    user->appendToBuffer(std::string(buffer));
    std::cout << buffer << std::endl;

    // keep parsing one full line at a time,
    // but re-lookup the User* on each iteration
    for (;;)
    {
        User *u = findUserByFD(fd);
        if (!u)               // user quit & was deleted
            break;
        if (!u->hasCompleteLine())  // no complete “\r\n” left
            break;

        std::string msg = u->extractLine();
        if (!u->isRegis())
            registrationParsing(u, msg, fds);
        else
            commandParsing(msg, fds, i);

        // loop around — but if commandParsing deleted the user,
        // the next findUserByFD(fd) will return nullptr and we’ll break.
    }
}
void disconnect(std::vector<pollfd> &fds, size_t &i)
{
    int disc_fd = fds[i].fd;
    std::cout << "Client disconnected: FD " << disc_fd << std::endl;

    // 1) Close the socket immediately
    close(disc_fd);

    // 2) Mark the User dead (will be cleaned up in the sweep phase)
    if (User* old = findUserByFD(disc_fd)) {
        old->markDead();
        // optionally detach from chatrooms now
        old->leaveAllChatrooms();
    } else {
        std::cout << "No matching User* for FD " << disc_fd << std::endl;
    }

    // 4) DON’T erase() here, and don’t call cleanupUser()—
    //    that happens in your end-of-loop reaper sweep.
}


void registrationParsing(User *user, std::string msg, std::vector<pollfd> &fds)
{
	(void) fds;
	if (user->isRegis() == true)
		return;
    std::cout << "WE NEW USER UP IN HERE" << std::endl;
    std::string nick = parseNick(msg);
    std::string host = parseHost(msg);
    std::string user_str = parseUser(msg);
if (!nick.empty()) {
    // 1) Check for duplicate
    bool duplicate = false;
    for (size_t k = 0; k < fds.size(); ++k) {
        int fd2 = fds[k].fd;
        if (fd2 < 0 || fd2 == user->getFD()) continue;
        User* other = findUserByFD(fd2);
        if (other != NULL && other->isRegis()
            && other->getNickname() == nick)
        {
            duplicate = true;
            break;
        }
    }

        if (duplicate) {
            // 2) Grab a fresh uwuTastic nick
            std::string newNick = uwuTasticNick();

            // 3) Inform them via NOTICE
            std::ostringstream notice;
            notice << ":" << servername
                   << " NOTICE " << nick
                   << " :Nickname “" << nick
                   << "” is already in use – assigning “"
                   << newNick << "”\r\n";
            user->appendToSendBuffer(notice.str());

            // 4) Tell their client to switch NICK
            std::ostringstream nickmsg;
            nickmsg << ":" << nick
                    << " NICK " << newNick
                    << "\r\n";
            user->appendToSendBuffer(nickmsg.str());

            nick = newNick;
        }

    // 5) Truncate if too long
    if (nick.size() > MAX_NICK_LEN) {
        std::string truncated = nick.substr(0, MAX_NICK_LEN);
        std::ostringstream warn;
        warn << ":" << servername
             << " NOTICE " << nick
             << " :Your nickname was truncated to " << truncated
             << "\r\n";
        user->appendToSendBuffer(warn.str());
        nick = truncated;
    }

    // 6) *Notify* the client of the new NICK
    std::string oldnick = user->getNickname();
    std::ostringstream nickmsg;
    nickmsg << ":" << oldnick
            << " NICK " << nick
            << "\r\n";
    user->appendToSendBuffer(nickmsg.str());

    // 7) Set it internally
    user->setNickname(nick);
}
    if (!host.empty())
        user->setHostname(host);
    if (!user_str.empty())
        user->setUser(user_str);
    if(user->getNickname() != "" && user->getHostname() != "" && user->getUsername() != "" )//fixed hiccups hopefully
    {
        user->setRegis(true);
        std::cout << "USER " << nick << " HAS BEEN ABSOLUTELY VERIFIED FOR SURE" << std::endl;
        user->HSwelcome();
    }
}


void welcomemessage()
{
    std::cout << "server-chan has been started UwU" << std::endl;
    std::cout << "type exit to exit and if a PASSWORD was set" << std::endl;
	std::cout << "enter it via \" /quote <PASSWORD>  \" UwU" << std::endl;
    std::cout << "have fun and don't be a mean cookie UwU" << std::endl;
    std::cout << std::endl;
    std::cout << "   ⠀⠀⠀⠀⡴⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⠀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⠚⠁⠀⠳⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠴⠋⠈⢧⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⢠⡆⠀⠀⠀⠀⢠⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⢀⡀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⢸⣧⠀⠀⠀⠀⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡄⠀⠀⠀⠀⣸⠃⠀⠀⠀⠀" << std::endl;
    std::cout << "⠒⠒⠂⠀⢿⡆⢀⣀⣴⠏⠀⠀⢾⡀⠀⣧⠀⠀⡇⠀⠈⣷⣄⠀⢀⣰⠃⠀⠈⠉⠉⠁" << std::endl;
    std::cout << "⡤⠖⠀⠀⠈⠙⠛⠋⠁⠀⠀⠀⠈⠷⠿⠙⠳⠞⠃⠀⠀⠈⠻⠷⠛⠁⠀⠀⡈⠑⠢⡄" << std::endl;
    std::cout << "⠀⢠⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣆⠀⠀" << std::endl;
    std::cout << "⠘⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠃⠀" << std::endl;
    std::cout << std::endl;
}

bool serverexit()
{
    std::string input;
    std::getline(std::cin, input);
    if (input == "exit")
    {
        std::cout << "Server shutting down.\n";
        return true;
    }
    else
        return false;
}

std::string PasswordManager::_password;

void PasswordManager::setPassword(const std::string& pw) {
    _password = pw;
}

const std::string& PasswordManager::getPassword() {
    return _password;
}
