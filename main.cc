#include "chatsvr.h"

int main(int argc, char *argv[])
{
	ChatServer svr("9877");
	svr.start();

	return 0;
}
