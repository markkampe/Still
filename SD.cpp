/**
 * simulation of Arduino SD library
 */
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include "SD.h"

bool SD::begin(int pin) {
	return true;
}

File *SD::open(char *name, int modes) {
	return new File(name, modes);
}

File::File(char *name, int modes) {
	if (modes == FILE_WRITE) {
		fdesc = creat(name, 0666);
	} else {
		fdesc = open(name, 0);
	}
}

void File::close() {
	::close(fdesc);
	fdesc = -1;
}

void File::println(char *text) {
	if (fdesc >= 0) {
		write(fdesc, text, strlen(text));
		write(fdesc, "\n", 1);
	}
}

char *File::read() {
	return((char *) "");
}
