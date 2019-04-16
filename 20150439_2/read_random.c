#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#define SUFFLE_NUM	10000	
#define RECORD_SIZE 250


void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);


int main(int argc, char **argv)
{
	int *read_order_list;
	int num_of_records;
	num_of_records = atoi(argv[2]);
	read_order_list = malloc(sizeof(int)*num_of_records); 

	GenRecordSequence(read_order_list, num_of_records); 


	int fd;
	char * src = argv[1];
	char buffer[RECORD_SIZE] = {0,};

	struct timeval startTime,endTime;
	int diffTime;

	gettimeofday(&startTime,NULL);


	if( 0< (fd = open(src,O_RDONLY))){

		for(int i =0; i<num_of_records; ++i){
			lseek(fd,read_order_list[i]*RECORD_SIZE,SEEK_SET);
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

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
