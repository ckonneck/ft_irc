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
    user->appendToBuffer(std::string(buffer));
    std::cout << buffer << std::endl;
    while (user->hasCompleteLine())
    {
        std::string msg = user->extractLine();
        if (!user->isRegis())
        {
            registrationParsing(user, msg);
        }
		if (!(g_serverPassword == "") && user->isPassValid() == true)
        	commandParsing(msg, fds, i);
		else
			commandParsing(msg, fds, i);
        continue;
    }

}

void disconnect(std::vector<pollfd> &fds, size_t &i)
{
    int disc_fd = fds[i].fd;
    std::cout << "Client disconnected: FD " << disc_fd << std::endl;
    close(disc_fd);
    
    User* old = findUserByFD(disc_fd);

    // remove this fd from the poll list
    fds.erase(fds.begin() + i);
    if (old)
    {
        old->leaveAllChatrooms();
        cleanupUser(old);
        std::cout << "Cleaned up User* for FD " << disc_fd << std::endl;
    }else {
        std::cout << "No matching User* for FD " << disc_fd << std::endl;//check logs if this ever happens
        }
    --i;
}


void registrationParsing(User *user, std::string msg)
{
	if (user->isRegis() == true)
		return;
    std::cout << "WE NEW USER UP IN HERE" << std::endl;
    std::string nick = parseNick(msg);
    std::string host = parseHost(msg);
    std::string user_str = parseUser(msg);
    if (!nick.empty()) {
    // Truncate if too long
    if (nick.size() > MAX_NICK_LEN) {
        std::string truncated = nick.substr(0, MAX_NICK_LEN);
        std::ostringstream notice;
        notice << ":" << servername
               << " NOTICE " << nick
               << " :Your nickname has been truncated to " << truncated
               << "\r\n";
        user->appendToSendBuffer(notice.str());
        nick = truncated;
    }
    user->setNickname(nick);
}
    if (!host.empty())
        user->setHostname(host);
    if (!user_str.empty())
        user->setUser(user_str);
    if(user->getNickname() != "" && user->getHostname() != "" && user->getUsername() != "")//fixed hiccups hopefully
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
