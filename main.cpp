#include "Server.hpp"
#include <csignal>

extern std::map<std::string, Chatroom*> g_chatrooms;
extern std::vector<User*>        g_mappa;

static std::vector<pollfd>* g_fds_ptr    = NULL;
static int                  g_server_fd = -1;


void cleanup(std::vector<pollfd> &fds)
{
    for (size_t i = 0; i < fds.size(); ++i) {
        close(fds[i].fd);
    }
}


void cleanupUsers()
{
    for (std::vector<User*>::iterator uit = g_mappa.begin();
         uit != g_mappa.end(); ++uit)
    {
        delete *uit;
    }
    g_mappa.clear();
}

void cleanupChatrooms()
{
    for (std::map<std::string, Chatroom*>::iterator cit = g_chatrooms.begin();
         cit != g_chatrooms.end(); ++cit)
    {
        delete cit->second;
    }
    g_chatrooms.clear();
}

static void signalHandler(int /*signum*/) {
    if (g_fds_ptr) {
        // close all fds…
        cleanup(*g_fds_ptr);
        // free the vector’s internal buffer (C++98-compatible)
        std::vector<pollfd> empty;
        empty.swap(*g_fds_ptr);
    }
    cleanupUsers();
    cleanupChatrooms();
    if (g_server_fd != -1) {
        close(g_server_fd);
    }
    std::exit(0);
}







int main(int argc, char** argv)
{
    if (!(argc <= 3 && argc >=2)) {
        std::cout << "invalid number of arguments.\n"
                  << "Usage: /ircserv <port> <password>  pw is optional\n";
        exit(1);
    }
    // Validate port & set up server socket
    validatePort(argv[1]);
    // record the password that clients must PASS
	if (argc == 3)
        g_serverPassword = argv[2];
	else
        g_serverPassword = "";
    std::cout << "g_pass is: " << g_serverPassword << std::endl;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	g_server_fd = server_fd;
	struct sigaction ignore_sa = {};
    ignore_sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignore_sa, NULL);

    // install our cleanup handler for INT/TERM
    struct sigaction sa = {};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(std::atoi(argv[1]));

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    // Set up polling on STDIN + server socket
    std::vector<pollfd> fds;
    pollfd stdin_pollfd  = { STDIN_FILENO, POLLIN, 0 };
    pollfd server_pollfd = { server_fd,    POLLIN, 0 };
    fds.push_back(stdin_pollfd);
    fds.push_back(server_pollfd);
    // std::cout << "[DEBUG-main] &fds=" << &fds
    //           << ", first_fd=" << (fds.empty() ? -1 : fds[0].fd)
    //           << ", size=" << fds.size() << "\n";
    bool running = true;
    welcomemessage();

	g_fds_ptr = &fds;

    // Main event loop
    while (running) {
        int activity = poll(fds.data(), fds.size(), -1);
        if (activity < 0)
            continue;
        serverloop(fds, running, server_fd);
    }

    // Shutdown: close sockets, free Users, free Chatrooms
    cleanup(fds);
    cleanupUsers();
    cleanupChatrooms();

    return 0;
}




void debugPrintPolloutSendBuffers(const std::vector<pollfd>& fds, const std::vector<User*>& users) {
    std::cout << "[DEBUG] --- POLLOUT Send Buffers ---" << std::endl;
    for (size_t i = 0; i < fds.size(); ++i) {
        if (!(fds[i].revents & POLLOUT))
            continue;

        int fd = fds[i].fd;
        User* user = NULL;

        // Find user matching this fd
        for (size_t j = 0; j < users.size(); ++j) {
            if (users[j] && users[j]->getFD() == fd) {
                user = users[j];
                break;
            }
        }

        if (!user) {
            std::cout << "FD: " << fd << " → [No matching user found]" << std::endl;
            continue;
        }

        const std::string& buffer = user->getSendBuffer();
        std::cout << "FD: " << fd
                  << ", Nick: " << user->getNickname()
                  << ", SendBuffer Size: " << buffer.size()
                  << ", Data: \"" << buffer.substr(0, 80) << "\"";

        if (buffer.size() > 80)
            std::cout << " [...]";

        std::cout << std::endl;
    }
    std::cout << "[DEBUG] --- End POLLOUT Send Buffers ---" << std::endl;
}
