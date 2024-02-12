#include "Arduino.h"
#include "Wire.h"

#define LED LED_BUILTIN

#define MPU 0x68
#define PWR_MGMT 0x6B
#define SMPLRT_DIV 0x19
#define GYRO_CNFG 0x1B

float gyro_raw;
float gyro_cal; // calibration value

void gyroOffsett();

void setup(){

  pinMode(LED, 1);

  Serial.begin(115200);
  Wire.begin();

  Wire.beginTransmission(MPU);  // PWR_MGMT register
  Wire.write(PWR_MGMT);
  Wire.write(0x00);
  Wire.endTransmission(1);

  Wire.beginTransmission(MPU); // GYRO_CNFG register
  Wire.write(GYRO_CNFG);
  Wire.write(0x10);
  Wire.endTransmission(1);

  gyroOffsett();
}

uint32_t prev_time;
uint32_t current_time = 0;
float elapsed_time;

float gyro_x;
float angle_x;

void loop(){
    Wire.beginTransmission(MPU);
    Wire.write(0x47);
    Wire.endTransmission(0);
    Wire.requestFrom(MPU, 2, 1);

    gyro_x = ((Wire.read() << 8 | Wire.read()) - gyro_cal)/32.8;

    current_time = micros();
    elapsed_time = (current_time - prev_time) / 1000000.0; // microseconds
    prev_time = current_time;
    
    angle_x += gyro_x * elapsed_time;

    Serial.println(angle_x);

    delay(100);
}

void gyroOffsett(){
  for(uint16_t x = 0; x < 2000; x++){
    Wire.beginTransmission(MPU);
    Wire.write(0x47);
    Wire.endTransmission(0);
    Wire.requestFrom(MPU, 2, 1);

    gyro_raw = (Wire.read() << 8 | Wire.read());

    gyro_cal += gyro_raw;

    delay(3);
  }

  gyro_cal /= 2000;
  digitalWrite(LED, 1);
  delay(300);
  digitalWrite(LED, 0);
  delay(300);
}