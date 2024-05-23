#include "IMU.h"
#include "ICM_20948.h"

IMU::IMU(int number, int CSpin, const char* name):
  IMUnumber(number), IMUpin(CSpin), IMUname(name) {}

bool IMU::init(SPIClass &spi) {
  // Continues attempting initialization until successful
  while (!initialized) {
    // Initializes SPI communication
    myICM.begin(IMUpin, spi);

    // Checks the status of the communication and prints it
    Serial.print("Initializing sensor '");
    Serial.print(IMUname);
    Serial.print(" - ");
    Serial.print(IMUnumber);
    Serial.print(" (CS pin: ");
    Serial.print(IMUpin);
    Serial.println(")'...");

    // If there's an error, retry after a brief pause
    if (myICM.status != ICM_20948_Stat_Ok) {
      Serial.println("Retry initialization...");
      delay(500);
    } else {
      Serial.println("Sensor connected successfully.");
      initialized = true;
    }

    // Initialize the Digital Motion Processor (DMP) and check the result
    bool success = (myICM.initializeDMP() == ICM_20948_Stat_Ok);

    // Sensor activation, ODR adjustment, FIFO, and DMP configurations
    activateSensors(&success);
    setOdrRate(&success);
    success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

    // Final check to ensure all configurations were successful
    if (success) {
      Serial.println("DMP enabled and configured successfully.");
    } else {
      Serial.println("DMPenabled and configuration failed.");
      Serial.println("Ensure line 29 (`#define ICM_20948_USE_DMP`) in `ICM_20948_C.h` is uncommented...");
      while (1); // Halt on failure
    }
  }
  return initialized;
}

char* IMU::readData(char* dataString) {
  // Collects and prints data if a new dataset is available
  icm_20948_DMP_data_t DMPdata;
  myICM.readDMPdataFromFIFO(&DMPdata);

  strcpy(dataString, "erro");

  if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) {
    Serial.print("IMU Data Read - Sensor ID: ");
    Serial.println(IMUnumber);

    // Extract and store the acceleration, gyroscope, and magnetometer data
    accelData[0] = DMPdata.Raw_Accel.Data.X;
    accelData[1] = DMPdata.Raw_Accel.Data.Y;
    accelData[2] = DMPdata.Raw_Accel.Data.Z;

    gyroData[0] = DMPdata.Raw_Gyro.Data.X;
    gyroData[1] = DMPdata.Raw_Gyro.Data.Y;
    gyroData[2] = DMPdata.Raw_Gyro.Data.Z;

    magData[0] = DMPdata.Compass.Data.X;
    magData[1] = DMPdata.Compass.Data.Y;
    magData[2] = DMPdata.Compass.Data.Z;

    snprintf(dataString, 350, "1; NONE; %d; %d; %d; %d; %d; %d; %d; %d; %d\n",
      accelData[0], accelData[1], accelData[2],
      gyroData[0], gyroData[1], gyroData[2],
      magData[0], magData[1], magData[2]);
  }

  // Print the data to Serial
  //Serial.println(dataString);

  return dataString;
}

void IMU::setOdrRate(bool *success) {
  // Sets the Output Data Rate (ODR) for accelerometer, gyroscope, and compass
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok);
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Gyro, 0) == ICM_20948_Stat_Ok);
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Cpass, 0) == ICM_20948_Stat_Ok);
}

void IMU::activateSensors(bool *success) {
  // Enables the necessary sensors on the IMU for data collection
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_GYROSCOPE) == ICM_20948_Stat_Ok);
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER) == ICM_20948_Stat_Ok);
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED) == ICM_20948_Stat_Ok);
}

ICM_20948_Status_e ICM_20948::initializeDMP(void)
{
  ICM_20948_Status_e  result = ICM_20948_Stat_Ok; 
  ICM_20948_Status_e  worstResult = ICM_20948_Stat_Ok;

  result = i2cControllerConfigurePeripheral(0, MAG_AK09916_I2C_ADDR, AK09916_REG_RSV2, 10, true, true, false, true, true); if (result > worstResult) worstResult = result;
  
  result = i2cControllerConfigurePeripheral(1, MAG_AK09916_I2C_ADDR, AK09916_REG_CNTL2, 1, false, true, false, false, false, AK09916_mode_single); if (result > worstResult) worstResult = result;

  uint8_t mstODRconfig = 0x04; 
  result = write(AGB3_REG_I2C_MST_ODR_CONFIG, &mstODRconfig, 1); if (result > worstResult) worstResult = result;

  result = setClockSource(ICM_20948_Clock_Auto); if (result > worstResult) worstResult = result; 

  result = setBank(0); if (result > worstResult) worstResult = result;                              
  uint8_t pwrMgmt2 = 0x40;                                                    
  result = write(AGB0_REG_PWR_MGMT_2, &pwrMgmt2, 1); if (result > worstResult) worstResult = result; 

  result = setSampleMode(ICM_20948_Internal_Mst, ICM_20948_Sample_Mode_Cycled); if (result > worstResult) worstResult = result;

  result = enableFIFO(false); if (result > worstResult) worstResult = result;

  result = enableDMP(false); if (result > worstResult) worstResult = result;

  ICM_20948_fss_t myFSS;
  myFSS.a = gpm4;
  myFSS.g = dps2000;     
  result = setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS); if (result > worstResult) worstResult = result;

  result = enableDLPF(ICM_20948_Internal_Gyr, true); if (result > worstResult) worstResult = result;

  result = setBank(0); if (result > worstResult) worstResult = result; 
  uint8_t zero = 0;
  result = write(AGB0_REG_FIFO_EN_1, &zero, 1); if (result > worstResult) worstResult = result;

  result = write(AGB0_REG_FIFO_EN_2, &zero, 1); if (result > worstResult) worstResult = result;

  result = intEnableRawDataReady(false); if (result > worstResult) worstResult = result;

  result = resetFIFO(); if (result > worstResult) worstResult = result;

  ICM_20948_smplrt_t mySmplrt;
  mySmplrt.g = 10;
  mySmplrt.a = 10;
  
  result = setSampleRate((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt); if (result > worstResult) worstResult = result;

  result = setDMPstartAddress(); if (result > worstResult) worstResult = result;

  result = loadDMPFirmware(); if (result > worstResult) worstResult = result;

  result = setDMPstartAddress(); if (result > worstResult) worstResult = result;

  result = setBank(0); if (result > worstResult) worstResult = result;
  uint8_t fix = 0x48;
  result = write(AGB0_REG_HW_FIX_DISABLE, &fix, 1); if (result > worstResult) worstResult = result;

  result = setBank(0); if (result > worstResult) worstResult = result;
  uint8_t fifoPrio = 0xE4;
  result = write(AGB0_REG_SINGLE_FIFO_PRIORITY_SEL, &fifoPrio, 1); if (result > worstResult) worstResult = result;

  const unsigned char accScale[4] = {0x04, 0x00, 0x00, 0x00};
  result = writeDMPmems(ACC_SCALE, 4, &accScale[0]); if (result > worstResult) worstResult = result;

  const unsigned char accScale2[4] = {0x00, 0x04, 0x00, 0x00};
  result = writeDMPmems(ACC_SCALE2, 4, &accScale2[0]); if (result > worstResult) worstResult = result;

  const unsigned char mountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
  const unsigned char mountMultiplierPlus[4] = {0x09, 0x99, 0x99, 0x99};
  const unsigned char mountMultiplierMinus[4] = {0xF6, 0x66, 0x66, 0x67};
  result = writeDMPmems(CPASS_MTX_00, 4, &mountMultiplierPlus[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_01, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_02, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_10, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_11, 4, &mountMultiplierMinus[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_12, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_20, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_21, 4, &mountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(CPASS_MTX_22, 4, &mountMultiplierMinus[0]); if (result > worstResult) worstResult = result;

  const unsigned char b2sMountMultiplierZero[4] = {0x00, 0x00, 0x00, 0x00};
  const unsigned char b2sMountMultiplierPlus[4] = {0x40, 0x00, 0x00, 0x00};
  result = writeDMPmems(B2S_MTX_00, 4, &b2sMountMultiplierPlus[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_01, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_02, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_10, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_11, 4, &b2sMountMultiplierPlus[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_12, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_20, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_21, 4, &b2sMountMultiplierZero[0]); if (result > worstResult) worstResult = result;
  result = writeDMPmems(B2S_MTX_22, 4, &b2sMountMultiplierPlus[0]); if (result > worstResult) worstResult = result;

  result = setGyroSF(4, 3); if (result > worstResult) worstResult = result;

  const unsigned char gyroFullScale[4] = {0x10, 0x00, 0x00, 0x00};
  result = writeDMPmems(GYRO_FULLSCALE, 4, &gyroFullScale[0]); if (result > worstResult) worstResult = result;

  const unsigned char accelOnlyGain[4] = {0x00, 0xE8, 0xBA, 0x2E};
  result = writeDMPmems(ACCEL_ONLY_GAIN, 4, &accelOnlyGain[0]); if (result > worstResult) worstResult = result;

  const unsigned char accelAlphaVar[4] = {0x3D, 0x27, 0xD2, 0x7D};
  result = writeDMPmems(ACCEL_ALPHA_VAR, 4, &accelAlphaVar[0]); if (result > worstResult) worstResult = result;

  const unsigned char accelAVar[4] = {0x02, 0xD8, 0x2D, 0x83};
  result = writeDMPmems(ACCEL_A_VAR, 4, &accelAVar[0]); if (result > worstResult) worstResult = result;

  const unsigned char accelCalRate[4] = {0x00, 0x00};
  result = writeDMPmems(ACCEL_CAL_RATE, 2, &accelCalRate[0]); if (result > worstResult) worstResult = result;

  const unsigned char compassRate[2] = {0x00, 0x45};
  result = writeDMPmems(CPASS_TIME_BUFFER, 2, &compassRate[0]); if (result > worstResult) worstResult = result;

  return worstResult;
}