/*
   표준입력으로 레코드 파일명을 읽어 들이고 (예: sread student1.dat), 이 파일에 저장되어 있는 모든 레코드를 순차적으로 읽고 이때 발생하는 전체 시간적 비용을 계산하여 출력한다.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>


int main(int argc, char **argv)
{
	struct timeval start_time, end_time;
	int fd = 0;		//파일 디스크립터
	int head = 0; 	//head record 받기
	int num = 0;	//읽어들인 전체 레코드 개수
	char student[250];	
	if(argc != 2){
		fprintf(stderr, "%s <파일명>\n", argv[0]);
		exit(1);
	}
	if((fd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	read(fd, &head, 4);		//record 수 읽어오기
	gettimeofday(&start_time, NULL);	//시간 측정 시작

	/* 모든 record 순차적으로 읽기 */	
	for(long i = 0; i<head; i++){
		lseek(fd, (i*250)+4, SEEK_SET);		//offset 위치 조절
		read(fd, student, 250);			//record 읽어오기
		num++;
	}	

	close(fd);
	gettimeofday(&end_time, NULL);	//시간 측정 종료
	long time_us = ((end_time.tv_sec - start_time.tv_sec)*1000000) + (end_time.tv_usec - start_time.tv_usec);
	printf("#records: %d  ", num);				//전체 레코드 수 출력
	printf("elapsed_time: %ld us\n", time_us);	//시간 출력
	
	return 0;
}

