#define DEBUG 1
#ifdef DEBUG
#include <stdlib.h>
#endif

#include <stdio.h>
#include "Arduino.h"
#include "still.h"

/**
 * constructor
 *	@param pointer to sensor configuration
 *	@param pointer to zone configuration
 */
Still::Still( struct sensor_cfg *s, struct zone_cfg *z) {
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
bool Still::checkStatus() {

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
#ifdef DEBUG
				printf("LOW TEMP - zone %d: %d < %d\n", z, min, zones[z].minVal);
#endif
			}
			if (max > zones[z].maxVal) {
				curStatus = tooHigh;
				heating = false;
#ifdef DEBUG
				printf("HIGH TEMP - zone %d: %d > %d\n", z, max, zones[z].maxVal);
#endif
			}
			if (max - min > zones[z].maxDelta) {
				curStatus = tooWide;
				heating = false;
#ifdef DEBUG
				printf("DISCREPANCY - zone %d: %d - %d\n", z, min, max);
#endif
			}
		} 
		curReading[z] = (min + max)/2;
	}

	if (curStatus != busy)
		return( false );

	// if we're busy, we have to manage the heat
	if (curComand == heat) {
		if (curReading[0] >= minTarget) {
			heating = false;
			curStatus = Still::ready;
			return( false );
		} else {
			heating = true;
			return( true );
		}
		// FIX: sanity check the cooling rate
	} else if (curComand == cool) {
		if (curReading[0] <= maxTarget) {
			heating = false;
			curStatus = Still::ready;
			return( false );
		}
		// FIX: sanity check the cooling rate
		return(true);
	} else if (curComand == hold) {
		if (curReading[0] < minTarget)
			heating = true;
		else
			heating = false;
		return( true );
	}

	// we apparently aren't doing anything
	return( false);
}


/**
 * reset the still to ready state
 */
void Still::reset() {
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
void Still::setCommand( enum command cmd, short min, short max ) {
	if (curStatus == ready || curStatus == busy) {
		curComand = cmd;
		minTarget = min;
		maxTarget = max;
		curStatus = busy;
	}
}
