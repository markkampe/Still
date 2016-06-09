/**
* configuration/calibration info for sensors
*	pin=0 indicates last entry
*	calibrated = (raw * scale) + offset
*/
struct sensor_cfg {
	unsigned char pin;	// analog signal pin
	unsigned char zone;	// zone number
};

/**
 * sanity checking for zones
 */
struct zone_cfg {
	short minVal;		// min safe reading
	short maxVal;		// max safe reading
	short maxDelta;		// max acceptable discrepancy
};

class Controller {
   public:
	int numzone;		// number of zones
   	unsigned short *curReading;	// current sensor readings

	enum status { 	ready=0, busy,
		tooHigh, tooLow, tooWide,
		tooFast, tooSlow,
	};
	unsigned char curStatus;	// current still status

	enum command { stop=0, heat, cool, hold };
	unsigned char curComand;	// current command
	short minTarget;		// low target temp
	short maxTarget;		// high target temp

	unsigned char heating;		// heat should be on

	/**
	 * constructor
	 * @param sensor configuration
	 * @param zone configuration
	 */
	Controller( struct sensor_cfg *, struct zone_cfg *);

	/**
	 * check status of sensors, determine what to do next
	 * @return true ... we are still executing last command
	 */
	bool checkStatus();

	/**
	 * load a new command into the controller
	 */
	void setCommand(enum command cmd, short min, short max);

	/**
	 * reset the error status of the controller
	 */
	void reset();

    private:
	struct sensor_cfg *sensors;
	struct zone_cfg *zones;
	unsigned short time0;	// time at start of heat/cool
	short temp0;		// temp at start of heat/cool
};
