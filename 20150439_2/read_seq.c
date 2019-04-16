#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define RECORD_SIZE 250

int main(int argc, char **argv)
{
	int fd;
	char buffer[RECORD_SIZE]= {0,};

	char * src = argv[1];
	int recordNum = atoi(argv[2]);

	struct timeval startTime, endTime;
	int diffTime;

	gettimeofday(&startTime,NULL);

	if(0<(fd = open(src,O_RDONLY))){
		for(int i =0; i<recordNum; ++i){
			read(fd,buffer,RECORD_SIZE);
		}
		
		close(fd);
	}else{
		printf("open error\n");
	}

	gettimeofday(&endTime,NULL);
	diffTime = (endTime.tv_sec - startTime.tv_sec)*1000000 + (endTime.tv_usec - startTime.tv_usec);

	printf("%d us\n",diffTime);
	
	return 0;
}
