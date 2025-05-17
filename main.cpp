#include "server.hpp"

int main (void)
{
	
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6667); //port of our choosing

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
	std::vector<pollfd> fds;
    pollfd server_pollfd = { server_fd, POLLIN, 0 };
    pollfd stdin_pollfd = { STDIN_FILENO, POLLIN, 0 };

    fds.push_back(stdin_pollfd);  // for server console input
    fds.push_back(server_pollfd); // for incoming client connections

    bool running = true;

    while (running)
    {
        welcomemessage();
        int activity = poll(fds.data(), fds.size(), -1);
        if (activity < 0)
        {
            continue;
        }
        serverloop(fds, running, server_fd);

    }
    cleanup(fds);

    

    return 0;
}

void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd)
{
    for (size_t i = 0; i < fds.size(); ++i)
    {
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
            }
            else if (fds[i].fd == server_fd)//new client connecting
            {
                newclient(server_fd, fds);
            }
            else
            {
                messagehandling(fds, i);
            }
        }
    }
}

void newclient(int &server_fd, std::vector<pollfd> &fds)
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd >= 0)
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";
    }
}

void messagehandling(std::vector<pollfd> &fds, size_t &i)
{
    char messagebuffer[2024];
    int n = recv(fds[i].fd, messagebuffer, sizeof(messagebuffer) - 1, 0);
    if (n <= 0)
    {
        std::cout << "Client disconnected: FD " << fds[i].fd << "\n";
        close(fds[i].fd);
        fds.erase(fds.begin() + i);
        --i;
    }
    else
    {
        //insert command parsing
        messagebuffer[n] = '\0';
        std::cout << "Client " << fds[i].fd << ": " << messagebuffer;
    }
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

void cleanup(std::vector<pollfd> &fds)
{
    for (size_t i = 0; i < fds.size(); ++i)
    {
        close(fds[i].fd);
    }
}

void welcomemessage()
{
    std::cout << "server-chan has been started UwU" << std::endl;
    std::cout << "type exit to exit UwU" << std::endl;
    std::cout << "if you wanna connect to this, the easiest way is to " << std::endl;
    std::cout << "open another cmdwindow and type: telnet localhost 6667" << std::endl;
    std::cout << "have fun and don't be a mean cookie UwU" << std::endl;
}