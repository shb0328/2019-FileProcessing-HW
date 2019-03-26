#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	char buffer[11] = {0,};

	int fd = open(argv[1],O_RDONLY); 
	long offset = atol(argv[2]);

	lseek(fd,offset,SEEK_SET); 

	int cnt = read(fd,buffer,10);
	printf("%s",buffer);

	close(fd);
	return 0;
}
