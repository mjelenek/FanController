#define fanSensor5FilterDefinition B0000011

// overflow on timer1 interrupt handler
ISR(TIMER1_OVF_vect){
  static byte lastState;
  static byte fanSensor5Filter = 0;
 // while (TCNT2 < 250){}
  byte fanSensor5 = (PINC >> 5) & 1;
  unsigned long now = micros();
  fanSensor5Filter = ((fanSensor5Filter << 1) | fanSensor5) & fanSensor5FilterDefinition;
  byte fanSensor5Value = lastState;
  if(fanSensor5Filter == 0){
    fanSensor5Value = 0;
  }
  if(fanSensor5Filter == fanSensor5FilterDefinition){
    fanSensor5Value = 1;
  }

  if(lastState ^ fanSensor5Value){
    if(rmpToMainboard == 5){
      digitalWrite(LED_OUT, fanSensor5Value);
    }
    if(fanSensor5Value){
      writeLastFanRpmSensorTime(5, now);
    }
  }
  lastState = fanSensor5Value;
}

// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(rmpToMainboard == 1){
      digitalWrite(LED_OUT, state & (1 << PINB0));
    }
    if(state & (1 << PINB0)){
      writeLastFanRpmSensorTime(1, now);
    }
  }

  if(changed & (1 << PINB4)){
    if(rmpToMainboard == 4){
      digitalWrite(LED_OUT, state & (1 << PINB4));
    }
    if(state & (1 << PINB4)){
      writeLastFanRpmSensorTime(4, now);
    }
  }

  lastState = state;
}

// change pin PD2, PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND2)){
    if(rmpToMainboard == 2){
      digitalWrite(LED_OUT, state & (1 << PIND2));
    }
    if(state & (1 << PIND2)){
      writeLastFanRpmSensorTime(2, now);
    }
  }

  if(changed & (1 << PIND4)){
    if(rmpToMainboard == 3){
      digitalWrite(LED_OUT, state & (1 << PIND4));
    }
    if(state & (1 << PIND4)){
      writeLastFanRpmSensorTime(3, now);
    }
  }

  if(changed & (1 << PIND7)){
    if(rmpToMainboard == 0){
      digitalWrite(LED_OUT, state & (1 << PIND7));
    }
    if(state & (1 << PIND7)){
      writeLastFanRpmSensorTime(0, now);
    }
  }

  lastState = state;
}

void writeLastFanRpmSensorTime(byte fanNumber, unsigned long now){
  lastFanRpmSensorTime[fanNumber]++;
  if(lastFanRpmSensorTime[fanNumber] >= FAN_RPM_SENSOR_TIMES_FIELD){
    lastFanRpmSensorTime[fanNumber] = 0;
  }
  fanRpmSensorTimes[fanNumber][lastFanRpmSensorTime[fanNumber]] = now;
}

unsigned int countRPM(byte fanNumber){
  unsigned int rpm = 0;
  byte time1Pointer = lastFanRpmSensorTime[fanNumber] + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  PCICR = 0;  // disable pin change interrupts
  unsigned long time0 = fanRpmSensorTimes[fanNumber][lastFanRpmSensorTime[fanNumber]];
  unsigned long time1 = fanRpmSensorTimes[fanNumber][time1Pointer];
  PCICR = (1 << PCIE0) | (1 << PCIE2);  // enable pin change interrupts
  if((micros() - time0) > 240000 || (micros() - time1) > 840000){
    return 0;
  }
  rpm = 60000000 / (time0 - time1);
  return rpm;
}

void countRPMs(){
  rpm0 = countRPM(0);
  rpm1 = countRPM(1);
  rpm2 = countRPM(2);
  rpm3 = countRPM(3);
  rpm4 = countRPM(4);
  rpm5 = countRPM(5);
}

