#include <stdio.h>
#include "student.h"
#include <string.h>
#include <stdlib.h>
//
// 학생 레코드 파일에 레코드를 저장하기 전에 구조체 STUDENT에 저장되어 있는 데이터를 레코드 형태로 만든다.
// 
void pack(char *recordbuf, const STUDENT *s);


// 
// 학생 레코드 파일로부터 레코드를 읽어 온 후 구조체 STUDENT로 변환한다.
//
void unpack(const char *recordbuf, STUDENT *s);

//
// 학생 레코드 파일로부터  레코드 번호에 해당하는 레코드를 읽어서 레코드 버퍼에 저장한다.
//
void readRecord(FILE *fp, char *recordbuf, int rn);

//
//알고리즘: 저장
//학생 레코드 파일에서 삭제 레코드의 존재 여부를 검사한 후 삭제 레코드가 존재하면 이 공간에
//새로운 레코드를 저장하며, 만약 삭제 레코드가 존재하지 않거나 조건에 부합하는 삭제 레코드가
//존재하지 않으면  파일의 맨마지막에 저장한다.
//
void add(FILE *fp, const STUDENT *s);

//
//알고리즘: 검색
//학생 레코드 파일에서 sequential search 기법으로 주어진 학번 키값고 일치하는 레코드를 찾는다.
//출력은 만드시 주어진 print 함수를 사용한다.
//검색 레코드가 존재하면 레코드 번호 rn을, 그렇지 않으면 -1을 리턴한다.
//
int search(FILE *fp, const char *keyval);

//
//알고리즘: 삭제
//학생 파일에서 주어진 학번 키값과 일치하는 레코드를 찾은 후 해당 레코드를 삭제 처리한다.
//
void delete(FILE *fp, const char *keyval);

void printRecord(const STUDENT *s, int n);



/*
*
* main
*
*/



int main(int argc, char *argv[])
{
	FILE *fp;  // 학생 레코드 파일의 파일 포인터
    STUDENT* s = malloc(sizeof(STUDENT));

    if(!strcmp(argv[1],"-a")){
        
        strcpy(s->id,argv[2]);
        strcpy(s->name,argv[3]);
        strcpy(s->dept,argv[4]);
        strcpy(s->year,argv[5]);
        strcpy(s->addr,argv[6]);
        strcpy(s->phone,argv[7]);
        strcpy(s->email,argv[8]);

        add(fp,s);
        printf("-a 완료 \n");
    }
    else if(!strcmp(argv[1],"-d")){
        printf("-d\n");

        delete(fp,argv[2]);
    }
    else if(!strcmp(argv[1],"-s")){
        printf("-s\n");

        search(fp,argv[2]);
    }else {
        printf("잘못된 옵션입니다. (-a,-d,-s)\n");
    }

	return 1;
}



/*
*
*   function definition
*
*/



void add(FILE *fp, const STUDENT *s){

    FILE* indexfp;
    char* recordbuf = malloc(MAX_RECORD_SIZE);
    short int * indexbuf;
    short int num; 
    char * header = "-1"; 
printf("header : %d\n", atoi(header));

    pack(recordbuf,s);
printf("recordbuf : %s\n",  recordbuf);

    //파일 생성
    if ((indexfp = fopen(INDEX_FILE_NAME, "r+")) == NULL 
        && (fp = fopen(RECORD_FILE_NAME, "r+")) == NULL)
    {
        indexfp = fopen(INDEX_FILE_NAME, "w");
        indexbuf = (short int*)malloc(sizeof(short int)*(3));
        indexbuf[0] = 1; // #records
        indexbuf[1] = 0; // offset
        indexbuf[2] = strlen(recordbuf);
        fwrite(indexbuf ,2,3,indexfp);

        fp = fopen(RECORD_FILE_NAME, "w");

        char* writebuf = malloc(2+strlen(recordbuf));
        memcpy(writebuf,header,2);
        memcpy(writebuf+2,recordbuf,strlen(recordbuf));
printf("writebuf : %s\n", writebuf);
        fwrite(writebuf, 2+strlen(recordbuf), 1,fp);
    }
    //파일 이미 존재
    else
    {
        //index 파일-> indexbuf로 읽어오기
        fread(&num,2,1,indexfp);
        indexbuf = (short int*) malloc(sizeof(short int)*(num+3));
        indexbuf[0] = num;

        for(short int i = 1; i<=num+2; ++i){
          fseek(indexfp, (long)(i*2), SEEK_SET);
          fread(&indexbuf[i],2,1,indexfp);
        //test
          printf("indexbuf[%d] = %d\n",i, indexbuf[i]);
        }

        //data 파일.header -> header로 읽어오기
        fread(header,sizeof(header),1,fp);
printf("header : %d\n", header);
        if(atoi(header) == -1){ // 삭제된 레코드가 없을 때,
            
            fseek(fp,0L,SEEK_END);
            fwrite(recordbuf,strlen(recordbuf),1,fp);
            
            indexbuf[num+2] = indexbuf[num+1]+strlen(recordbuf);
            fseek(indexfp,0L,SEEK_END);
            fwrite(&indexbuf[num+2],2,1,indexfp);

            //record 개수 갱신
            indexbuf[0]++;
            fseek(indexfp,0L,SEEK_SET);
            fwrite(&indexbuf[0],2,1,indexfp);

        }
        else{ //삭제된 레코드가 있을 때,
            short int addr;
            addr = atoi(header);

            while(1){

                short int index;
                for(index = 1; index<=num; ++index){
                    if(indexbuf[index] == addr) break;
                }

                //size 비교
                short int size;
                size = indexbuf[index+1] - indexbuf[index];

                if(size < strlen(recordbuf)){
                    fseek(fp,(long)addr+2+1,SEEK_SET);
                    fread(&addr,2,1,fp);
                    
                    if(addr == -1) { //list 마지막까지 맞는 size가 없었다! 
                        fseek(fp,0L,SEEK_END);
                        fwrite(recordbuf,strlen(recordbuf),1,fp);
                        
                        indexbuf[num+2] = indexbuf[num+1]+strlen(recordbuf);
                        fseek(indexfp,0L,SEEK_END);
                        fwrite(&indexbuf[num+2],2,1,indexfp);

                        //record 개수 갱신
                        indexbuf[0]++;
                        fseek(indexfp,0L,SEEK_SET);
                        fwrite(&indexbuf[0],2,1,indexfp);
                        break;
                    }

                }else{
                    fseek(fp,(long)addr+2,SEEK_SET);
                    fwrite(recordbuf,strlen(recordbuf),1,fp);
                    break;
                }

            }
        }
    }

    if(indexbuf != NULL)    free(indexbuf);
    if(fp != NULL)    fclose(fp);
    if(indexfp != NULL)    fclose(indexfp);
}

int search(FILE *fp, const char *keyval){

    FILE * indexfp;
    STUDENT * s = malloc(sizeof(STUDENT));
    char* recordbuf= malloc(MAX_RECORD_SIZE);
    short int * indexbuf;
    short int num;
    char * header = "-1"; 


    indexfp = fopen(INDEX_FILE_NAME,"r+");
    fp = fopen(RECORD_FILE_NAME,"r+");

    //index 파일 읽어오기
    fread(&num,2,1,indexfp);
    indexbuf = (short int*) malloc(sizeof(short int)*(num+1));
    indexbuf[0] = num;

    for(short int i = 1; i<=num; ++i){
      fseek(indexfp, (long)(i*2), SEEK_SET);
      fread(&indexbuf[i],2,1,indexfp);
    }

    //data 파일.header -> header로 읽어오기
    fread(header,sizeof(header),1,fp);
    //record 파일에서 keyval 검사
    short int index;
    for(index = 1;index<=num; ++index){
printf("for\n");
        fseek(fp,(long)(indexbuf[index]+2),SEEK_SET);
        fread(recordbuf,strlen(recordbuf),1,fp);

printf("recordbuf : %s\n", recordbuf);

        //삭제 레코드인지 확인
        char deleteMark;
        memcpy(&deleteMark,recordbuf,1);
printf("%s\n", deleteMark);

        if(strcmp(&deleteMark,"*")) {
            unpack(recordbuf,s);
            if(strcmp(s->id,keyval) == 0) //찾았다!
                break;
        }
    }

    if(index > num)
        printf("search fail\n");
    else
        printRecord(s,1);
    
    if(s!= NULL) free(s);
    if(indexbuf != NULL)    free(indexbuf);
    if(fp != NULL)    fclose(fp);
    if(indexfp != NULL)    fclose(indexfp);
}

void delete(FILE *fp, const char *keyval){

    FILE * indexfp;
    char* recordbuf= malloc(MAX_RECORD_SIZE);
    short int * indexbuf;
    short int num;
    char * header = "-1"; 


    indexfp = fopen(INDEX_FILE_NAME,"r+");
    fp = fopen(RECORD_FILE_NAME,"r+");


    //index 파일 읽어오기
    fread(&num,2,1,indexfp);
    indexbuf = (short int*) malloc(sizeof(short int)*(num+1));
    indexbuf[0] = num;

    for(short int i = 1; i<=num; ++i){
      fseek(indexfp, (long)(i*2), SEEK_SET);
      fread(&indexbuf[i],2,1,indexfp);
    }

    //data 파일.header -> header로 읽어오기
    fread(header,sizeof(header),1,fp);


    //record 파일에서 keyval 검사
    short int index;
    for(index = 1;index<=num; ++index){
        fseek(fp,(long)(indexbuf[index]+2),SEEK_SET);
        fread(recordbuf,strlen(recordbuf),1,fp);

        STUDENT * s = malloc(sizeof(STUDENT));
        unpack(recordbuf,s);
        if(strcmp(s->id,keyval) == 0) //찾았다!
            break;
        if(s!= NULL) free(s);
    }

    short int tmp = atoi(header);
    // header = indexbuf[index];
    sprintf(header,"%d",indexbuf[index]);
    char* delete = "*";
    memcpy(delete+1,&tmp,2);
    fseek(fp,(long)(indexbuf[index]+2),SEEK_SET);
    fwrite(delete,3,1,fp);
    fseek(fp,0L,SEEK_SET);
    fwrite(header,2,1,fp);

    if(indexbuf != NULL)    free(indexbuf);
    if(fp != NULL)    fclose(fp);
    if(indexfp != NULL)    fclose(indexfp);
}

void pack(char *recordbuf, const STUDENT *s){

    strcpy(recordbuf,s->id);
    strcat(recordbuf,"|");


    strcat(recordbuf,s->name);
    strcat(recordbuf,"|");

    strcat(recordbuf,s->addr);
    strcat(recordbuf,"|");

    strcat(recordbuf,s->year);
    strcat(recordbuf,"|");

    strcat(recordbuf,s->dept);
    strcat(recordbuf,"|");

    strcat(recordbuf,s->phone);
    strcat(recordbuf,"|");

    strcat(recordbuf,s->email);
    strcat(recordbuf,"|\0");
}

void unpack(const char *recordbuf, STUDENT *s){
    int i =0;
    int j = 0;
    int k;
    while(j < 7){

        switch (j) {
            case 0 :
            s->id[i] = recordbuf[i]; break;
            case 1 :
            s->name[i-k] = recordbuf[i]; break;
            case 2 :
            s->addr[i-k] = recordbuf[i]; break;
            case 3 :
            s->year[i-k] = recordbuf[i]; break;
            case 4 :
            s->dept[i-k] = recordbuf[i]; break;
            case 5 :
            s->phone[i-k] = recordbuf[i]; break;
            case 6 :
            s->email[i-k] = recordbuf[i]; break;
        }

        if(recordbuf[i+1] == '|'){
            ++i;
            ++j;
            k=i;
        }

        ++i;
    }
}

void printRecord(const STUDENT *s, int n)
{
    int i;

    for(i=0; i<n; i++)
    {
        printf("%s|%s|%s|%s|%s|%s|%s\n", 
            s[i].id, s[i].name, s[i].dept, s[i].year, s[i].addr, s[i].phone, s[i].email);
    }
}

void readRecord(FILE *fp, char *recordbuf, int rn){
    //?????????????
}