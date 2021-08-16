#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	if (argc!= 4){
		printf("%s [filename_final] [filename1] [filename2]\n", argv[0]);
		exit(0);
	}

	char *cmd1;
	char *cmd2;
	char *cmd_final;

	cmd_final = argv[1];
	cmd1 = argv[2];
	cmd2 = argv[3];

	char buf[BUFSIZ];
	int in1, in2, out;
	int nread1, nread2;

	in1 = open(cmd1, O_RDONLY);
	in2 = open(cmd2, O_RDONLY);
	out = open(cmd_final, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	nread1 = read(in1, buf, sizeof(buf));
	write(out, buf, nread1);
	lseek(out, 0, SEEK_END);
	nread2 = read(in2, buf, sizeof(buf));
	write(out, buf, nread2);
	exit(0);
}
