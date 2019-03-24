#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int fd;
	fd = open(argv[1],O_RDWR|O_NONBLOCK);

	long offset;
	offset = atol(argv[2]);

	char* data;
	data = argv[3];

	char buffer[200];
	char c;
	int i=0;

	lseek(fd,offset,SEEK_SET);
	while(read(fd,&c,1) != 0)
		buffer[i++]=c;
	printf("%s\n",buffer);

	lseek(fd,offset,SEEK_SET);
	write(fd,data,10);

	lseek(fd,offset+10l,SEEK_SET);
	write(fd,buffer,strlen(buffer));
	close(fd);
	return 0;
}
