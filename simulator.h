/**
 * still simulator for testing the control software
 */

class Simulator {
	
    public:
	Simulator( int watts, int ml, short ambient );
	void heat( bool on );
	short readPin( int pin );
	void simulate( int seconds );

    private:
	// current state
	bool heating;		// heater is on
    	float values[7];	// sensor values
	enum sensorPins {kettle1=1, kettle2, ambient1, ambient2, condensor1, alcohol};

	// simulated still parameters
	int power;		// heater power (watts)
	int ambient;		// air temp (degS)
	float C_heating;	// coil-kettle heating: watts/tick
	float C_cooling;	// kettle-air cooling: watts/tick
	float C_air;		// kettle-air heating: watts/tick
	float C_drain;		// air-air cooling: watts/tick
};
