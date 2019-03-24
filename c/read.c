#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	char buffer[10] = {0,};

	int fd = open(argv[1],O_RDONLY); 
	long offset = atol(argv[2]);

	lseek(fd,offset,SEEK_SET); 

	int i=0;char c;
	while(read(fd,&c,1) != 0)
	{
		printf("%c",c);
		if(++i>9) break;
	}

	close(fd);
	return 0;
}
