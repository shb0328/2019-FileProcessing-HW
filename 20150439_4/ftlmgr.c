
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "blkmap.h"
#include <stdbool.h>

extern FILE *devicefp;
int FTLtable[DATABLKS_PER_DEVICE];
int freeBlk;


void ftl_open()
{
    freeBlk = DATABLKS_PER_DEVICE;
    for(int i =0; i<DATABLKS_PER_DEVICE; ++i)
        FTLtable[i] = -1;

    return;
}


void ftl_read(int lsn, char *sectorbuf)
{

    int lbn = lsn/NONBUF_PAGES_PER_BLOCK;
    int offset = lsn%NONBUF_PAGES_PER_BLOCK;
    int pbn = FTLtable[lbn];

    if(-1 == pbn) {
        printf("error : pbn is empty.\n");
        return;
    }
    int ppn = (pbn*PAGES_PER_BLOCK)+offset;
    char readBuf[PAGE_SIZE] = {0, };
    if(-1 == dd_read(ppn,readBuf)){
        printf("error : dd_read is failed.\n");
        return;
    }
  char spare_ok[3]; //"ok" 이면 유효값 , "--"이면 overwrite
    int spare_lsn; 
    memcpy(spare_ok,readBuf+SECTOR_SIZE,sizeof(spare_ok));
    memcpy(&spare_lsn,readBuf+SECTOR_SIZE+sizeof(spare_ok),sizeof(spare_lsn));

    if(strcmp(spare_ok,"ok") != 0){ //spare_ok 가 ok가 아니면,

        for(int i = 1 ; i<=BUF_PAGES_PER_BLOCK; ++i){
            ppn = ((pbn+1)*PAGES_PER_BLOCK)-i; //backward
            if(-1 == dd_read(ppn,readBuf)){
                printf("error : dd_read is failed.\n");
                return;
            }
            memcpy(spare_ok,readBuf+SECTOR_SIZE,sizeof(spare_ok));
            memcpy(&spare_lsn,readBuf+SECTOR_SIZE+sizeof(spare_ok),sizeof(spare_lsn));

            if(strcmp(spare_ok,"ok") == 0){
                if(spare_lsn == lsn){
                    break;
                }
            }
        }
    }

    memcpy(sectorbuf,readBuf,SECTOR_SIZE);

    return;
}



void ftl_write(int lsn, char *sectorbuf)
{

    int lbn = lsn/NONBUF_PAGES_PER_BLOCK;
    int offset = lsn%NONBUF_PAGES_PER_BLOCK;
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


    char writeBuf[PAGE_SIZE] = {0,};
    char readBuf[PAGE_SIZE] = {0, };

    char spare_ok[3] = "ok"; //"ok" , "--"
    int spare_lsn = lsn; 
    memcpy(writeBuf, sectorbuf, SECTOR_SIZE);
    memcpy(writeBuf+SECTOR_SIZE,spare_ok,sizeof(spare_ok));
    memcpy(writeBuf+SECTOR_SIZE+sizeof(spare_ok),&spare_lsn,sizeof(spare_lsn));

    dd_read(ppn,readBuf);
    memcpy(spare_ok,readBuf+SECTOR_SIZE,sizeof(spare_ok));
    memcpy(&spare_lsn,readBuf+SECTOR_SIZE+sizeof(spare_ok),sizeof(spare_lsn));


    if(strcmp(spare_ok,"ok") == 0){//overwrite

        memcpy(readBuf+SECTOR_SIZE,"--",sizeof("--"));
        dd_write(ppn,readBuf); //"--" update

        bool needErase=true;
        for(int i = 0; i<BUF_PAGES_PER_BLOCK; ++i){

            int tmpPpn = (pbn*PAGES_PER_BLOCK)+NONBUF_PAGES_PER_BLOCK+i;
            dd_read(tmpPpn,readBuf);
            memcpy(spare_ok,readBuf+SECTOR_SIZE,sizeof(spare_ok));
            memcpy(&spare_lsn,readBuf+SECTOR_SIZE+sizeof(spare_ok),sizeof(spare_lsn));

            if (strcmp(spare_ok,"ok") == 0){
                if(spare_lsn == lsn){
                    memcpy(readBuf+SECTOR_SIZE,"--",sizeof("--"));
                    dd_write(tmpPpn,readBuf); //"--" update
                }
            }
            else if((strcmp(spare_ok,"ok") != 0) && (strcmp(spare_ok,"--") != 0)){ //empty buf!
                needErase = false;
		dd_write(tmpPpn,writeBuf);
                break;
            }
        }


        if(needErase){

            //change 'pbn <-> freeBlk' & update ppn
            int priorPbn = pbn;
            pbn = freeBlk;
            freeBlk = priorPbn;
            FTLtable[lbn] = pbn;

            ppn = (pbn*PAGES_PER_BLOCK)+offset;

            if(-1 == dd_write(ppn,writeBuf)){ //write A'(new data)
                printf("error : dd_write is failed.\n");
            return;
            }

            for(int i = 1 ; i<=PAGES_PER_BLOCK; ++i){ 
                int tmpPpn = ((freeBlk+1)*PAGES_PER_BLOCK)-i; //backward

                if(-1 == dd_read(tmpPpn,readBuf)){
                    printf("error : dd_read is failed.\n");
                    return;
                }

                memcpy(spare_ok,readBuf+SECTOR_SIZE,sizeof(spare_ok));
                memcpy(&spare_lsn,readBuf+SECTOR_SIZE+sizeof(spare_ok),sizeof(spare_lsn));

                if (strcmp(spare_ok,"ok") == 0){
                 ppn = (pbn*PAGES_PER_BLOCK)+(spare_lsn%NONBUF_PAGES_PER_BLOCK);
                    dd_write(ppn,readBuf);
                }
            }//end of for
            dd_erase(freeBlk);

        }

    }else{ //just write
        if(-1 == dd_write(ppn,writeBuf)){
            printf("error : dd_write is failed.\n");
            return;
        }   
    }    
    
    return;
}
