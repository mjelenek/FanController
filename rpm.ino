#define writeLastFanRpmSensorTimeMacro(i) writeLastFanRpmSensorTime(&lastFanRpmSensorTime[i], fanRpmSensorTimes[i], now);\
  lastFanRpmSensorTimeUpdated[i] = true;

#if HWversion == 1
  #include "ISR_Nano.h"
#endif
#if HWversion == 2
  #include "ISR_Meduino2560.h"
#endif

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(byte *lastFanRpmSensorTime, unsigned long fanRpmSensorTimes[], unsigned long now){
  byte lastFanRpmSensorTimeValue = *lastFanRpmSensorTime;
  lastFanRpmSensorTimeValue++;
  if(lastFanRpmSensorTimeValue >= FAN_RPM_SENSOR_TIMES_FIELD){
    lastFanRpmSensorTimeValue = 0;
  }
  fanRpmSensorTimes[lastFanRpmSensorTimeValue] = now;
  *lastFanRpmSensorTime = lastFanRpmSensorTimeValue;
}

double countRPM(byte lastFanRpmSensorTimeIndex, unsigned long fanRpmSensorTimes[]){
  disableRpmIRS();
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[time1Pointer];
  enableRpmIRS();
  unsigned long now = micros();
  if((now - time0) > 240000 || (now - time1) > 840000){
    return 0;
  }
  return (60000000 / (time0 - time1));
}

// 255, 254, 253, 252, 191, 190 ...(189, 188, 127, 126, 125, 124, 63, 62)
//#define TIME_TO_COMPUTE_RPM(fanNumber) i == (255 - ((fanNumber >> 2) << 6) - (fanNumber & B00000011))
//#define TIME_TO_COMPUTE_RPM(fanNumber) i == (255 - fanNumber)
byte TIME_TO_COMPUTE_RPM[] = {255, 254, 253, 252, 191, 190, 189, 188, 127, 126, 125, 124, 63, 62};
void countRPMs(){
  boolean lastFanRpmSensorTimeUpdatedLocal[NUMBER_OF_FANS];
  disableRpmIRS();
  memcpy(&lastFanRpmSensorTimeUpdatedLocal, &lastFanRpmSensorTimeUpdated, NUMBER_OF_FANS);
  memset(&lastFanRpmSensorTimeUpdated, 0, NUMBER_OF_FANS);
  enableRpmIRS();

  for(byte x = 0; x < NUMBER_OF_FANS; x++){
    if((lastFanRpmSensorTimeUpdatedLocal[x]) || (i == TIME_TO_COMPUTE_RPM[x])){
      rpm[x] = countRPM(lastFanRpmSensorTime[x], fanRpmSensorTimes[x]);
    }
  }
}

