#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "blockmap.h"

FILE *flashfp;
int mappingTable[DATABLKS_PER_DEVICE];      //address mapping table
int freeblockNum = DATABLKS_PER_DEVICE;

void ftl_open()
{
	// address mapping table 초기화 또는 복구
	int i = 0;
	int count = 0;
	char *pagebuf;
	pagebuf = (char*)malloc(PAGE_SIZE);	
	int sparelbn = 0;
	
	
	for (i = 0; i<DATABLKS_PER_DEVICE; i++){
		mappingTable[i] = -1;
	}
	for (i = 0; i<BLOCKS_PER_DEVICE; i++){
		int j = i * PAGES_PER_BLOCK;
		dd_read(j, pagebuf);
		memcpy(&sparelbn, &pagebuf[SECTOR_SIZE], sizeof(int));
		if (sparelbn >= 0)
			mappingTable[sparelbn] = i;
		else ;
	}


	// free block's pbn 초기화
	   for (i = DATABLKS_PER_DEVICE; i>=0; i--){
		int j = i * PAGES_PER_BLOCK;
		dd_read(j, pagebuf);
		memcpy(&sparelbn, &pagebuf[SECTOR_SIZE], sizeof(int));
		if (sparelbn < 0){
			freeblockNum = i;
			break;
		}
		else ;
	}
	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_read(int lsn, char *sectorbuf)
{
	int lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	int ppn = mappingTable[lbn] * PAGES_PER_BLOCK + offset;
	char * pagebuf;
	pagebuf = (char*)malloc(PAGE_SIZE);
	memset(pagebuf, 0, PAGE_SIZE);
	dd_read(ppn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	return;
}

//
// 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
// (즉, 이 함수에서 메모리를 할당 받으면 안됨)
//
void ftl_write(int lsn, char *sectorbuf)
{
	int lbn = lsn/PAGES_PER_BLOCK;
	int offset = lsn%PAGES_PER_BLOCK;
	char *pagebuf;
	int i = 0;
	int j = 0;
	int spareLbn = 0;
	int spareLsn = 0;
	pagebuf = (char*)malloc(PAGE_SIZE);
	memset(pagebuf, 0, PAGE_SIZE);
	
	if (mappingTable[lbn] == -1){	//pbn 할당해줌
		while(i<BLOCKS_PER_DEVICE){
			j = i*PAGES_PER_BLOCK;
			dd_read(j, pagebuf);
			//spare에 저장된 lbn 읽어오기 
			memcpy(&spareLbn, &pagebuf[SECTOR_SIZE], sizeof(int));
			if (spareLbn < 0) {
				mappingTable[lbn] = i;
				break;
			}
			else i++;
		}
	}
	else ;
	
	int ppn = mappingTable[lbn] * PAGES_PER_BLOCK + offset;
	memset(pagebuf, 0, PAGE_SIZE);
	dd_read(ppn, pagebuf);
	memcpy(&spareLsn, &pagebuf[SECTOR_SIZE+4], sizeof(int));
	memset(pagebuf, 0, PAGE_SIZE);

	if (spareLsn >= 0){		//freeblock 사용
		int temp = freeblockNum;
		freeblockNum = mappingTable[lbn];
		mappingTable[lbn] = temp;
		
		ppn = mappingTable[lbn] * PAGES_PER_BLOCK + offset;

		//블록단위로 데이터 옮기기
		for (i = 0; i<PAGES_PER_BLOCK; i++){
			memset(pagebuf, 0, PAGE_SIZE);
			j = (freeblockNum*4) + i;
			int k = (mappingTable[lbn] * 4) + i;
			dd_read(j, pagebuf);
			dd_write(k, pagebuf);
		}

		//블록의 첫 페이지의 spare영역에 lbn값 저장
		j = mappingTable[lbn] * 4;
		memset(pagebuf, 0, PAGE_SIZE);
		dd_read(j, pagebuf);
		memcpy(&pagebuf[SECTOR_SIZE], &lbn, sizeof(int));
		dd_write(j, pagebuf);

		memset(pagebuf, 0, PAGE_SIZE);
		memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
		memcpy(&pagebuf[SECTOR_SIZE], &lbn, sizeof(int));
		memcpy(&pagebuf[SECTOR_SIZE+4], &lsn, sizeof(int));
		dd_write(ppn, pagebuf);
		//새 freeblock 초기화
		dd_erase(freeblockNum);                 
	}
	else {
		//블록의 첫 페이지의 spare영역에 lbn값 저장
		memset(pagebuf, 0, PAGE_SIZE);
		dd_read(5*mappingTable[lbn], pagebuf);
		memcpy(&pagebuf[SECTOR_SIZE], &lbn, sizeof(int));
		dd_write(mappingTable[lbn]*4, pagebuf);

		memset(pagebuf, 0, PAGE_SIZE);
		memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
		memcpy(&pagebuf[SECTOR_SIZE], &lbn, sizeof(int));
		memcpy(&pagebuf[SECTOR_SIZE+4], &lsn, sizeof(int));
		dd_write(ppn, pagebuf);
	}
	return;
}

void ftl_print()
{
	printf("lbn\t\tpbn\n");
    for (int i = 0; i<DATABLKS_PER_DEVICE; i++){
		printf("%d\t\t%d\n", i, mappingTable[i]);
    }
	printf("free block's pbn = %d\n", freeblockNum);
	return;
}

