#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

int printfield(int, char*);
// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	int pageOffset = (pagenum*PAGE_SIZE) + 16;
	fseek(fp, pageOffset, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	int pageOffset = (pagenum*PAGE_SIZE) + 16;
	fseek(fp, pageOffset, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf, 0, sizeof(MAX_RECORD_SIZE));
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	sscanf(recordbuf, "%[^'|']|%[^'|']|%[^'|']|%[^'|']|%[^'|']|%[^'|']|", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{

}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{

}

//
// 주어진 레코드 파일(recordfp)을 이용하여 심플 인덱스 파일(idxfp)을 생성한다.
//
void createIndex(FILE *idxfp, FILE *recordfp)
{
	int npages=0, nrecords=0;
	int count=0, n=0;
	int recordOffset=0, offset=0;
	char id[14];
	long long *idbuf;
	char pagebuf[PAGE_SIZE];
	char buf[21];	//indexfile record하나 담을 버퍼

	//header record에서 page수 읽기 
	fseek(recordfp, 0, SEEK_SET);
	fread(&npages, sizeof(int), 1, recordfp);
	//header record에서 record수 읽기
	fseek(recordfp, 4, SEEK_SET);
	fread(&n, sizeof(int), 1, recordfp);

	idbuf = (long long*)malloc(sizeof(long long)*n);

	//전체 page수만큼 readPage() 반복
	for (int i=0; i<npages; i++){
		memset(pagebuf, 0, PAGE_SIZE);
		readPage(recordfp, pagebuf, i);
		//header area에서 record수 읽기 -> nrecords
		memcpy(&nrecords, pagebuf, 4);
		
		//전체 record수만큼 readRecord
		for (int j=0; j<nrecords; j++){
			//header area에서 j번째 record 읽고 그 위치로 이동
			memcpy(&offset, pagebuf+4+(j*8), 4);
			recordOffset = offset+HEADER_AREA_SIZE;
			if (pagebuf[recordOffset] != '*'){
				//주민번호 읽기 (# 전까지)
				int c = 0;
				memset(id, '\0', 14);
				while(1){
					if (pagebuf[recordOffset+c] != '#'){
						id[c] = pagebuf[recordOffset+c];
						c++;
					}
					else break;
				}
				idbuf[count] = atol(id);

				//recordbuf에 차례로 id, i, j 넣기
				memset(buf, '\0', 21);
				memcpy(buf, id, 13);
				memcpy(&buf[13], &i, 4);
				memcpy(&buf[17], &j, 4);

				fseek(idxfp, (21*count)+4, SEEK_SET);
				fwrite(buf, 21, 1, idxfp);

				count++;
			}
			else ;	//삭제 레코드일 경우 : 아무것도 안함
		}
	}

	//indexfile header에 레코드 개수 넣기
	fseek(idxfp, 0, SEEK_SET);
	fwrite(&count, sizeof(int), 1, idxfp);
			
	//주민번호 오름차순 정렬
	for (int i = 0; i<count; i++){
		for (int j = 0; j<count-1-i; j++){
			if (idbuf[j] > idbuf[j+1]){	//레코드 순서 바꾸기
				char temp[21];
				long long idtemp;
				idtemp = idbuf[j];
				idbuf[j] = idbuf[j+1];
				idbuf[j+1] = idtemp;
				memset(temp, '\0', 21);
				memset(buf, '\0', 21);
				fseek(idxfp, 4+(21*j), SEEK_SET);
				fread(buf, 21, 1, idxfp);
				memcpy(temp, buf, 21);
				fseek(idxfp, 25+(21*j), SEEK_SET);
				fread(buf, 21, 1, idxfp);
				fseek(idxfp, 4+(21*j), SEEK_SET);
				fwrite(buf, 21, 1, idxfp);
				fseek(idxfp, 25+(21*j), SEEK_SET);
				fwrite(temp, 21, 1, idxfp);
			}
			else ;
		}
	}
}

//
// 주어진 심플 인덱스 파일(idxfp)을 이용하여 주민번호 키값과 일치하는 레코드의 주소, 즉 페이지 번호와 레코드 번호를 찾는다.
// 이때, 반드시 이진 검색 알고리즘을 사용하여야 한다.
//
void binarysearch(FILE *idxfp, const char *id, int *pageNum, int *recordNum)
{
	int nrecords=0;
	long long idxid, key;
	char idbuf[14];
	int count=0;
	int offset=0;
	char idxrecord[21];
	char *pagebuf;
	Person *k;

	//header에 있는 레코드 개수 읽기
	fseek(idxfp, 0, SEEK_SET);
	fread(&nrecords, sizeof(int), 1, idxfp);

	int low=0;
	int high = nrecords-1;
	key = atol(id);

	//이진검색
	while(1){
		offset = (low+high)/2;	
		if (low > high){
			printf("#reads : %d\n", count);
			break;
		}
		else {
			int indexOffset = 21*offset + 4;
			memset(idbuf, '\0', 14);
			memset(idxrecord, '\0', 21);
			fseek(idxfp, indexOffset, SEEK_SET);
			fread(idxrecord, 21, 1, idxfp);
			count++;
			memcpy(idbuf, idxrecord, 13);
			idxid = atol(idbuf);
			//idxid와 key 비교
			if (idxid == key){
				//idxrecord에서 pn, rn 뽑아오기
				memcpy(pageNum, idxrecord+13, sizeof(int));
				memcpy(recordNum, idxrecord+17, sizeof(int));
				printf("#reads : %d\n", count);
				break;
			}
			else {
				if (idxid < key){
					low = offset+1;
				}
				else {
					high = offset-1;
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *recordfp;  // 레코드 파일의 파일 포인터
	FILE *idxfp;
	Person *p;
	char *id;
	int *pageNum, *recordNum;
	int pn=-1, rn=-1;
	char pagebuf[PAGE_SIZE];

	p = (Person*)malloc(sizeof(Person));
	id = (char*)malloc(14);

	//createIndex()
	if (strcmp(argv[1], "i") == 0){
		if (argc != 4){
			fprintf(stderr, "Usage : %s i <record file name> <index file name>\n", argv[0]);
			exit(1);
		}
		if ((recordfp = fopen(argv[2], "r+")) == NULL){
			fprintf(stderr, "file open error\n");
			exit(1);
		}
		idxfp = fopen(argv[3], "w+");
		createIndex(idxfp, recordfp);
	}

	//binarysearch()
	else if (strcmp(argv[1], "b") == 0){
		if (argc != 5){
			fprintf(stderr, "Usage : %s b <record file name> <index file name> <key value>\n",
					argv[0]);
			exit(1);
		}
		if ((recordfp = fopen(argv[2], "r+")) == NULL){
			fprintf(stderr, "file open error\n");
			exit(1);
		}
		idxfp = fopen(argv[3], "r");
		id = argv[4];
		pageNum = &pn;
		recordNum = &rn;
		binarysearch(idxfp, id, pageNum, recordNum);
		if (pn != -1){
			readPage(recordfp, pagebuf, pn);
			//pagebuf에서 rn에 해당하는 record 읽어오기
			int o = 4 + 8*rn;
			int offset=0, len=0;
			memcpy(&offset, pagebuf+o, sizeof(int));
			memcpy(&len, pagebuf+o+4, sizeof(int));
			int recordOffset = HEADER_AREA_SIZE + offset;
			char recordbuf[len];
			memcpy(recordbuf, pagebuf+recordOffset, len);
			
			//record 출력
			int c=0;
			printf("id=");
			c = printfield(c, recordbuf);
			printf("name=");
			c = printfield(c, recordbuf);
			printf("age=");
			c = printfield(c, recordbuf);
			printf("addr=");
			c = printfield(c, recordbuf);
			printf("phone=");
			c = printfield(c, recordbuf);
			printf("email=");
			printfield(c, recordbuf);
		}
		else {
			printf("no persons\n");
		}
	}
	return 0;
}

int printfield(int c, char* recordbuf){
	while(recordbuf[c] != '#'){
		printf("%c", recordbuf[c]);
		c++;
	}
	c++;
	printf("\n");
	return c;
}



