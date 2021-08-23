/*
   표준입력으로 레코드 파일명을 읽어 들이고 (예: rread student2.dat), 이 파일에 저장되어 있는 모든 레코드를 무작위로 읽고 이때 발생하는 전체 시간적 비용을 계산하여 출력한다.
*/


#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int num_of_records; // 레코드 파일에 저장되어 있는 전체 레코드의 수
void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
	int *read_order_list;
	struct timeval start_time, end_time;
	int fd = 0;		//파일 디스크립터
	int num = 0;	//전체 레코드 수
	char student[250];

	if (argc != 2){
		fprintf(stderr, "%s <파일명>\n", argv[0]);
		exit(1);
	}
	if ((fd = open(argv[1], O_RDONLY)) < 0){	//record file open
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	read(fd, &num_of_records, 4);	//head record 읽어오기
	gettimeofday(&start_time,NULL); //시간 측정 시작
	read_order_list = (int*)malloc(sizeof(int)*num_of_records);
	GenRecordSequence(read_order_list, num_of_records);
	//모든 record random 읽기
	for (long i = 0; i<num_of_records; i++){
		int p = read_order_list[i];
		lseek(fd, (p*250)+4, SEEK_SET);       //offset 위치 조절
		read(fd, student, 250);		//record 250씩 읽음	
		num++;
	}
	free(read_order_list);
	close(fd);
	gettimeofday(&end_time, NULL);	//시간 측정 종료
	long time_us = ((end_time.tv_sec - start_time.tv_sec)*1000000) + (end_time.tv_usec - start_time.tv_usec);
	printf("#records: %d  ", num);
	printf("elapsed_time: %ld us\n", time_us);	//시간 출력 
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
		
	for(i=0; i<num_of_records; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
