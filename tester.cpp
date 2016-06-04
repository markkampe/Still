/*
 * this is a test driver to exercise the still class
 * (as such, it is not much like a real still manager)
 */
#include "still.h"
#include "Arduino.h"
#include <stdio.h>

static struct sensor_cfg sensors[] = {
	{1, 0}, {2, 0},	// two kettle sensors
	{3, 1}, {4, 1},	// two ambient sensors
	{5, 2}, 	// one condensor sensor
	{6, 3}		// one alcohol sensor
};

static struct zone_cfg zones[] = {
	{64, 800, 50},	// kettle temp (sensor units)
	{0, 400, 50},	// ambient temp (sensor units)
	{64, 700, 50},	// condensor temp (sensor units)
	{0, 800, 50}	// alcohol (sensor units)
};

int clock = 0;
static char *sts[] = {	// map for status display
	(char *) "ready", (char *) "busy",
	(char *) "high", (char *) "low", (char *) "wide",
	(char *) "fast", (char *) "slow" };

void dumpStatus(Still *still ) {
	extern int sensorToTemp(int);
	printf("%d:\tstatus=%s, heater %s\n", clock, 
		sts[still->curStatus], 
		still->heating ? "on" : "off");
	printf("\tkettle %dF\n", sensorToTemp(still->curReading[0]));
	printf("\tambient %dF\n", sensorToTemp(still->curReading[1]));
	printf("\tcondens %dF\n", sensorToTemp(still->curReading[2]));
	printf("\talcohol %d\n", still->curReading[3]);
}

/*
 * wait up to a specified period of time for an operation to complete
 */
void waitfor(Still *still, int minutes) {
	extern void tick(int);	// simulator

	for(int m = 0; m < minutes; m++ ) {
		bool cont = still->checkStatus();
		dumpStatus(still);
		digitalWrite(10, still->heating);
		if (!cont)
			break;
		clock++;
		tick(1);
	}
	digitalWrite(10, 0);	// heater off
	printf("Operation completed\n");
	dumpStatus(still);
}

/*
 * run a standard test scenario
 */
void test(Still *still) {
	extern int tempToSensor(int);

	// bring it up to 180
	printf("HEAT 180\n");
	still->setCommand(Still::heat, tempToSensor(180), tempToSensor(180));
	waitfor(still,100);

	// hold it between 175 and 185 for 10 minutes
	printf("HOLD 175-185, 10 minutes\n");
	still->setCommand(Still::hold, tempToSensor(175), tempToSensor(185));
	waitfor(still,10);

	// take it down to 100
	printf("COOL 100\n");
	still->setCommand(Still::cool, tempToSensor(100), tempToSensor(100));
	waitfor(still,100);

	printf("Test completed\n");
}

int main(int argc, char **argv) {
	// instantiate a still controller
	Still *still = new Still(sensors, zones);

	// simulated heater relay pin
	pinMode(10, OUTPUT);

	test(still);
}
