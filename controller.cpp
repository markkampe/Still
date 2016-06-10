#include "Arduino.h"
#include "SD.h"
#include "controller.h"

/**
 * This class implements the still controller.
 *
 * This module should be able to build both on Linux (for s/w testing)
 * and on an Ardunio.  Hence it uses limited space and libraries.
 */

/**
 * constructor
 *	@param pointer to sensor configuration
 *	@param pointer to zone configuration
 */
Controller::Controller( struct sensor_cfg *s, struct zone_cfg *z, File *logfile) {
	// remember the configuration information
	sensors = s;
	zones = z;

	// count the zones
	numzone = 0;
	for( int i = 0; s[i].pin != 0; i++ ) {
		if (s[i].zone >= numzone)
			numzone = s[i].zone + 1;
		pinMode(s[i].pin, INPUT);
	}

	// allocate the zone value array
	curReading = (unsigned short *) malloc(numzone);
	curStatus = ready;

	// and start logging
	log = logfile;
	if (log) {
		logHeader();
	}
	
}

/**
 * main polling/action loop for still status/control
 *
 *	gather and average the sensor values
 *	sanity check max/min values and intra-zone variation
 *	decide whether or not the heater should be on
 *
 * @return	true ... still executing this command
 *		false ... this command has completed
 */
bool Controller::checkStatus() {

#define SENSOR_MINVAL 0
#define SENSOR_MAXVAL 32767
	// get and sanity check the zone values
	for( int z = 0; z < numzone; z++) {
		short min = SENSOR_MAXVAL;
		short max = SENSOR_MINVAL;
		for( int i = 0; sensors[i].pin != 0; i++ ) {
			if (sensors[i].zone != z)
				continue;
			short v = analogRead(sensors[i].pin);
			if (v > max)
				max = v;
			if (v < min)
				min = v;
		}
		if (curStatus == busy) {
			if (min < zones[z].minVal) {
				curStatus = tooLow;
				heating = false;
			}
			if (max > zones[z].maxVal) {
				curStatus = tooHigh;
				heating = false;
			}
			if (max - min > zones[z].maxDelta) {
				curStatus = tooWide;
				heating = false;
			}
		} 
		curReading[z] = (min + max)/2;
	}

	if (log)
		logSensors();

	if (curStatus != busy)
		return( false );

	// if we're busy, we have to manage the heat
	if (curComand == heat) {
		if (curReading[0] >= minTarget) {
			curStatus = Controller::ready;
			return( false );
		} else {
			heating = 100;
			return( true );
		}
		// FIX: sanity check the cooling rate
	} else if (curComand == cool) {
		heating = 0;
		if (curReading[0] <= maxTarget) {
			curStatus = Controller::ready;
			return( false );
		}
		// FIX: sanity check the cooling rate
		return(true);
	} else if (curComand == hold) {
		if (curReading[0] < minTarget)
			heating = 50;
		else
			heating = 0;
		return( true );
	}

	// we apparently aren't doing anything
	return( false);
}


/**
 * reset the still to ready state
 */
void Controller::reset() {
	curStatus = ready;
	heating = false;
}

/**
 * load a new command into the controller (if it is ready)
 *
 * @param cmd (heat, cool, hold, none)
 * @param min (low temp limit, in sensor units)
 * @param max (hi temp limit, in sensor units)
 */
void Controller::setCommand( enum command cmd, short min, short max ) {
	if (curStatus == ready || curStatus == busy) {
		curComand = cmd;
		minTarget = min;
		maxTarget = max;
		curStatus = busy;
	}
}

/**
 * write out a log file header
 */
void Controller::logHeader() {
	log->println((char *) "# time(hhmmss),status,power,z[0],z[1],...");
}

/**
 * number to ascii conversion
 *
 * @param pointer to buffer for characters
 * @param value to be converted
 * @param highest power of ten to start with
 * @param character for leading padding
 *
 * @return next free position in buffer
 */
char *outInt(char *s, int value, int position, char pad) {

	// start with leading zeroes
	while(position > 1 && value/position == 0) {
		if (pad != 0)
			*s++ =  pad;
		value %= position;
		position /= 10;
		
	}

	// then the significant digits
	while(position > 0) {
		int v = value/position;
		*s++ = '0' + value/position;
		value %= position;
		position /= 10;
	}

	return(s);
}

/**
 * write out a sensor log record
 */
void Controller::logSensors() {
	char outbuf[80];
	char *s = outbuf;
	
	// log the time
	long now = millis()/1000;
	int hr = now/3600;
	int min = (now % 3600)/60;
	int sec = now%60;
	s = outInt(s, hr, 10, '0');
	s = outInt(s, min, 10, '0');
	s = outInt(s, sec, 10, '0');
	*s++ = ',';

	// log the status and heating power
	s = outInt(s, curStatus, 1, 0);
	*s++ = ',';
	s = outInt(s, heating, 100, 0);

	// log the sensors
	for( int i = 0; i < numzone; i++) {
		*s++ = ',';
		s = outInt(s, curReading[i], 1000, 0);
	}
	*s++ = 0;

	log->println(outbuf);
}
