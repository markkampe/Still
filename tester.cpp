/*
 * this is a test driver to exercise the still controller class
 * (as such, it is not much like a real still manager)
 */
#include <stdio.h>
#include "SD.h"
#include "controller.h"
#include "sensors.h"
#include "brew.h"
#include "still.h"
#include "simulator.h"
#include "Arduino.h"

// configuration of sensors
static struct sensor_cfg sensors[] = {
	{1, 0}, {2, 0},	// two kettle sensors
	{3, 1}, {4, 1},	// two ambient sensors
	{5, 2}, 	// one condensor sensor
	{6, 3},		// one alcohol sensor
	{0, 0}		// LAST ENTRY
};

// configuration of zones
static struct zone_cfg zones[] = {
	{64, 800, 50},	// kettle temp (sensor units)
	{0, 400, 50},	// ambient temp (sensor units)
	{64, 700, 50},	// condensor temp (sensor units)
	{0, 800, 50}	// alcohol (sensor units)
};

// still parameters
#define	HEATER		3000	// 1500 Watts
#define	CAPACITY	40	// 40L = 10G
#define	COND_LENGTH	2	// 2M condensor tube

// condition parameters
#define	AMBIENT		25	// degC
#define	HUMIDITY	70	// percent

// brew parameters
#define VOLUME		20	// 20L = 5G
#define	MALT		6	// Kg
#define	BREW_TEMP	79	// DegC
#define BREW_DELTAT	 2	// DegC
#define BREW_TIME	10	// minutes
#define BREW_COOL	35	// DegC

#define	MAX_WAIT	100	//minutes

int clock = 0;
static char *sts[] = {	// map for status display
	(char *) "ready", (char *) "busy",
	(char *) "high", (char *) "low", (char *) "wide",
	(char *) "fast", (char *) "slow" };

/*
 * wait up to a specified period of time for an operation to complete
 */
void waitfor(Controller *cont, Simulator *simulator, int minutes) {

	for(int m = 0; m < minutes * 60; m++ ) {
		bool ok = cont->checkStatus();
		digitalWrite(10, cont->heating);
		if (!ok)
			break;
		clock += 1;
		simulator->simulate(1);
	}
	digitalWrite(10, 0);	// heater off
	fprintf(stderr,"Operation completed\n");
}

/*
 * run a standard test scenario on a still and brew
 */
void test(Controller *cont, Simulator *simulator) {
	// bring it up to 180
	fprintf(stderr,"HEAT %dC\n", BREW_TEMP);
	cont->setCommand(Controller::heat, degCtoSensor(BREW_TEMP), degCtoSensor(BREW_TEMP));
	waitfor(cont,simulator, MAX_WAIT);

	// hold it between 175 and 185 for 10 minutes
	int hi = BREW_TEMP + BREW_DELTAT;
	int lo = BREW_TEMP;
	fprintf(stderr,"HOLD %dC-%dC, %d minutes\n", lo, hi, BREW_TIME);
	cont->setCommand(Controller::hold, degCtoSensor(lo), degCtoSensor(hi));
	waitfor(cont,simulator, BREW_TIME);

	// take it down to 100
	fprintf(stderr,"COOL %dC\n", BREW_COOL);
	cont->setCommand(Controller::cool, degCtoSensor(BREW_COOL), degCtoSensor(BREW_COOL));
	waitfor(cont,simulator, MAX_WAIT);

	fprintf(stderr,"Test completed\n");
}

/*
 * instantiate the objects and run the simulation
 */
int main(int argc, char **argv) {
	// create an output file
	SD *sd = new SD();
	File *logfile = sd->open((char *) "sensors.csv", FILE_WRITE);

	// instantiate a still, a controller, and a simulation
	Controller *cont = new Controller(sensors, zones, logfile);
	Still *still = new Still(CAPACITY, HEATER, COND_LENGTH);
	Brew *brew = new Brew(VOLUME, MALT);
	Simulator *simulator = new Simulator(still, brew, degCtoSensor(AMBIENT), HUMIDITY);

	// run the test
	test(cont, simulator);

	logfile->close();
}
