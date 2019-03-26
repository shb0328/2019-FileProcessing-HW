#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int cnt;
	char buffer[100] = {0,};

	int src = open(argv[1],O_RDONLY);
	int dest = open(argv[2],O_WRONLY|O_TRUNC|O_CREAT);

	while((cnt=read(src,buffer,100))!=0)
	{
		write(dest,buffer,cnt);
		if(cnt != 100) break;
	}
	close(src);
	close(dest);

	return 0;
}	
