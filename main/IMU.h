#include "ICM_20948.h" 

class IMU_20948 {

  public:
    ICM_20948_SPI myICM;
    int IMUNumber;
    int IMUPin;
    int IMUIntPin;
    const char* IMUName;
    IMU_20948(int numberIMU, int pinIMU, int intPinIMU, const char* nameIMU);
    void readData();

  private:
    void activate_sensors(bool *success);
    void setOdrRate(bool *success);
};