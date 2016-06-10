/* fake arduino.h for testing */
extern long millis();
extern short analogRead(int);
extern void digitalWrite(int, short);
extern void pinMode(int, int);
#define INPUT 0
#define OUTPUT 1

#include <stdlib.h>	// for malloc
