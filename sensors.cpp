/*
 * sensor unit conversion methods
 */

/**
 * convert Fahrenheit temperature to sensor reading
 *
 * @param Fahrenheit temperature
 * @return sensor units
 */
short degFtoSensor( short degF ) {
	return(4 * degF);
}

/**
 * convert temperature sensor reading to Fahrenheit
 *
 * @param sensor units
 * @return Fahrenheit temperature
 */
short sensorToDegF( short sensor ) {
	return( sensor / 4 );
}

/**
 * convert Centegrade temperature to sensor reading
 *
 * @param Centegrade temperature
 * @return sensor units
 */
short degCtoSensor( short degC ) {
	return(((degC * 36)/5) + 128);
}

/**
 * convert temperature sensor reading to Centegrade
 *
 * @param sensor units
 * @return Centegrade temperature
 */
short sensorToDegC( short sensor ) {
	return (((sensor - 128) * 5)/36);
}

/**
 * convert alcohol % temperature to sensor reading
 *
 * @param %alcohol (in tenths)
 * @return sensor units
 */
short alc10toSensor( short alc10 ) {
	return alc10;
}

/**
 * convert alcohol sensor reading to %/10
 *
 * @param sensor units
 * @return alcohol percentage (in tenths %)
 */
short sensorToAlc10( short sensor ) {
	return sensor;
}

