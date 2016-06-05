/*
 * sensor unit conversion methods
 */

/**
 * convert Fahrenheit temperature to sensor reading
 *
 * @param Fahrenheit temperature
 * @return sensor units
 */
short degFtoSensor( short degF );

/**
 * convert temperature sensor reading to Fahrenheit
 *
 * @param sensor units
 * @return Fahrenheit temperature
 */
short sensorToDegF( short sensor );

/**
 * convert Centegrade temperature to sensor reading
 *
 * @param Centegrade temperature
 * @return sensor units
 */
short degCtoSensor( short degC );

/**
 * convert temperature sensor reading to Centegrade
 *
 * @param sensor units
 * @return Centegrade temperature
 */
short sensorToDegC( short sensor );

/**
 * convert alcohol % temperature to sensor reading
 *
 * @param %alcohol (in tenths)
 * @return sensor units
 */
short alc10toSensor( short alc10 );

/**
 * convert alcohol sensor reading to %/10
 *
 * @param sensor units
 * @return alcohol percentage (in tenths %)
 */
short sensorToAlc10( short sensor );
