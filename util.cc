#include <sstream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util.h"
using namespace std;

string Util::sockNtop(struct sockaddr *addr)
{
	char buf[128];
	stringstream stream;

	memset(buf, 0, sizeof(buf));
	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)addr;
		if (inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)) == NULL)
			return string();
		stream << string(buf, strlen(buf));
		if (ntohs(sin->sin_port) != 0)
			stream << ":" << ntohs(sin->sin_port);
	}
	else if (addr->sa_family == AF_INET6)
	{
	}

	return stream.str();
}

void * Util::getInAddr(struct sockaddr *addr)
{
	if (addr->sa_family == AF_INET)
		return &(((struct sockaddr_in *)addr)->sin_addr);
	if (addr->sa_family == AF_INET6)
		return &(((struct sockaddr_in6 *)addr)->sin6_addr);

	return NULL;
}

