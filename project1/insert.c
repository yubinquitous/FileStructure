#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("%s <파일명> <오프셋> <데이터>\n", argv[0]);
		exit(0);
	}

	char *cmd1, *cmd2;
	int nread1, nread2;
	int first, last;
	char buf1[BUFSIZ];
	char buf2[BUFSIZ];
	char *data;

	cmd1 = argv[1];
	int offset = atoi(argv[2]) + 1;
	data = argv[3];
	int ndata = strlen(data);

	//cmd1->buf1, buf2
	first = open(cmd1, O_RDWR);
	nread1 = read(first, buf1, sizeof(buf1));
	lseek(first, offset, SEEK_SET);
	nread2 = read(first, buf2, sizeof(buf2));
	last = open(cmd1, O_WRONLY | O_TRUNC);
	write(last, buf1, offset);

	//insert data
	lseek(first, 0, offset);
	write(last, (char*)data, ndata);
	
	//buf ->cmd1
	lseek(last, 0, SEEK_END);
	write(last, buf2, nread2);
	exit(0);
}

