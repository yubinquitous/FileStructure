#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	if(argc!= 4){
		printf("%s <파일명> <오프셋> <데이터>\n", argv[0]);
		exit(0);
	}

	char *cmd, *data;
	int offset;
	cmd = argv[1];
	offset = atoi(argv[2]);
	data = argv[3];
	
	int out;
	out = open(cmd, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	lseek(out, offset,SEEK_SET);
	int ndata;
	ndata = strlen(data);
	write(out, (char*)data, ndata);
	exit(0);
}
	



