#include <string>
#include <map>
#include <sys/socket.h>

class ChatServer
{
public:
    enum HANDLE_RESULT
    {
        INVALID_COMMAND,
        INVALID_ARGUMENT,
        HANDLE_SUCCESS,
        HANDLE_FAIL,
    };
    
    struct UserInfo 
    {
        int fd;
		std::string name;
    };
    typedef std::map<std::string, UserInfo> typeClientInfo;
	typedef std::map<std::string, int> typeUser2Socket;

	ChatServer(const std::string& port);

	void start();

private:
	void doChat(int fd, const std::string& peer);
    HANDLE_RESULT handleCmd(const std::string& cmd, const std::string& peer, std::string& msg);
    HANDLE_RESULT handleListUser(const std::string& args, std::string& msg);
    HANDLE_RESULT handleLogin(const std::string& args, const std::string& peer, std::string& msg);
    HANDLE_RESULT handleAt(const std::string& user, const std::string& args, const std::string& peer, std::string& msg);

	std::string port_;
    typeClientInfo clients_;
	typeUser2Socket users_;
};
