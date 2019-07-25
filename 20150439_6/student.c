#include <stdio.h>
#include "student.h"
#include <stdlib.h>
#include <string.h>

//
// 학생 레코드 파일로부터  레코드 번호에 해당하는 레코드를 읽어 레코드 버퍼에 저장한다.
//
void readStudentRec(FILE *fp, char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*STUDENT_RECORD_SIZE),SEEK_SET);
    fread(recordbuf,STUDENT_RECORD_SIZE,1,fp);
}

//
// Hash file로부터 rn의 레코드 번호에 해당하는 레코드를 읽어 레코드 버퍼에 저장한다.
//
void readHashRec(FILE *fp, char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*HASH_RECORD_SIZE+4),SEEK_SET);
    fread(recordbuf,HASH_RECORD_SIZE,1,fp);
}

//
// Hash file에 rn의 레코드 번호에 해당하는 위치에 레코드 버퍼의 레코드를 저장한다.
//
void writeHashRec(FILE *fp, const char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*HASH_RECORD_SIZE+4),SEEK_SET);
    fwrite(recordbuf,HASH_RECORD_SIZE,1,fp);
}

//
// n의 크기를 갖는 hash file에서 주어진 학번 키값을 hashing하여 주소값(레코드 번호)를 리턴한다.
//
int hashFunction(const char *sid, int n)
{
    int rn;
    rn = (sid[(strlen(sid)-1)-1]*sid[strlen(sid)-1])%n;
    return rn;
}

//
// n의 크기를 갖는 hash file을 생성한다.
// Hash file은 fixed length record 방식으로 저장되며, 레코드의 크기는 14 바이트이다.
// (student.h 참조)
//
void makeHashfile(int n)
{
	// Hash file을 생성할 때 이 파일의 맨앞부분에 4바이트 헤더를 둔다. 
	// 여기에는 hash file의 크기 n을 저장한다. 이것은 search()와 (필요하면) delete()를 	
    // 위한 것이다.

    FILE * hashfp = fopen(HASH_FILE_NAME,"w+");
    FILE * recordfp = fopen(RECORD_FILE_NAME,"r");

    char recordbuf[STUDENT_RECORD_SIZE];
    char * sid = malloc(SID_FIELD_SIZE);
    
    char * tmp = malloc(n*HASH_RECORD_SIZE+sizeof(n));
    memcpy(tmp,&n,sizeof(n));
    fseek(hashfp,0L,SEEK_SET);
    fwrite(tmp,n*HASH_RECORD_SIZE+sizeof(n),1,hashfp);
    printf("n : %d\n", n);
    char null = tmp[4];

    fclose(hashfp);
    hashfp = fopen(HASH_FILE_NAME,"r+");

    fseek(recordfp,0,SEEK_END);
    int filesize = ftell(recordfp);




    for(int i =0; i< (filesize/STUDENT_RECORD_SIZE); ++i){
        char hashbuf[HASH_RECORD_SIZE];

        readStudentRec(recordfp,recordbuf,i);
        memcpy(sid,recordbuf,SID_FIELD_SIZE);

        int hn = hashFunction(sid,n);
        int hn_origin = hn;
        
    printf("[for] %d\n*sid: %s\n",i,sid);


        do{
            readHashRec(hashfp,hashbuf,hn);
    printf("*hn : %d\n",hn);
            if(hashbuf[0] == null ) 
            {
                memcpy(hashbuf,sid,SID_FIELD_SIZE);
                memcpy(hashbuf+SID_FIELD_SIZE,&i,sizeof(int));
                writeHashRec(hashfp,hashbuf,hn);
                break;
            }
                ++hn;
                hn %= n;
            
        }while(hn != hn_origin);
    printf("\n\n");
    }

    if(sid != NULL) free(sid);

    fclose(hashfp);
    fclose(recordfp);
}

//
// 주어진 학번 키값을 hash file에서 검색한다.
// 그 결과는 주어진 학번 키값이 존재하는 hash file에서의 주소(레코드 번호)와 search length이다.
// 검색한 hash file에서의 주소는 rn에 저장하며, 이때 hash file에 주어진 학번 키값이
// 존재하지 않으면 rn에 -1을 저장한다. (search()는 delete()에서도 활용할 수 있음)
// search length는 함수의 리턴값이며, 검색 결과에 상관없이 search length는 항상 계산되어야 한다.
//

int search(const char *sid, int *rn)
{
    int hn,n;
    FILE * fp = fopen(HASH_FILE_NAME,"r+");
    fseek(fp,0L,SEEK_SET);
    fread(&n,4,1,fp);

    char recordbuf[HASH_RECORD_SIZE];

    hn = hashFunction(sid, n);

    char *candidateSid = malloc(SID_FIELD_SIZE);
    int i;
    for( i=0; i<n ;i++){
        int hni = (hn+i)%n;
        readHashRec(fp,recordbuf,hni);
        memcpy(candidateSid,recordbuf,SID_FIELD_SIZE);
        if(strcmp(candidateSid,sid) == 0) //찾았다!
        {
            *rn = hni;
            if(candidateSid != NULL) free(candidateSid);
            break;
        }
    }

    fclose(fp);
    if( i == n ) {
        *rn = -1;
        return i;
    }
    return i+1;
}

//
// Hash file에서 주어진 학번 키값과 일치하는 레코드를 찾은 후 해당 레코드를 삭제 처리한다.
// 이때 학생 레코드 파일에서 레코드 삭제는 필요하지 않다.
//
void delete(const char *sid)
{
    int rn;
    search(sid,&rn);
    FILE* fp = fopen(HASH_FILE_NAME,"r+");
    fseek(fp,(long)(rn*HASH_RECORD_SIZE+4),SEEK_SET);
    fwrite("*",1,1,fp);
    fclose(fp);
}

//
// rn은 hash file에서의 레코드 번호를, sl은 search length를 의미한다.
//
void printSearchResult(int rn, int sl)
{
	printf("%d %d\n", rn, sl);
}

int main(int argc, char *argv[])
{
	// 학생레코드파일은 student.h에 정의되어 있는 STUDENT_FILE_NAME을, 
	// hash file은 HASH_FILE_NAME을 사용한다.

	// 검색 기능을 수행할 때 출력은 반드시 주어진 printSearchResult() 함수를 사용한다

    int rn, sl;
    if( strcmp(argv[1] , "-c") == 0 ){
        makeHashfile(atoi(argv[2]));

    }else if( strcmp(argv[1] , "-s") == 0){
        sl = search(argv[2], &rn);
        printSearchResult(rn,sl);

    }else if( strcmp(argv[1] , "-d") == 0){
        delete(argv[2]);
        
    }else{
        printf("잘못된 옵션입니다.\n\n-c <size> : create hash table\n-s <key> : search key\n-d <key> : delete key\n");
    }
}
