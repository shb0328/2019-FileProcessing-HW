#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int cnt;
	
	int src = open(argv[1],O_RDONLY);
	int dest = open(argv[2],O_WRONLY|O_TRUNC|O_CREAT);
	
	while(read(src,&cnt,1)!=0)
		write(dest,&cnt,1);

	close(src);
	close(dest);	
	return 0;
}	
