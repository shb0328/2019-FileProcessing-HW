#include <stdio.h>
#include "student.h"
#include <stdlib.h>
#include <string.h>

//
// �л� ���ڵ� ���Ϸκ���  ���ڵ� ��ȣ�� �ش��ϴ� ���ڵ带 �о� ���ڵ� ���ۿ� �����Ѵ�.
//
void readStudentRec(FILE *fp, char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*STUDENT_RECORD_SIZE),SEEK_SET);
    fread(recordbuf,STUDENT_RECORD_SIZE,1,fp);
}

//
// Hash file�κ��� rn�� ���ڵ� ��ȣ�� �ش��ϴ� ���ڵ带 �о� ���ڵ� ���ۿ� �����Ѵ�.
//
void readHashRec(FILE *fp, char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*HASH_RECORD_SIZE+4),SEEK_SET);
    fread(recordbuf,HASH_RECORD_SIZE,1,fp);
}

//
// Hash file�� rn�� ���ڵ� ��ȣ�� �ش��ϴ� ��ġ�� ���ڵ� ������ ���ڵ带 �����Ѵ�.
//
void writeHashRec(FILE *fp, const char *recordbuf, int rn)
{
    fseek(fp,(long)(rn*HASH_RECORD_SIZE+4),SEEK_SET);
    fwrite(recordbuf,HASH_RECORD_SIZE,1,fp);
}

//
// n�� ũ�⸦ ���� hash file���� �־��� �й� Ű���� hashing�Ͽ� �ּҰ�(���ڵ� ��ȣ)�� �����Ѵ�.
//
int hashFunction(const char *sid, int n)
{
    int rn;
    rn = (sid[(strlen(sid)-1)-1]*sid[strlen(sid)-1])%n;
    return rn;
}

//
// n�� ũ�⸦ ���� hash file�� �����Ѵ�.
// Hash file�� fixed length record ������� ����Ǹ�, ���ڵ��� ũ��� 14 ����Ʈ�̴�.
// (student.h ����)
//
void makeHashfile(int n)
{
	// Hash file�� ������ �� �� ������ �Ǿպκп� 4����Ʈ ����� �д�. 
	// ���⿡�� hash file�� ũ�� n�� �����Ѵ�. �̰��� search()�� (�ʿ��ϸ�) delete()�� 	
    // ���� ���̴�.

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
// �־��� �й� Ű���� hash file���� �˻��Ѵ�.
// �� ����� �־��� �й� Ű���� �����ϴ� hash file������ �ּ�(���ڵ� ��ȣ)�� search length�̴�.
// �˻��� hash file������ �ּҴ� rn�� �����ϸ�, �̶� hash file�� �־��� �й� Ű����
// �������� ������ rn�� -1�� �����Ѵ�. (search()�� delete()������ Ȱ���� �� ����)
// search length�� �Լ��� ���ϰ��̸�, �˻� ����� ������� search length�� �׻� ���Ǿ�� �Ѵ�.
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
        if(strcmp(candidateSid,sid) == 0) //ã�Ҵ�!
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
// Hash file���� �־��� �й� Ű���� ��ġ�ϴ� ���ڵ带 ã�� �� �ش� ���ڵ带 ���� ó���Ѵ�.
// �̶� �л� ���ڵ� ���Ͽ��� ���ڵ� ������ �ʿ����� �ʴ�.
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
// rn�� hash file������ ���ڵ� ��ȣ��, sl�� search length�� �ǹ��Ѵ�.
//
void printSearchResult(int rn, int sl)
{
	printf("%d %d\n", rn, sl);
}

int main(int argc, char *argv[])
{
	// �л����ڵ������� student.h�� ���ǵǾ� �ִ� STUDENT_FILE_NAME��, 
	// hash file�� HASH_FILE_NAME�� ����Ѵ�.

	// �˻� ����� ������ �� ����� �ݵ�� �־��� printSearchResult() �Լ��� ����Ѵ�

    int rn, sl;
    if( strcmp(argv[1] , "-c") == 0 ){
        makeHashfile(atoi(argv[2]));

    }else if( strcmp(argv[1] , "-s") == 0){
        sl = search(argv[2], &rn);
        printSearchResult(rn,sl);

    }else if( strcmp(argv[1] , "-d") == 0){
        delete(argv[2]);
        
    }else{
        printf("�߸��� �ɼ��Դϴ�.\n\n-c <size> : create hash table\n-s <key> : search key\n-d <key> : delete key\n");
    }
}
