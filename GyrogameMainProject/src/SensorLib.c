#include "SensorLib.h"
#include "bma400_defs.h"


uint8_t readflag(uint8_t address){return (address | 0x01 << 7);}

// Read the Accelartiondata from Registe
void readAcceldataxyz(int16_t *Accelxyz){
    uint8_t getAccelWritebuffer[]={readflag(BMA400_REG_ACCEL_DATA)};
    uint8_t getAccelReadbuffer[8];
    CS_SENSOR_Clear();
    SERCOM0_SPI_WriteRead(&getAccelWritebuffer[0],8,&getAccelReadbuffer[0],8);
    CS_SENSOR_Set();
    Accelxyz[0]=getAccelReadbuffer[2]+256*getAccelReadbuffer[3];
    Accelxyz[1]=getAccelReadbuffer[4]+256*getAccelReadbuffer[5];
    Accelxyz[2]=getAccelReadbuffer[6]+256*getAccelReadbuffer[7];
    for(uint8_t i=0;i!=3;i++){
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
    readAcceldataxyz();
    for(uint8_t i=0;i!=2;i++){
    //Check if value is negativ 12 bit value and convert to 16 bit in
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