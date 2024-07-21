#include "SensorLib.h"
#include "bma400_defs.h"

int16_t Accelxyz[3];
//Arctan from 0 to 45°
uint8_t arctan[]= {0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10, 11, 
                   11, 12, 12, 13, 13, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
                   20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 
                   27, 28, 28, 29, 29, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34,
                   34, 35, 35, 35, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40,
                   40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 44, 45,
                   45};
//SPI Flag for Read or Write 
uint8_t readflag(uint8_t address){return (address | 0x01 << 7);}
// Read the Accelartiondata from Registe
void readAcceldataxyz(int16_t *Accelxyz){
    //Getting the Accelation Value from the Registers
    uint8_t getAccelWritebuffer[]={readflag(BMA400_REG_ACCEL_DATA)};
    uint8_t getAccelReadbuffer[8];
    CS_SENSOR_Clear();
    SERCOM0_SPI_WriteRead(&getAccelWritebuffer[0],8,&getAccelReadbuffer[0],8);
    CS_SENSOR_Set();
    //Formula to Combine the Register values to the total Value as signed 12-bit 
    Accelxyz[0]=getAccelReadbuffer[2]+256*getAccelReadbuffer[3];
    Accelxyz[1]=getAccelReadbuffer[4]+256*getAccelReadbuffer[5];
    Accelxyz[2]=getAccelReadbuffer[6]+256*getAccelReadbuffer[7];
    //Change the number from a signed 12-bit to signed 16-Bit
    for(uint8_t i=0;i<3;i++){
     Accelxyz[i] = (  Accelxyz[i] & 0x800 ?  Accelxyz[i] | 0xf000 :  Accelxyz[i] );
    }
}
   
//Reads Chip Id to initalize SPI and get ChipID as communication test (Warning first Read of Chip ID after Powerup will be invalid)
uint8_t SensorSPIInit(){
    uint8_t InitWritebuffer[3]={readflag(BMA400_REG_CHIP_ID),0x00,0x00};
    uint8_t InitReadbuffer[3];
    CS_SENSOR_Clear();
    SERCOM0_SPI_WriteRead(&InitWritebuffer[0],sizeof(InitWritebuffer),&InitReadbuffer[0],sizeof(InitReadbuffer));
    CS_SENSOR_Set();
    return InitReadbuffer[2];
}
// Setting Accelconfig within the Sensor 
bool SensorConfig(){
    uint8_t Configbuffer[]={BMA400_REG_ACCEL_CONFIG_0,0xE2,BMA400_REG_ACCEL_CONFIG_1,0x38,BMA400_REG_ACCEL_CONFIG_2,0x04};
    if(SensorSPIInit()==0x90){
    CS_SENSOR_Clear();
    SERCOM0_SPI_Write(&Configbuffer[0],sizeof(Configbuffer));
    CS_SENSOR_Set();
    return false;
    }
    else{return true;}
}
// (Input Argument = Output Array use global var orientation is boolean neg = true angle , pos = false)
Angle getAnglexy(){
    Angle a;
    readAcceldataxyz(Accelxyz);
    for(uint8_t i=0;i!=2;i++){
    //Check if value is negativ 12 bit value and convert to 16 bit int
        Accelxyz[i] = (  Accelxyz[i] & 0x800 ?  Accelxyz[i] | 0xf000 :  Accelxyz[i] );
        a.Anglexy[i]= Accelxyz[i]*100/Accelxyz[2];
        if(a.Anglexy[i]<0){
            a.Anglexy[i]=a.Anglexy[i]*(-1);
            a.orientation[i]=1;
        }
        else{a.orientation[i]=0;}
        if(a.Anglexy[i]>100){
            a.Anglexy[i]=45;
        }
        else{
            a.Anglexy[i]=arctan[a.Anglexy[i]];
        }
    }
    return a;
    
}