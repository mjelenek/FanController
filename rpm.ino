// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(*rmpToMainboard == 3){
      if(state & (1 << PINB0)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime3, fanRpmSensorTimes3, now);
    lastFanRpmSensorTimeUpdated |= B00001000;
  }

  if(changed & (1 << PINB4)){
    if(*rmpToMainboard == 4){
      if(state & (1 << PINB4)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime4, fanRpmSensorTimes4, now);
    lastFanRpmSensorTimeUpdated |= B00010000;
  }

  lastState = state;
}

// change pin PC5
ISR(PCINT1_vect){
  unsigned long now = micros();
  byte state = PINC & (1 << PINC5);
  if(*rmpToMainboard == 5){
    if(state) LED_OUT_SET;
  }
  writeLastFanRpmSensorTime(&lastFanRpmSensorTime5, fanRpmSensorTimes5,  now);
  lastFanRpmSensorTimeUpdated |= B00100000;
}

// change pin PD2, PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND2)){
    if(*rmpToMainboard == 0){
      if(state & (1 << PIND2)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime0, fanRpmSensorTimes0, now);
    lastFanRpmSensorTimeUpdated |= B00000001;
  }

  if(changed & (1 << PIND4)){
    if(*rmpToMainboard == 1){
      if(state & (1 << PIND4)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime1, fanRpmSensorTimes1, now);
    lastFanRpmSensorTimeUpdated |= B00000010;
  }

  if(changed & (1 << PIND7)){
    if(*rmpToMainboard == 2){
      if(state & (1 << PIND7)) LED_OUT_SET;
    }
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime2, fanRpmSensorTimes2, now);
    lastFanRpmSensorTimeUpdated |= B00000100;
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

void countRPMs(){
  PCICR = 0;                            // disable pin change interrupts
  byte lastFanRpmSensorTimeUpdatedLocal = lastFanRpmSensorTimeUpdated;
  lastFanRpmSensorTimeUpdated = 0;
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts

  if(((lastFanRpmSensorTimeUpdatedLocal & B00000001) > 0) || (i == (254 - 64))){
    rpm[0] = countRPM(lastFanRpmSensorTime0, fanRpmSensorTimes0);
//    pidUpdateDirect(0, *ConfigurationPWM(0]);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00000010) > 0) || (i == (255 - 64))){
    rpm[1] = countRPM(lastFanRpmSensorTime1, fanRpmSensorTimes1);
//    pidUpdateDirect(1, *ConfigurationPWM(1]);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00000100) > 0) || (i == (254 - 32))){
    rpm[2] = countRPM(lastFanRpmSensorTime2, fanRpmSensorTimes2);
//    pidUpdateDirect(2, *ConfigurationPWM(2]);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00001000) > 0) || (i == (255 - 32))){
    rpm[3] = countRPM(lastFanRpmSensorTime3, fanRpmSensorTimes3);
//    pidUpdateDirect(3, *ConfigurationPWM(3]);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00010000) > 0) || (i == 254)){
    rpm[4] = countRPM(lastFanRpmSensorTime4, fanRpmSensorTimes4);
//    pidUpdateDirect(4, *ConfigurationPWM(4]);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00100000) > 0) || (i == 255)){
    rpm[5] = countRPM(lastFanRpmSensorTime5, fanRpmSensorTimes5);
//    pidUpdateDirect(5, *ConfigurationPWM(5]);
  }
}

