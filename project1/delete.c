#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	if (argc != 4){
		printf("%s <파일명> <오프셋> <삭제 바이트 수>", argv[0]);
		exit(0);
	}

	char *cmd;
	int nread1, nread2;
	int in, out;
	char buf1[BUFSIZ];
	char buf2[BUFSIZ];
	int nbyte; //삭제 바이트 수

	cmd = argv[1];
	int offset = atoi(argv[2]);
	nbyte = atoi(argv[3]);

	in = open(cmd, O_RDWR);
	nread1 = read(in, buf1, offset);
	int p = nbyte + offset;
	lseek(in, p, SEEK_SET);
	nread2 = read(in, buf2, sizeof(buf2));

	out = open(cmd, O_WRONLY | O_TRUNC);
	write(out, buf1, nread1);
	write(out, buf2, nread2);
	exit(0);
}



	
