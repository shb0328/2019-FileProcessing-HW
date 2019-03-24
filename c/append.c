#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int fd = open(argv[1],O_RDWR|O_APPEND);	
	write(fd,argv[2],10);
	close(fd);
	return 0;
}
