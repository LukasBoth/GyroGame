// Prototyp function for Sensor

// Value of Angle and postiv an negativ
    typedef
    struct 
    {
        int16_t Anglexy[2];
        uint8_t orientation[2];
    } Angle;
  //Returns absolut value of Angle and a neg and pos bool for orientation
Angle getAnglexy();
//Returns 0 if successful
bool SensorConfig();
//Reads Chip ID
uint8_t SensorSPIInit();
//Read Sensor data in 2G Range 
void readAcceldataxyz(int16_t Accelxyz[]);
// Set Read bit to read
uint8_t readflag(uint8_t address);



