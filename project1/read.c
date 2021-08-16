#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFSIZE 1024

int main(int argc, char* argv[]){
	char buf[BUFSIZE];
	int count;
	int fd1;
	char *cmd1;
	cmd1=argv[1];

	fd1 = open(cmd1, O_RDONLY, 0644);
	if (fd1 < 0){
		fprintf(stderr, "open error for %s\n", cmd1);
		exit(1);
	}

	int offset, nbyte;
	offset = atoi(argv[2]);
	nbyte = atoi(argv[3]);


	lseek(fd1, offset, SEEK_SET);
	count = read(fd1,buf,nbyte);
	write(1, buf, count);
	exit(0);
}
