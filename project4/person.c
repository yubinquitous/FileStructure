#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "person.h"

#define MAX_RECORDS ((HEADER_AREA_SIZE-4)/8)
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.

// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다

// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.


// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다.
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
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

}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{
	char recordbuf[MAX_RECORD_SIZE];
	char pagebuf[PAGE_SIZE];
	char prepagebuf[PAGE_SIZE];
	int pagenum = 0;
	int recordnum = 0;
	pack(recordbuf, p);
	char deletedBuf[MAX_RECORD_SIZE];
	int recordlen = strlen(recordbuf);
	int prepagenum = -1, nextpagenum = -1;
	int prerecordnum = -1, nextrecordnum = -1;
	int preindex = -1;
	int predrecordOffset = -1;
	char buf_int[4]; //int형 담을 임시버퍼
	int pageCount = 0, recordCount = 0;	//header record 정보
	int records=0, header_offset=0;	//header area 정보
	//int page_length=0;	//page 전체 길이
	int newrecordOffset=0;
	int count = 0;
	
	//삭제 레코드 페이지번호, 레코드 번호 읽기
	fseek(fp, 8, SEEK_SET);
	fread(&pagenum, sizeof(int), 1, fp);
	fseek(fp, 12, SEEK_SET);
	fread(&recordnum, sizeof(int), 1, fp);


	while(1){	//right size를 찾을 때까지 반복
		if (pagenum != -1){		//삭제 레코드가 있다면
			//해당 페이지 읽어오기
			memset(pagebuf, 0, PAGE_SIZE);
			readPage(fp, pagebuf, pagenum);
			//header area에서 삭제된 recordnum 찾아서 이동
			int index = recordnum*8 + 4;	//header area에서의 삭제레코드 offset
			int drecordOffset=0, drecordlen=0;
			memcpy(&drecordOffset, pagebuf+index, 4);
			memcpy(&drecordlen, pagebuf+index+4, 4);
			drecordOffset += HEADER_AREA_SIZE;

			//첫번째 offset은 항상 0,
			//Header Area 크기도 추가해야 함
			if (drecordlen >= recordlen){	//rightsize
				memcpy(&nextpagenum, pagebuf+drecordOffset+1, 4);
				memcpy(&nextrecordnum, pagebuf+drecordOffset+5, 4);

				if (count == 0){	//전체 삭제 레코드 : 1개 => header record 수정			
					fseek(fp, 8, SEEK_SET);
					fwrite(&nextpagenum, sizeof(int), 1, fp);	//삭제된레코드는 이제 하나도 X
					fseek(fp, 12, SEEK_SET);
					fwrite(&nextrecordnum, sizeof(int), 1, fp);	//삭제된 레코드는 이제 하나도 X
				}
				else {	//linked list 수정
					if (pagenum == prepagenum){	//readPage 생략
						//삭제 레코드의 다음 recordnum 변경
						memcpy(pagebuf+predrecordOffset+5, &nextrecordnum, 4);	//이전 삭제레코드와 연결
					}
					else {	//readPage 다시 해야
						readPage(fp, prepagebuf, prepagenum);			//이전 page 읽어오기
						memcpy(prepagebuf+predrecordOffset+1, &nextpagenum, 4);
						memcpy(prepagebuf+predrecordOffset+5, &nextrecordnum, 4);
						writePage(fp, prepagebuf, prepagenum);
					}	
				}
				//pagebuf에 입력받은 person 정보 저장
				memcpy(pagebuf+drecordOffset, recordbuf, recordlen);
				writePage(fp, pagebuf, pagenum);
				break;
			}
			else if (drecordlen < recordlen){	//right size가 아님
				//다음 삭제 레코드로 이동
				prepagenum = pagenum;
				prerecordnum = recordnum;
				predrecordOffset = drecordOffset;
				preindex = index;
				memcpy(&pagenum, pagebuf+drecordOffset+1, 4);
				memcpy(&recordnum, pagebuf+drecordOffset+5, 4);
				count++;
				continue;
			}
		}
		else {	//삭제 레코드가 없거나 right size를 끝까지 찾기 못함	
			//마지막 페이지로 이동
			fseek(fp, 0, SEEK_SET);
			fread(&pageCount, sizeof(int), 1, fp);
			fread(&recordCount, sizeof(int), 1, fp);
	
			if (pageCount == 0){	//record 파일 첫 생성
				memset(pagebuf, 0, PAGE_SIZE);
				memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, recordlen);
		
				//header area 수정
				pagenum = 0;
				records = 1;
				header_offset = 0;
				memcpy(pagebuf, &records, 4);
				memcpy(pagebuf+4, &header_offset, 4);
				memcpy(pagebuf+8, &recordlen, 4);

				//HEADER_RECORD 수정->성공
				pageCount = 1;
				fseek(fp, 0, SEEK_SET);
				fwrite(&pageCount, sizeof(int), 1, fp);
				fwrite(&records, sizeof(int), 1, fp);
				writePage(fp, pagebuf, pagenum);
			}
			else {
				pagenum = pageCount -1;
				memset(pagebuf, 0, PAGE_SIZE);
				readPage(fp, pagebuf, pagenum);
				memcpy(&records, pagebuf, 4);
				int o = 0, r = 0;
				int page_length = 0;
				int ind = (records*8)-4;
				memcpy(&o, pagebuf+ind, 4);
				memcpy(&r, pagebuf+ind+4, 4);
				page_length = o+r;

				if ((recordlen < DATA_AREA_SIZE - page_length) && (records < MAX_RECORDS)){	//page 마지막에 append
					newrecordOffset = page_length;
					memcpy(pagebuf+newrecordOffset+HEADER_AREA_SIZE, recordbuf, recordlen);
					//Header Area 수정
					o = 8*records + 4;
					records=records+1;
					memcpy(pagebuf, &records, 4);
					memcpy(pagebuf+o, &newrecordOffset, 4);
					memcpy(pagebuf+o+4, &recordlen, 4);
					writePage(fp, pagebuf, pagenum);

					//HEADER_RECORD 수정
					fseek(fp, 4, SEEK_SET);
					recordCount++;
					fwrite(&recordCount, sizeof(int), 1, fp);
				}
				else {	//page 추가
					pagenum = pageCount;
					memset(pagebuf, 0, PAGE_SIZE);
					memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, recordlen);
					//header area 수정
					records = 1;
					header_offset=0;
					memcpy(pagebuf, &records, 4);
					memcpy(pagebuf+4, &header_offset, 4);
					memcpy(pagebuf+8, &recordlen, 4);

					writePage(fp, pagebuf, pagenum);
					//HEADER_RECORD 수정
					fseek(fp, 4, SEEK_SET);
					fread(&recordCount, sizeof(int), 1, fp);
					pageCount++;
					recordCount++;
					fseek(fp, 0, SEEK_SET);
					fwrite(&pageCount, sizeof(int), 1, fp);
					fwrite(&recordCount, sizeof(int), 1, fp);
				}
			}
		}
		break;
	}
} 

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	int pageCount = 0;
	int records=0, index=0, record_offset=0;	//header area 정보
	char pagebuf[PAGE_SIZE];
	char buf_int[4];
	char recordbuf[MAX_RECORD_SIZE];	//id 담을 배열
	char delete_mark = '*';
	int dpagenum = -1, drecordnum=-1;	//header record 정보
	int init = -1;	//삭제 레코드 가장 최신값

	fseek(fp, 0, SEEK_SET);
	fread(&pageCount, sizeof(int), 1, fp);
	for (int i = 0; i<pageCount; i++){	//i : page
		memset(pagebuf, 0, PAGE_SIZE);
		readPage(fp, pagebuf, i);
		//header area에서 record 수 읽어오기
		memcpy(&records, pagebuf, 4);

		for (int j = 0; j<records; j++){	//j : record
			//record의 offset 읽어오기
			index = j*8+4;
			memcpy(&record_offset, pagebuf+index, sizeof(int));
			int k = 0;
			memset(recordbuf,0,MAX_RECORD_SIZE);
			//record에서 id 읽어오기
			while(1){
				memcpy(recordbuf+k, pagebuf+record_offset+HEADER_AREA_SIZE+k, 1);
				if (recordbuf[k] == '#'){
					recordbuf[k] = '\0';
					break;
				}
				else k++;
			}

			//id 찾기 
			if (strcmp(id, recordbuf) == 0){
				fseek(fp, 8, SEEK_SET);
				fread(&dpagenum, sizeof(int), 1, fp);
				fread(&drecordnum, sizeof(int), 1, fp);
				
				memcpy(pagebuf+record_offset+HEADER_AREA_SIZE, &delete_mark, 1);	//mark -> 1바이트
				memset(buf_int, 0, sizeof(int));
				if (dpagenum == -1){	//기존의 삭제 레코드가 없음
					memcpy(pagebuf+record_offset+HEADER_AREA_SIZE+1, &init, 4);
					memcpy(pagebuf+record_offset+HEADER_AREA_SIZE+5, &init, 4);
					}
				else {
					memcpy(pagebuf+record_offset+HEADER_AREA_SIZE+1, &dpagenum, 4);
					memcpy(pagebuf+record_offset+HEADER_AREA_SIZE+5, &drecordnum, 4);
				}
				writePage(fp, pagebuf, i);

				//header record 수정
				fseek(fp, 8, SEEK_SET);
				fwrite(&i, sizeof(int), 1, fp);
				fwrite(&j, sizeof(int), 1, fp);
				break;
			}
			else ;	
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	Person *p;
	char pagebuf[PAGE_SIZE];
	int pagenum = 0;
	p = (Person *)malloc(sizeof(Person));
	if ((fp = fopen(argv[2], "r+")) == NULL){
		fp = fopen(argv[2], "w+");
		//헤더 레코드 초기화
		int count = 0;	//전체 페이지, 레코드 초기값	
		int init = -1;	//pn, rn 초기값
		fseek(fp, 0, SEEK_SET);
		fwrite(&count, sizeof(int), 1, fp);	//data page 수 초기화
		fwrite(&count, sizeof(int), 1, fp);	//레코드 수 초기화
		fwrite(&init, sizeof(int), 1, fp);	//페이지번호 초기화	
		fwrite(&init, sizeof(int), 1, fp);	//레코드번호 초기화
	}

	//레코드 삽입(add)
	if (strcmp(argv[1], "a") == 0){
		if (argc != 9){
			fprintf(stderr, "%s a <record file name> <field values list>\n", argv[0]);
			exit(1);
		}
		//Person 구조체에 먼저 저장
		strcpy(p->id, argv[3]);
		strcpy(p->name, argv[4]);
		strcpy(p->age, argv[5]);
		strcpy(p->addr, argv[6]);
		strcpy(p->phone, argv[7]);
		strcpy(p->email, argv[8]);
		add(fp, p);
	}
	//레코드 삭제(delete)
	if (strcmp(argv[1], "d") == 0){
		if (argc != 4){
			fprintf(stderr, "%s d <record file name> <field value>\n", argv[0]);
			exit(1);
		}
		delete(fp, argv[3]);
	}

	fclose(fp);
	return 1;
}

