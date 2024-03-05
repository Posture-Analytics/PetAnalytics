#include "IMU.h"
#include "ICM_20948.h" 

bool isrFired[3] = {false, false, false};

int intPin1 = 25;
int intPin2 = 26;
int intPin3 = 34;

// vai ser uma função auxiliar que altera o estado da variável isrFired
// O PROBLEMA TÁ AQUI, QUANDO EU COLOQUEI O ELSE IF ROLOU, MAS NÃO COLETA OS DADOS
void icmISR()
{
  Serial.println("OK");
  // Determina qual instância de IMU disparou a interrupção
  if (!digitalRead(intPin1)) {
    isrFired[0] = true;
  }
  
  if (!digitalRead(intPin2)) {
    isrFired[1] = true;
  } 

  if (!digitalRead(intPin3)) {
    isrFired[2] = true;
  }
}

IMU_20948::IMU_20948(int numberIMU, int pinIMU, int intPinIMU, const char* nameIMU)
{
  IMUPin = pinIMU;
  IMUName = nameIMU;
  IMUIntPin = intPinIMU;
  IMUNumber = numberIMU;

  // vai configurar a interrupção no pino em que está o IMU (assim como no exemplo 3)
  pinMode(IMUIntPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMUIntPin), icmISR, FALLING);

  // construindo o loop de inicialização do IMU
  bool initialized = false;
  while (!initialized)
  {
    // inicializa a comunicação via SPI
    myICM.begin(pinIMU, SPI);

    // verifica o status da comunicação
    Serial.print("Initialization of the sensor '");
    Serial.print(IMUName);
    Serial.print("' returned: ");
    Serial.println(myICM.statusString());

    // verifica se houve algum erro para então tentar novamente
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      Serial.println("Trying again...");
      delay(500);
    }
    else
    {
      Serial.println("Device connected!");
      initialized = true;
    }

    bool success = true;

    // vai inicializar o DMP e retornar o status/resultado
    success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);

    // ativando os sensores
    activate_sensors(&success);

    // ajustando a taxa do ODR para o IMU
    setOdrRate(&success);

    // habilitando o FIFO 
    success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);

    // habilitando o DMP
    success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);

    // resetando o DMP
    success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);

    // resetando a FIFO
    success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

    // Verifica o sucesso das configurações acima
    if (success)
    {
      Serial.println("DMP enabled!");}
    else
    {
      Serial.println("Enable DMP failed!");
      Serial.println("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h...");
      while (1);
    }

    myICM.cfgIntActiveLow(true);  // Active low to be compatible with the breakout board's pullup resistor
    myICM.cfgIntOpenDrain(false); // Push-pull, though open-drain would also work thanks to the pull-up resistors on the breakout
    myICM.cfgIntLatch(false);      // Latch the interrupt until cleared
    myICM.intEnableRawDataReady(true); // enable interrupts on raw data ready

  }
}

void IMU_20948::setOdrRate(bool *success)
{
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok);
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Gyro, 0) == ICM_20948_Stat_Ok);
  *success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Cpass, 0) == ICM_20948_Stat_Ok);
}

void IMU_20948::activate_sensors(bool *success) 
{
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_GYROSCOPE) == ICM_20948_Stat_Ok);
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER) == ICM_20948_Stat_Ok);
  *success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED) == ICM_20948_Stat_Ok);
}

void IMU_20948::readData()
{
  // se a interrupção for ativada, poderemos coletar os dados
  if(isrFired[IMUNumber])
  {
    icm_20948_DMP_data_t data;
    myICM.readDMPdataFromFIFO(&data);

    Serial.print("Dados lidos da IMU ");
    Serial.println(IMUNumber);

    float acc_x = (float)data.Raw_Accel.Data.X;
    float acc_y = (float)data.Raw_Accel.Data.Y;
    float acc_z = (float)data.Raw_Accel.Data.Z;

    float x = (float)data.Raw_Gyro.Data.X;
    float y = (float)data.Raw_Gyro.Data.Y;
    float z = (float)data.Raw_Gyro.Data.Z;

    float comp_x = (float)data.Compass.Data.X;
    float comp_y = (float)data.Compass.Data.Y;
    float comp_z = (float)data.Compass.Data.Z;

    isrFired[IMUNumber] = false;
  }
  else
  {
    Serial.println("DEU RUIM AO COLETAR OS DADOS...");
  }

  myICM.clearInterrupts();
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