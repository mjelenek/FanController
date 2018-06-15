void setup() {

//  wdt_disable();
 //serial port configuration
  Serial.begin(115200);
//  Serial.begin(19200);

//welcome
  Serial.println(F("Starting..."));

#ifdef FREE_MEMORY_DEBUG
  fillFreeMemoryByZeroes();
#endif

  setPinsIO();

  setTimers();

  init_adc();

  init_thermistors();
  
  loadConfiguration();

  init_extint();
  
  init_pcint();

  setSerialCommandHandler();

  init_pid();

//  printTempProfile();
//  measureInterrupts();

  Serial.println(F("System started. Type !help for more informations."));
  delay(1);

  start = micros();
}

void init_thermistors(){
// VOLTAGETHERMISTOR == ANALOGREFERENCEVOLTAGE
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    RTkoeficient[i] = (unsigned long)RT[i] * 1023;
  }

  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    Serial.print(F("koeficientT"));
    Serial.print(i);
    Serial.print(F(":"));
    Serial.print(RTkoeficient[i]);
    Serial.println("");
  }

  delay(20);  //wait for read values from ADC;
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    countT(i);  
  }
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    Serial.print(F("T"));
    Serial.print(i);
    if(TConnected[i]){
      Serial.println(F(" connected"));
    } else {
      Serial.println(F(" not connected"));
    }
  }
}

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

void init_pid(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    pid[i].SetOutputLimits(ConfigurationPWM(i).minPidPwm, 255);
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
      unsigned long thermistorResistance = RTkoeficient[0] / i - RT[0];
      unsigned int t = countTemperature(thermistorResistance, thermistors(0));
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
    result = result + countTemperature(i, thermistors(0));
  }
  return result;
}

