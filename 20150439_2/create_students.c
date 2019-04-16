#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "record.h"

#define RECORD_SIZE 250

int main(int argc, char **argv)
{
	int size = atoi(argv[1]);
	int fd;

	char record[RECORD_SIZE];


	Student student_buf = {	"20150439",
							"hyebeen",
							"Seoul Sando 1",
							"SSU",
							"software",
							"Hello World!"};


	memcpy(record,student_buf.id,10);
	memcpy(record+10,student_buf.name,30);
	memcpy(record+10+30,student_buf.address,70);
	memcpy(record+10+30+70,student_buf.univ,30);
	memcpy(record+10+30+70+30,student_buf.dept,40);
	memcpy(record+10+30+70+30+40,student_buf.others,70);


	if(0<(fd = open("students.dat",O_RDWR|O_CREAT,0644))){
		
		for(int i =0; i<size; ++i){
			write(fd,record,RECORD_SIZE);
		}

		close(fd);

	}else{
		printf("open error\n");
	}
    
	return 0;
}
