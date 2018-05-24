void setup() {

//  wdt_disable();
 //serial port configuration
  Serial.begin(115200);
//  Serial.begin(19200);

//welcome
  Serial.println(F("Starting..."));

  setPinsIO();

  setTimers();

  init_adc();

  init_thermistors();
  
  loadConfiguration();

  init_pcint();

  setSerialCommandHandler();

  init_pid();

//  printTempProfile();
//  measureInterrupts();

  Serial.println(F("System started. Type !help for more informations."));
  delay(1);

  start = micros();
}

void setPinsIO(){
  pinMode(RPMSENSOR0, INPUT);
  pinMode(RPMSENSOR1, INPUT);
  pinMode(RPMSENSOR2, INPUT);
  pinMode(RPMSENSOR3, INPUT);
  pinMode(RPMSENSOR4, INPUT);
  pinMode(RPMSENSOR5, INPUT);

  pinMode(LED_OUT, OUTPUT);

  pinMode(PWM0, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  pinMode(PWM4, OUTPUT);
  pinMode(PWM5, OUTPUT);
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
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
   
  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
}

void init_thermistors(){
// VOLTAGETHERMISTOR == ANALOGREFERENCEVOLTAGE
  RT0koeficient = (unsigned long)RT0 * 1023;
  RT1koeficient = (unsigned long)RT1 * 1023;

  Serial.print(F("koeficientT0:"));
  Serial.print(RT0koeficient);
  Serial.print(F(", koeficientT1:"));
  Serial.println(RT1koeficient);

  delay(5);  //wait for read values from ADC;
  countT0();
  countT1();
  if(T0Connected){
    Serial.print(F("T0 connected"));
  } else {
    Serial.print(F("T0 not connected"));
  }
  if(T1Connected){
    Serial.println(F(", T1 connected"));
  } else {
    Serial.println(F(", T1 not connected"));
  }
}

void init_pcint()
{
  // PB0, PB4
  PCMSK0 = (1 << PCINT0) | (1 << PCINT4);

  // PC5
  PCMSK1 = (1 << PCINT13);

  // PD2, PD4, PD7
  PCMSK2 = (1 << PCINT18) | (1 << PCINT20) | (1 << PCINT23);

  // PORTB, PORTC, PORTD
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
}

void init_adc()
{
  DIDR0 = B11011111;                                   // Disable digital input buffer for ADC pins
  
  ADMUX = 0;                                          // VREF is EXTERNAL, channel 0
  ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // prescaler to 128
         | (1 << ADATE)                               // ADC Auto Trigger Enable
         | (1 << ADIE)                                // Enable ADC conversion complete interrupt
         | (1 << ADEN)                                // Enable ADC
         | (1 << ADSC);                               // Start conversion
}

void init_pid(){
  for(int i = 0; i <= 5; i++){
    pid[i].SetOutputLimits(ConfigurationPWM(i).minPidPwm, 255);
//    pid[i].SetSampleTime(62000);                     // will be computed every 64ms
    pid[i].SetSampleTime(120000);                     // will be computed every 128ms

    switch (ConfigurationPWM(i).pwmDrive) {
    case 0:
    case 1:
    case 2:
      pid[i].SetMode(MANUAL);
      break;
    case 3:
    case 4:
      pid[i].SetMode(AUTOMATIC);
    }
  }
}

void printTempProfile(){
  for(int i = 0; i <= 1024; i++){
      unsigned long thermistorResistance = RT0koeficient / i - RT0;
      unsigned int t = countTemperature(thermistorResistance, thermistors);
      short tShort = (short)t;
      Serial.print(i);
      Serial.print(" - ");
      Serial.println(tShort);
  }
}

void measureInterrupts(){
  delay(20);
  PCICR = 0;                            // disable pin change interrupts
  ADCSRA &= ~(1 << ADIE);               // disable ADC conversion complete interrupt
  unsigned long start = micros();
  unsigned int a = doSomeMath(100);
  now = micros();
  unsigned long m1 = now - start;

  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
  ADCSRA |= (1 << ADIE);                // enable ADC conversion complete interrupt
  delay(10);
  start = micros();
  unsigned int b = doSomeMath(101);
  now = micros();
  unsigned long m2 = now - start;
  float percentage = (1 - (float)m1 / (float)m2 ) * 100;
  unsigned int c = a + b;
  Serial.print(F("c="));
  Serial.println(c);
  Serial.print(F("m1:"));
  Serial.print(m1);
  Serial.println(F("us"));
  Serial.print(F("m2:"));
  Serial.print(m2);
  Serial.println(F("us"));
  Serial.print(F("Interrupts(except TIMER0_OVF) occupy "));
  Serial.print(percentage, 2);
  Serial.println(F("% of processor time"));
}

unsigned int doSomeMath(unsigned int x){
  unsigned int result = 0;  
  for(unsigned int i = x; i <= (x + 2000); i++){
    result = result + countTemperature(i, thermistors);
  }
  return result;
}

