/**
 * still simulator for testing the control software
 */

class Simulator {
	
    public:
	Simulator( Still *still, Brew *brew, short ambient, short humidity );
	void heat( int percent );
	short readPin( int pin );
	void simulate( int seconds );

	int heating;		// percent of heating power

    private:
	// current state of system
    	float values[7];	// sensor values
	enum sensorPins {kettle1=1, kettle2, ambient1, ambient2, output1, alcohol};

	// basic system characteristics
	int power;		// available heating power (Watts)
	int T_ambient;		// air temp (degS)

	// calculated specific heats (joules/sensor tick)
	float C_kettle;		// kettle
	float C_condensor;	// condensor tubes
	float C_brew;		// brew
	float C_local;		// immediately surrounding air

	float enthalpy;		// of boiling (j/Kg)

	// conductive/convective heat transfers (watts/delta sensor tick)
	float dQ_ka;		// kettle to air
	float dQ_kc;		// kettle to condensor
	float dQ_ca;		// condensor to surrounding air
	float dQ_ae;		// surrounding cube to environment
};
