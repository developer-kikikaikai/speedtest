#include<stdio.h>
#include<stdlib.h>
#include "timetestlog.h"
#define TMPFNAME ".tmpfile"

static int create_testdata(int cnt) {
	int i=0;
	FILE *fp = fopen(TMPFNAME, "w");
	if(!fp) return -1;

	
	for(i=0;i<cnt;i++) {
		fprintf(fp, "%d, %d\n", i, time(NULL));
	}
	fclose(fp);
	return 0;
}

static inline void read_all_file(FILE *fp, void *handle) {
	int input1, input2;
	char buffer[256];
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		sscanf(buffer, "%d, %d", &input1, &input2);
//		timetestlog_store_printf(handle, "%d,%d\n", input1, input2);
	}
}

void read_testdata(void *handle) {
	//try by fopen directory
	timetestlog_store_printf(handle, "Start to read file directory!\n");

	FILE *fp = fopen(TMPFNAME, "r");
	if(!fp) return ;

	read_all_file(fp, handle);
	fclose(fp);

	timetestlog_store_printf(handle, "End to read file directory!\n");

	//try by popen
	timetestlog_store_printf(handle, "Start to read file by popen!\n");

	fp = popen("cat "TMPFNAME , "r");
	if(!fp) return;

	read_all_file(fp, handle);
	pclose(fp);
	timetestlog_store_printf(handle, "End to read file by popen!\n");
}

int main(int argc, char *argv[]) {
	if(argc < 2) return 0;
	int fsize=atoi(argv[1]);

	/*write id, len*/
	if(create_testdata(fsize)) return 0;

	void *handle = timetestlog_init(",", 100, fsize*2+20);
	if(!handle) return 0;

	read_testdata(handle);

	//exit
	timetestlog_exit(handle);
	unlink(TMPFNAME);
	return 0;
}
