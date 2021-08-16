#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char* argv[]){
	if(argc != 3){
		printf("%s [file name] [copy filename]\n", argv[0]);
		exit(0);
	}

	char *cmd1;
	char *cmd2; 
	cmd1 = argv[1];
	cmd2 = argv[2];
	char buf[10];
	int nread;
	int in, out;
	in = open(cmd1, O_RDONLY);
	out = open(cmd2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	while((nread = read(in, buf, sizeof(buf)))!=0){
		write(out, buf, nread);
	}
	exit(0);


	//nread = read(in, buf, sizeof(buf));
	//write(out, buf, nread);
	exit(0);
}
