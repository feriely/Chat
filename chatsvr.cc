#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <sstream>
#include "util.h"
#include "chatsvr.h"
using namespace std;

#define MAXLINE 1024


ChatServer::ChatServer(const string& port) : port_(port)
{
}

void ChatServer::start()
{
	int ret;
	int listenfd, connfd;
	struct addrinfo hints, *res, *p;
	struct sockaddr_storage cliaddr;
	socklen_t clilen;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((ret = getaddrinfo(NULL, port_.c_str(), &hints, &res)) != 0)
	{
		cerr << "getaddrinfo error:" << gai_strerror(ret) << endl;
		return;
	}

	for (p = res; p != NULL; p = p->ai_next)
	{
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			perror("socket");
			continue;
		}

		int yes = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		{
			perror("setsockopt");
			return;
		}

		if (bind(listenfd, p->ai_addr, p->ai_addrlen) < 0)
		{
			perror("bind");
			close(listenfd);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		cerr << "Failed to bind port " << port_ << endl;
		return;
	}

	if (listen(listenfd, 5) < 0)
	{
		perror("listen");
		return;
	}

	freeaddrinfo(res);

	for (;;)
	{
		clilen = sizeof(cliaddr);
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
		{
			perror("accept");
			return;
		}

		string peer = Util::sockNtop((struct sockaddr *)&cliaddr);
		UserInfo info;
		info.fd = connfd;
		info.name = "anonymous";
		clients_.insert(make_pair(peer, info));
	
		doChat(connfd, peer);
	}
}

void ChatServer::doChat(int fd, const string& peer)
{
    int nbytes;
    char buf[MAXLINE + 1];
    string banner = "Welcome to chat server\n";
    
    if (write(fd, banner.data(), banner.size()) != (int)banner.size())
    {
        perror("write error");
        return;
    }

    for (;;)
    {
        if ((nbytes = read(fd, buf, MAXLINE)) < 0)
        {
            perror("read error");
            return;
        }

        if (nbytes == 0)
        {
            cout << "Client close connection[" << fd << "]" << endl;
			clients_.erase(peer);
			users_.erase(peer);
			close(fd);
            return;
        }

        // Remove \r\n
        if (nbytes > 0 && buf[nbytes - 1] == '\n')
            --nbytes;
        if (nbytes > 0 && buf[nbytes - 1] == '\r')
            --nbytes;

		string msg;
        HANDLE_RESULT result = handleCmd(string(buf, nbytes), peer, msg);
        if (result == INVALID_COMMAND)
			msg = "Invalid command\n";
		else if (result == INVALID_ARGUMENT)
			msg = "Invalid argument\n";
		
		write(fd, msg.data(), msg.size());
    }
}

ChatServer::HANDLE_RESULT ChatServer::handleCmd(const string& cmd, const string& peer, string& msg)
{
    cout << "Handle command [" << cmd << "]" << endl;

    string action, rest;
    string::size_type pos = cmd.find(' ');
    if (pos == string::npos)
    {
        action = cmd;
        rest = "";
    }
    else
    {
        action = cmd.substr(0, pos);
        rest = cmd.substr(pos + 1);
    }

    if (action == "listuser")
        return handleListUser(rest, msg);
    if (action == "login")
        return handleLogin(rest, peer, msg);
    if (action[0] == '@')
        return handleAt(action.substr(1), rest, peer, msg);

    return INVALID_COMMAND;
}

ChatServer::HANDLE_RESULT ChatServer::handleListUser(const string& args, string& result)
{
    if (!args.empty())
        return INVALID_ARGUMENT;

    typeClientInfo::const_iterator it(clients_.begin()), itEnd(clients_.end());
    for (; it != itEnd; ++it)
    {
		result += it->first + "[" + it->second.name + "]\n";
    }

	return HANDLE_SUCCESS;
}

ChatServer::HANDLE_RESULT ChatServer::handleLogin(const string& args, const string& peer, string& result)
{
	string::size_type pos = args.find(' ');
	if (pos == string::npos)
		return INVALID_ARGUMENT;

	string user = args.substr(0, pos);
	string password = args.substr(pos + 1);
	clients_[peer].name = user;
	users_[user] = clients_[peer].fd;

	return HANDLE_SUCCESS;
}

ChatServer::HANDLE_RESULT ChatServer::handleAt(const string& to, const string& args, const string& peer, string& result)
{
	string msg = args + "\n";

	if (to == "all")
	{
		typeUser2Socket::const_iterator it(users_.begin()), itEnd(users_.end());
		for (; it != itEnd; ++it)
		{
			if (write(it->second, msg.data(), msg.size()) != static_cast<int>(msg.size()))
				cerr << "Write msg to " << it->first << "[" << it->second << "] failed" << endl;
		}
		return HANDLE_SUCCESS;
	}
	typeUser2Socket::iterator itFind = users_.find(to);
	if (itFind == users_.end())
	{
		result = "User not found\n";
		return HANDLE_FAIL;
	}
	if (write(itFind->second, msg.data(), msg.size()) != static_cast<int>(msg.size()))
		cerr << "Write msg to " << itFind->first << "[" << itFind->second << "] failed" << endl;


	return HANDLE_SUCCESS;
}

