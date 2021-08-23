/*
	레코드의 크기가 250바이트라고 가정하고
	학생 레코드 파일을 생성하는 프로그램을 구현한다.

	- 고정길이레코드(fixed length record) 방식으로 250바이트짜리 레코드를 레코드 파일에 저장함.
	- 레코드 파일 맨앞에 4바이트짜리 헤더 레코드(head record)를 두며, 여기에 레코드 파일에 저장되어 있는 레코드의 수를 저장함. 
	레코드 수를 저장할 때 반드시 binary integer 모드로 저장함.
	- 헤더 레코드 다음 첫 번째 레코드를 저장해야 함.

	- 표준입력으로 레코드 수와 레코드 파일명을 읽어 들이고 (예: creat 1000 student1.dat), 주어진 레코드 수만큼의 레코드를 주어진 레코드 파일에 저장한다. 
	반드시 레코드 파일의 헤더 레코드에는 레코드 수가 저장되어야 한다.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i=0;
	char st1[250];

	if(argc != 3){
		printf("%s <record 수> <파일명>\n", *argv);
		exit(0);
	}
	int num = atoi(*++argv);					//record 개수
	int fd = open(*++argv, O_WRONLY | O_CREAT, 0640);		
	write(fd, &num, sizeof(num));				//head record에 record의 수 저장
	for (i=0; i<num; i++){
		memset(st1, 0,250);
		strncpy(st1, "20191234", 250);
		write(fd, st1, 250);
	}
	close(fd);
	exit(0);
	return 0;
}


