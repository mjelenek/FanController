#define NUMBER_OF_THERMISTORS 2
#define NUMBER_OF_FANS 6
#define NUMBER_OF_MAINBOARD_CONNECTORS 5
#define NUMBER_OF_RPM_TO_MAINBOARD 1

#define CURVE_ANALOG_POINTS 4
#define CURVE_PWM_POINTS 5
#define CURVE_RPM_POINTS 5

// size of temperatures cache. Size is 2^CACHE_T_SIZE - value 6 means 64 records ~ 6°C
#define CACHE_T_SIZE 6
// size of PWM by temperature or PRM by temperature cache. Size is 2^CACHE_PWM_SIZE - value 2 means 4 records
#define CACHE_PWM_SIZE 2

//by multimeter
//#define ANALOGREFERENCEVOLTAGE 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
const unsigned long PROGMEM RT_PGM[NUMBER_OF_THERMISTORS] ={9990, 9990};

#define RPMSENSOR0 7
#define RPMSENSOR1 8
#define RPMSENSOR2 2
#define RPMSENSOR3 4
#define RPMSENSOR4 12
#define RPMSENSOR5 19   // A5

//define input pin int -> rpm
#define RPMSENSOR_INT0 0
#define RPMSENSOR_PCINT0 3
#define RPMSENSOR_PCINT4 4
#define RPMSENSOR_PCINT13 5
#define RPMSENSOR_PCINT20 1
#define RPMSENSOR_PCINT23 2

//PWM output pins
#define PWM0 3  //OC2B
#define PWM1 5  //OC0B
#define PWM2 6  //OC0A
#define PWM3 9  //OC1A
#define PWM4 10 //OC1B
#define PWM5 11 //OC2A

#define TACH0 13
#define TACH0_1 PORTB |= _BV(PB5)
#define TACH0_0 PORTB &= ~_BV(PB5)
#define TACH0_SET {TACH0_1;} else {TACH0_0;}

void setPinsIO(){
  pinMode(RPMSENSOR0, INPUT);
  pinMode(RPMSENSOR1, INPUT);
  pinMode(RPMSENSOR2, INPUT);
  pinMode(RPMSENSOR3, INPUT);
  pinMode(RPMSENSOR4, INPUT);
  pinMode(RPMSENSOR5, INPUT);

  pinMode(TACH0, OUTPUT);

  pinMode(PWM0, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  pinMode(PWM4, OUTPUT);
  pinMode(PWM5, OUTPUT);
}

#define RT(P) ( pgm_read_dword( RT_PGM + (P) ) )

void init_extint()
{
  EICRA |= (1 << ISC00);
  EIMSK |= (1 << INT0);
}

void init_pcint()
{
  // PB0, PB4
  PCMSK0 = (1 << PCINT0) | (1 << PCINT4);

  // PC5
  PCMSK1 = (1 << PCINT13);

  // PD4, PD7
  PCMSK2 = (1 << PCINT20) | (1 << PCINT23);

  // PORTB, PORTC, PORTD
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
}

void disableRpmIRS(){
  EIMSK = 0;                            // disable external interrupt
  PCICR = 0;                            // disable pin change interrupts
}

void enableRpmIRS(){
  EIMSK |= (1 << INT0);                               // enable external interrupt
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
}

void setTimers(){

  TCCR0B = TCCR0B & B11111000;
  TCCR1B = TCCR1B & B11111000;
  TCCR2B = TCCR2B & B11111000;

  TCNT0 = 0;
  TCNT1 = 0;
  TCNT2 = 0;
  
  //---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------
  // put timer 0 in 8-bit fast hardware pwm mode
  //sbi(TCCR0A, WGM01);
  //sbi(TCCR0A, WGM00);
  // timer 0 is in 8-bit fast hardware pwm mode
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz

  // timer 0 is in 8-bit phase correct pwm mode
  TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(10);
   
  //---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(10);
   
  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
  
  // connect timers to output pins
  TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
  TCCR2A |= (1 << COM2A1) | (1 << COM2B1);
}

void writePwmValue(byte fanNumber, byte val) {
  switch(fanNumber) {
    case 0:
      OCR2B = 255 - val; // set pwm duty
      break;
    case 1:
      OCR0B = 255 - val; // set pwm duty
      break;
    case 2:
      OCR0A = 255 - val; // set pwm duty
      break;
    case 3:
      OCR1A = 255 - val; // set pwm duty
      break;
    case 4:
      OCR1B = 255 - val; // set pwm duty
      break;
    case 5:
      OCR2A = 255 - val; // set pwm duty
      break;
  }
}


