#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <vector>
#include <poll.h>

class Chatroom; // forward declaration

class User {
public:
    User(const std::string& nickname, const std::string& password);
    ~User();

    // Registration state
    bool isRegistered() const;
    void setRegistered(bool reg);

    // Socket FD
    int  getFD() const;
    void setFD(int fd);

    // Metadata
    const std::string& getNickname() const;
    const std::string& getPassword() const;
    const std::string& getRealName() const;
    void setRealName(const std::string& realName);

    // Operator status
    bool isOp() const;
    void setOp(bool op);
    void Mode(char &modeChar);

    // Accept new connections
    static void newclient(int server_fd, std::vector<pollfd>& fds);

    // Numeric replies & command handlers
    void HSwelcome();
    void HSNick(const std::string& newnick);
    void HSKick(const std::string& target);
    void HSInvite(const std::string& whom);
    void HSTopicQuery(Chatroom& room);

    // Lookup by FD or nickname
    static User* findUserByFD(int fd);
    static User* findUserByNickname(const std::string& nick);

    // Temporary registration data
    std::string tempPass;
    std::string tempNick;
    std::string tempUser;
    std::string tempRealname;
    bool        hasPass;
    bool        hasNick;
    bool        hasUser;

private:
    int         _FD;
    std::string _nickname;
    std::string _password;
    std::string _realName;
    bool        _isOp;
    bool        _registered;
};

extern std::vector<User*> g_mappa;

#endif