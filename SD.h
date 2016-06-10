/*
 * simulation of Arduino SD class
 */
class File {
    public:
	File(char *name, int modes);
	void println(char *);
	char *read();
	void close();

    private:
    	int fdesc;
};

#define FILE_WRITE	1
#define	FILE_READ	0

class SD {
    public:
	static bool begin(int);
    	File *open(char *name, int modes);
};
