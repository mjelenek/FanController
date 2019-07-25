#define TACH0_SET {TACH0_1;} else {TACH0_0;}

#define writeTachMacro(RPMSENSOR, PIN, PIN_NUM)  if(rmpToMainboard(0) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH0_SET;\
  }

#define writeTachMacroState(RPMSENSOR, PIN_NUM)  if(rmpToMainboard(0) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH0_SET;\
  }

#define handleIntMacro(RPMSENSOR, PIN)    if(changed & (1 << PIN)){\
    writeTachMacroState(RPMSENSOR, PIN)\
    writeLastFanRpmSensorTimeMacro(RPMSENSOR);\
  }

// external interrupt on pin PD2
ISR(INT0_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT0, PIND, PIND2)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT0);
}

// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  handleIntMacro(RPMSENSOR_PCINT0, PINB0)

  handleIntMacro(RPMSENSOR_PCINT4, PINB4)

  lastState = state;
}

// change pin PC5
ISR(PCINT1_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINC;
  byte changed = state ^ lastState;

  handleIntMacro(RPMSENSOR_PCINT13, PINC5)

  lastState = state;
}

// change pin PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  handleIntMacro(RPMSENSOR_PCINT20, PIND4)

  handleIntMacro(RPMSENSOR_PCINT23, PIND7)

  lastState = state;
}

