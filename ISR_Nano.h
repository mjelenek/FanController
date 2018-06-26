// external interrupt on pin PD2
ISR(INT0_vect){
  unsigned long now = micros();

  if(rmpToMainboard(0) == 0){
    byte state = PIND;
    if(state & (1 << PIND2)) TACH0_SET;
  }
  writeLastFanRpmSensorTimeMacro(0);
}

// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(rmpToMainboard(0) == 3){
      if(state & (1 << PINB0)) TACH0_SET;
    }
    writeLastFanRpmSensorTimeMacro(3);
  }

  if(changed & (1 << PINB4)){
    if(rmpToMainboard(0) == 4){
      if(state & (1 << PINB4)) TACH0_SET;
    }
    writeLastFanRpmSensorTimeMacro(4);
  }

  lastState = state;
}

// change pin PC5
ISR(PCINT1_vect){
  unsigned long now = micros();
  byte state = PINC & (1 << PINC5);
  if(rmpToMainboard(0) == 5){
    if(state) TACH0_SET;
  }
  writeLastFanRpmSensorTimeMacro(5);
}

// change pin PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND4)){
    if(rmpToMainboard(0) == 1){
      if(state & (1 << PIND4)) TACH0_SET;
    }
    writeLastFanRpmSensorTimeMacro(1);
  }

  if(changed & (1 << PIND7)){
    if(rmpToMainboard(0) == 2){
      if(state & (1 << PIND7)) TACH0_SET;
    }
    writeLastFanRpmSensorTimeMacro(2);
  }

  lastState = state;
}

