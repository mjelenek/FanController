// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(rmpToMainboard == 3){
      if(state & (1 << PINB0)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime[3], fanRpmSensorTimes[3], now);
    lastFanRpmSensorTimeUpdated[3] = true;
  }

  if(changed & (1 << PINB4)){
    if(rmpToMainboard == 4){
      if(state & (1 << PINB4)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime[4], fanRpmSensorTimes[4], now);
    lastFanRpmSensorTimeUpdated[4] = true;
  }

  lastState = state;
}

// change pin PC5
ISR(PCINT1_vect){
  unsigned long now = micros();
  byte state = PINC & (1 << PINC5);
  if(rmpToMainboard == 5){
    if(state) LED_OUT_SET;
  }
  writeLastFanRpmSensorTime(&lastFanRpmSensorTime[5], fanRpmSensorTimes[5],  now);
  lastFanRpmSensorTimeUpdated[5] = true;
}

// change pin PD2, PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND2)){
    if(rmpToMainboard == 0){
      if(state & (1 << PIND2)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime[0], fanRpmSensorTimes[0], now);
    lastFanRpmSensorTimeUpdated[0] = true;
  }

  if(changed & (1 << PIND4)){
    if(rmpToMainboard == 1){
      if(state & (1 << PIND4)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime[1], fanRpmSensorTimes[1], now);
    lastFanRpmSensorTimeUpdated[1] = true;
  }

  if(changed & (1 << PIND7)){
    if(rmpToMainboard == 2){
      if(state & (1 << PIND7)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime[2], fanRpmSensorTimes[2], now);
    lastFanRpmSensorTimeUpdated[2] = true;
  }

  lastState = state;
}

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
  PCICR = 0;                            // disable pin change interrupts
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[time1Pointer];
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
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
  PCICR = 0;                            // disable pin change interrupts
  memcpy(&lastFanRpmSensorTimeUpdatedLocal, &lastFanRpmSensorTimeUpdated, NUMBER_OF_FANS);
  memset(&lastFanRpmSensorTimeUpdated, 0, NUMBER_OF_FANS);
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts

  for(byte x = 0; x < NUMBER_OF_FANS; x++){
    if((lastFanRpmSensorTimeUpdatedLocal[x]) || (i == TIME_TO_COMPUTE_RPM[x])){
      rpm[x] = countRPM(lastFanRpmSensorTime[x], fanRpmSensorTimes[x]);
    }
  }
}

