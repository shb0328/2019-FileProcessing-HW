
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "blkmap.h"

extern FILE *devicefp;
int FTLtable[DATABLKS_PER_DEVICE]; 
int freeBlk;


int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

void ftl_open()
{

    freeBlk = BLOCKS_PER_DEVICE - 1; 
	for(int i =0; i<DATABLKS_PER_DEVICE; ++i){
		FTLtable[i] = -1;
	}


printf("FTLtable ::");
for(int i=0; i<DATABLKS_PER_DEVICE; ++i){
printf("%d ",FTLtable[i]);
}
printf("\n\n");



    return;
}


void ftl_read(int lsn, char *sectorbuf)
{
printf("* read *\n\nFTLtable ::");
for(int i=0; i<DATABLKS_PER_DEVICE; ++i){
printf("%d ",FTLtable[i]);
}
printf("\n\n");


	int lbn = lsn/PAGES_PER_BLOCK;
	int offset = lsn%PAGES_PER_BLOCK;
	int pbn = FTLtable[lbn];

	if(-1 == pbn) {
		printf("error : pbn is empty.\n");
		return;
	}
	int ppn = (pbn*PAGES_PER_BLOCK)+offset;
	char readBuf[PAGE_SIZE] = {0, };
printf("lbn : %d, offset : %d, pbn : %d, ppn: %d, FTLtable[%d] : %d \n\n",lbn,offset,pbn,ppn,lbn,FTLtable[lbn]);
	if(-1 == dd_read(ppn,readBuf)){
		printf("error : dd_read is failed.\n");
		return;
	}

	memcpy(sectorbuf,readBuf,SECTOR_SIZE);

	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
printf("* write *\n\nFTLtable ::");
for(int i=0; i<DATABLKS_PER_DEVICE; ++i){
printf("%d ",FTLtable[i]);
}
printf("\n\n");


	int lbn = lsn/PAGES_PER_BLOCK;
	int offset = lsn%PAGES_PER_BLOCK;
	int pbn,ppn;

	if(FTLtable[lbn] == -1){ // 계산된 lbn에 할당된 pbn가 아직 없을 때,

		int num, usedCheck =0;
		for(num=0; num < BLOCKS_PER_DEVICE; ++num){
				usedCheck = 0;
				if(num != freeBlk) {
				
					for(int i =0; i<DATABLKS_PER_DEVICE; ++i){
						if(FTLtable[i] == num){
							usedCheck = 1;
							break;
						}
					}
				}else{
					usedCheck = 1;
					break;
				}
			if(usedCheck == 0) break;
		}
		FTLtable[lbn] = num;
	} 



	pbn = FTLtable[lbn];
	ppn = (pbn*PAGES_PER_BLOCK)+offset;

printf("lbn : %d, offset : %d, pbn : %d, ppn: %d, FTLtable[%d] : %d \n\n",lbn,offset,pbn,ppn,lbn,FTLtable[lbn]);

	char writeBuf[PAGE_SIZE] = {0,};
	char readBuf[PAGE_SIZE] = {0, };
	long double spare = 1; //long double 이 16 byte 라서 사용
	memcpy(writeBuf, sectorbuf, SECTOR_SIZE);
	memcpy(writeBuf+SECTOR_SIZE, &spare, SPARE_SIZE);

	spare = 0;
	dd_read(ppn,readBuf);
	memcpy(&spare, readBuf+SECTOR_SIZE, SPARE_SIZE);

	if(1 == spare){ //overwrite

		//change pbn <-> freeBlk & update ppn
		int priorPbn = pbn;
		pbn = freeBlk;
		freeBlk = priorPbn;
		FTLtable[lbn] = pbn;

		int priorPpn = ppn;
		ppn = (pbn*PAGES_PER_BLOCK)+offset;


		if(-1 == dd_write(ppn,writeBuf)){//write A'(new data)
			printf("error : dd_write is failed.\n");
			return;
		}

		char cpySectorBuf[SECTOR_SIZE] = {0,};
		char cpyBuf[PAGE_SIZE] = {0,};
		for(int i =0; i<PAGES_PER_BLOCK; ++i){ //overwrite 시도된 페이지가 포함된 블럭의 나머지 유효한 페이지들을 옮김
			int tmp = (priorPbn*PAGES_PER_BLOCK) + i;
			
			if (tmp != priorPpn){//이전 데이터가 쓰여진 페이지인 경우 복사할 필요 없음
				//나머지 페이지의 경우 spare 값 검사 후 복사 실행
				if(1 == dd_read(tmp,cpyBuf)){
					spare = 0;
					memcpy(&spare, cpyBuf+SECTOR_SIZE, SPARE_SIZE);
					if(spare == 1) {//1이면 유효
					dd_write((pbn*PAGES_PER_BLOCK) + i,cpyBuf);
					}
				}
			} else{

			}
		}

		//erase freeBlk
		dd_erase(freeBlk);

	}else{ //just write
		if(-1 == dd_write(ppn,writeBuf)){
			printf("error : dd_write is failed.\n");
			return;
		}		
	}

	return;
}
