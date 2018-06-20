#define NUMBER_OF_THERMISTORS 5
#define NUMBER_OF_FANS 12
#define NUMBER_OF_MAINBOARD_CONNECTORS 4
#define NUMBER_OF_RPM_TO_MAINBOARD 4

//by multimeter
//#define ANALOGREFERENCEVOLTAGE 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
unsigned short RT[NUMBER_OF_THERMISTORS] ={9990, 9990, 9990, 9990, 9990};

#define RPMSENSOR0 21  //INT0
#define RPMSENSOR1 20  //INT1
#define RPMSENSOR2 19  //INT2
#define RPMSENSOR3 18  //INT3
#define RPMSENSOR4 2   //INT4
#define RPMSENSOR5 3   //INT5
#define RPMSENSOR6 15  //PCINT9/PJ0
#define RPMSENSOR7 14  //PCINT10/PJ1
#define RPMSENSOR8 53  //PCINT0/PB0
#define RPMSENSOR9 52  //PCINT1/PB1
#define RPMSENSOR10 83 //PCINT22/PK7/ADC15
#define RPMSENSOR11 82 //PCINT23/PK6/ADC14



//PWM output pins
#define PWM0 13  //OC1C
#define PWM1 12  //OC1B
#define PWM2 11  //OC1A
#define PWM3 10  //OC2A
#define PWM4 9   //OC2B
#define PWM5 8   //OC4C
#define PWM6 7   //OC4B
#define PWM7 6   //OC4A
#define PWM8 5   //OC3A
#define PWM9 46  //OC5A
#define PWM10 45 //OC5B
#define PWM11 44 //OC5C

byte PWMOUT[] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5, PWM6, PWM7, PWM8, PWM9, PWM10, PWM11};

//TACH output pins
#define TACH0 39 //PG2
#define TACH1 37 //PC7
#define TACH2 35 //PC5
#define TACH3 33 //PC3

#define TACH0_1 PORTG |= _BV(PG2)
#define TACH0_0 PORTG &= ~_BV(PG2)
#define TACH0_SET {TACH0_1;} else {TACH0_0;}

#define TACH1_1 PORTC |= _BV(PC7)
#define TACH1_0 PORTC &= ~_BV(PC7)
#define TACH1_SET {TACH1_1;} else {TACH1_0;}

#define TACH2_1 PORTC |= _BV(PC5)
#define TACH2_0 PORTC &= ~_BV(PC5)
#define TACH2_SET {TACH2_1;} else {TACH2_0;}

#define TACH3_1 PORTC |= _BV(PC3)
#define TACH3_0 PORTC &= ~_BV(PC3)
#define TACH3_SET {TACH3_1;} else {TACH3_0;}

void setPinsIO(){
  pinMode(RPMSENSOR0, INPUT);
  pinMode(RPMSENSOR1, INPUT);
  pinMode(RPMSENSOR2, INPUT);
  pinMode(RPMSENSOR3, INPUT);
  pinMode(RPMSENSOR4, INPUT);
  pinMode(RPMSENSOR5, INPUT);
  pinMode(RPMSENSOR6, INPUT);
  pinMode(RPMSENSOR7, INPUT);
  pinMode(RPMSENSOR8, INPUT);
  pinMode(RPMSENSOR9, INPUT);
  pinMode(RPMSENSOR10, INPUT);
  pinMode(RPMSENSOR11, INPUT);

  pinMode(TACH0, OUTPUT);
  pinMode(TACH1, OUTPUT);
  pinMode(TACH2, OUTPUT);
  pinMode(TACH3, OUTPUT);

  pinMode(PWM0, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  pinMode(PWM4, OUTPUT);
  pinMode(PWM5, OUTPUT);
  pinMode(PWM6, OUTPUT);
  pinMode(PWM7, OUTPUT);
  pinMode(PWM8, OUTPUT);
  pinMode(PWM9, OUTPUT);
  pinMode(PWM10, OUTPUT);
  pinMode(PWM11, OUTPUT);
}

void init_extint()
{
  EICRA |= (1 << ISC00);
  EIMSK |= (1 << INT0);
}

void init_pcint()
{
  /*
  // PB0, PB4
  PCMSK0 = (1 << PCINT0) | (1 << PCINT4);

  // PC5
  PCMSK1 = (1 << PCINT13);

  // PD4, PD7
  PCMSK2 = (1 << PCINT20) | (1 << PCINT23);

  // PORTB, PORTC, PORTD
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
*/
}

void setTimers(){
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
   
  //---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
   
  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  //TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  TCCR3B = TCCR3B & B11111000 | B00000100;    // set timer 3 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  TCCR4B = TCCR4B & B11111000 | B00000100;    // set timer 4 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  TCCR5B = TCCR5B & B11111000 | B00000100;    // set timer 5 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
}

