#include <string>
#include <sys/types.h>
#include <sys/socket.h>

class Util
{
public:
	static void *getInAddr(struct sockaddr *addr);
	static std::string sockNtop(struct sockaddr *addr);
};
