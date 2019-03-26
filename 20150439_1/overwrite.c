#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int fd = open(argv[1],O_WRONLY); /* file descriptor */
	long offset = atol(argv[2]);
	char *data = argv[3];

	lseek(fd,offset,SEEK_SET);

	write(fd,data,10);

	close(fd); 
	return 0;
}
