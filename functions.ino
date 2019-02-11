void serialWriteInt(unsigned int i){
  Serial.write(lowByte(i));
  Serial.write(highByte(i));
}

void serialWriteLong(unsigned long l){
  Serial.write(lowByte(l));
  Serial.write(lowByte(l >> 8));
  Serial.write(lowByte(l >> 16));
  Serial.write(lowByte(l >> 24));
}

void configuration(){
  Serial.print(F("!"));
  Serial.write(NUMBER_OF_RPM_TO_MAINBOARD + NUMBER_OF_THERMISTORS * 5 + 14);
  Serial.print(F("conf"));
  Serial.write((byte)HWversion);
  Serial.write(NUMBER_OF_FANS);
  Serial.write(NUMBER_OF_THERMISTORS);
  Serial.write(NUMBER_OF_MAINBOARD_CONNECTORS);
  Serial.write(NUMBER_OF_RPM_TO_MAINBOARD);
  Serial.write(CURVE_ANALOG_POINTS);
  Serial.write(CURVE_PWM_POINTS);
  Serial.write(CURVE_RPM_POINTS);
  for(byte i = 0; i < NUMBER_OF_RPM_TO_MAINBOARD; i++){
    Serial.write(rmpToMainboard(i));
  }
  Serial.write(hysteresis);
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    thermistors(i).sendDefinition();
  }
  Serial.write(profile);
  Serial.print(F("#"));
}

void guistat(CommandParameter &parameters){
  byte confToSend[NUMBER_OF_FANS];
  byte numberOfBytesToSend = 4;
  byte numberOfConfigurationsToSend = 0;
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    byte confNumber = parameters.NextParameterAsInteger( 255 );
    if(confNumber >= NUMBER_OF_FANS){
      break;
    }
    confToSend[i] = confNumber;
    numberOfConfigurationsToSend++;
    numberOfBytesToSend++;
    numberOfBytesToSend += sizeof(PWMConfiguration);
  }

  Serial.print(F("!"));
  Serial.write(numberOfBytesToSend);
  Serial.print(F("gui"));
  Serial.write(numberOfConfigurationsToSend);
  for(byte i = 0; i < numberOfConfigurationsToSend; i++){
    Serial.write(confToSend[i]);
    ConfigurationPWM(confToSend[i]).guiStat();
  }
  Serial.print(F("#"));
}

void guiEnable(){
  gui = 1;  
}

void guiDisable(){
  gui = 0;  
}

#ifdef TIMING_DEBUG
  void timing(){
    timeCountingStartFlag = 1;
  }

  void sendTime(){
    unsigned long us = micros();
    unsigned long ms = millis();
    unsigned long s = seconds();
    Serial.print(us);    
    Serial.println(F("us"));    
    Serial.print(ms);    
    Serial.println(F("ms"));    
    Serial.print(s);    
    Serial.println(F("s"));    
    s = s / 60;
    Serial.print(s);    
    Serial.println(F("min"));    
    s = s / 60;
    Serial.print(s);    
    Serial.println(F("hod"));    
  }
#endif

void guiUpdate(){
  Serial.print(F("!"));
  Serial.write(4 + 3 + NUMBER_OF_MAINBOARD_CONNECTORS * 2 + NUMBER_OF_THERMISTORS * 4 + NUMBER_OF_FANS * 2);
  Serial.print(F("guiU"));

  Serial.write(NUMBER_OF_FANS);
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    serialWriteInt(rpm[i]);
  }
  Serial.write(NUMBER_OF_THERMISTORS);
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(fakeTemp[i] > 0){
      serialWriteInt(Tint[i] + 10000);
    } else {
      serialWriteInt(Tint[i]);
    }
    serialWriteInt(TWithHysteresisInt[i]);
  }
  Serial.write(NUMBER_OF_MAINBOARD_CONNECTORS);
  for(byte i = 0; i < NUMBER_OF_MAINBOARD_CONNECTORS; i++){
    serialWriteInt(powerInADCAveraged[i]);
  }
  Serial.print(F("#"));
}

byte pidUpdate(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0 && (((fanNumber << 1) + i) & B00011111) == 0){
    unsigned short expectedRpm = rpm[fanNumber];
    if(pwmDisabled[fanNumber] == 0){
      if(conf.pwmDrive == 3){
        expectedRpm = conf.constRpm;
      }
      if(conf.pwmDrive == 4){
        expectedRpm = setpointPid[fanNumber];
      }
    }
    Serial.print(F("!"));
    Serial.write(8);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    serialWriteInt(expectedRpm);
    serialWriteInt((unsigned int) (rpm[fanNumber] + 0.5));
    Serial.write(pwm[fanNumber]);
    Serial.print(F("#"));
    updatesRTToSend[fanNumber]--;
  }
}

byte pidUpdateDirect(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0){
    unsigned short expectedRpm = rpm[fanNumber];
    if(pwmDisabled[fanNumber] == 0){
      if(conf.pwmDrive == 3){
        expectedRpm = conf.constRpm;
      }
      if(conf.pwmDrive == 4){
        expectedRpm = setpointPid[fanNumber];
      }
    }
    Serial.print(F("!"));
    Serial.write(8);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    serialWriteInt(expectedRpm);
    serialWriteInt((unsigned int) (rpm[fanNumber] + 0.5));
    Serial.write(pwm[fanNumber]);
    Serial.print(F("#"));
    updatesRTToSend[fanNumber]--;
  }
}

unsigned short roundRPM(double rpm){
  unsigned short rpmRounded = (rpm + 5) / 10;
  return rpmRounded * 10;
}

void setFanConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte powerInNumber = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  unsigned short constRPM = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte kp = parameters.NextParameterAsInteger();
  byte ki = parameters.NextParameterAsInteger();
  byte kd = parameters.NextParameterAsInteger();
  byte minPidPwm = parameters.NextParameterAsInteger();
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).set(pwmDrive, powerInNumber, constPwm, tSelect, constRPM, kp, ki, kd, minPidPwm);
    pid[pwmChannel].SetTunings((double) kp / 200, (double) ki / 200, (double) kd / 200);
    pid[pwmChannel].SetOutputLimits(ConfigurationPWM(pwmChannel).minPidPwm, 255);
    switch (pwmDrive) {
    case 0:
    case 1:
    case 2:
      pid[pwmChannel].SetMode(MANUAL);
      break;
    case 3:
    case 4:
      pid[pwmChannel].SetMode(AUTOMATIC);
    }
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setPowerInCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  unsigned short powerIn[CURVE_ANALOG_POINTS];
  byte pwm[CURVE_ANALOG_POINTS];
  for(byte i = 0; i < CURVE_ANALOG_POINTS; i++){
    powerIn[i] = parameters.NextParameterAsInteger( 0 );
    pwm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setPowerInCurve(CURVE_ANALOG_POINTS, powerIn, pwm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setPwmCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte t[CURVE_PWM_POINTS];
  byte pwm[CURVE_PWM_POINTS];
  for(byte i = 0; i < CURVE_PWM_POINTS; i++){
    t[i] = parameters.NextParameterAsInteger( 0 );
    pwm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setPWMCurve(CURVE_PWM_POINTS, t, pwm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setRpmCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte t[CURVE_RPM_POINTS];
  unsigned short rpm[CURVE_RPM_POINTS];
  for(byte i = 0; i < CURVE_RPM_POINTS; i++){
    t[i] = parameters.NextParameterAsInteger( 0 );
    rpm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setRPMCurve(CURVE_RPM_POINTS, t, rpm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
      pwmDisabled[pwmChannel] = delayParam;
    }
  }
}

void sendPidUpdates(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte numberOfUpdates = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || numberOfUpdates == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
      updatesRTToSend[pwmChannel] = numberOfUpdates;
    }
  }
}

void setConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  profile = parameters.NextParameterAsInteger( 0 );

  byte h = parameters.NextParameterAsInteger( 10 );

  if(h >= 0 && h <= 100){
    hysteresis = h;
  }

  byte i = 0;
  while(1){
    byte select = parameters.NextParameterAsInteger( 255 );
    if(select >= 0 && select < NUMBER_OF_FANS && i < NUMBER_OF_RPM_TO_MAINBOARD){
      rmpToMainboard(i++) = select;
    } else {
      return;
    }
  }
}

void setThermistor(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  byte thermistorNumber = parameters.NextParameterAsInteger();
  if(thermistorNumber < 0 || thermistorNumber >= NUMBER_OF_THERMISTORS) return;
  
  byte tempNominal = parameters.NextParameterAsInteger(25);
  unsigned short resistanceNominal = parameters.NextParameterAsInteger(10000);
  unsigned short bCoefficient = parameters.NextParameterAsInteger(3950);
  (thermistors(thermistorNumber)).Set(tempNominal, resistanceNominal, bCoefficient);

 #ifdef USE_TEMP_CACHE
    cacheT[thermistorNumber].clear();
 #endif    
}

void setTemp(CommandParameter &parameters){
  byte thermistorNumber = parameters.NextParameterAsInteger(255);
  if(thermistorNumber < 0 || thermistorNumber >= NUMBER_OF_THERMISTORS) return;
  
  unsigned char fakeTemperature = parameters.NextParameterAsInteger(0);
  if(fakeTemperature >= 0 && fakeTemperature <= MAX_ALLOWED_TEMP){
    fakeTemp[thermistorNumber] = fakeTemperature;
  }
}

void cacheStatus(){
#ifdef USE_TEMP_CACHE
if(gui){
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    cacheT[i].printStatus();
  }
} else {
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    Serial.print(F("cache T"));
    Serial.println(i);
    cacheT[i].printStatus();
  }
}
#endif
#ifdef USE_PWM_CACHE
if(gui){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    cacheFan[i].printStatus();
  }
} else {
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F("cache RPMbyTemp["));
    Serial.print(i);
    Serial.println(F("]"));
    cacheFan[i].printStatus();
  }
}
#endif
}

#ifdef FREE_MEMORY_DEBUG
int getPointerToStartOfFreeMemory(){
  extern int  __bss_end;
  extern int* __brkval;
  if (reinterpret_cast<int>(__brkval) == 0) {
    // if no heap use from end of bss section
    if(!gui){
      Serial.println(F("from end of bss"));
    }
    return reinterpret_cast<int>(&__bss_end);
  } else {
    // use from top of stack to heap
    if(!gui){
      Serial.println(F("from top of stack to heap"));
    }
    return reinterpret_cast<int>(__brkval);
  }
}

void fillFreeMemoryByZeroes(){
  int memPointer = getPointerToStartOfFreeMemory();
  int free_memory;
  free_memory = reinterpret_cast<int>(&free_memory) - memPointer;
  while(memPointer < reinterpret_cast<int>(&free_memory)) {
    (*(byte*)(memPointer++)) = 0;
  }
}

void freeMem(CommandParameter &parameters)
{
  int f = parameters.NextParameterAsInteger( 0 );
  if(f > 0){
    Serial.print(F("fibbonacci: "));
    Serial.println(fibbonacci(f));
  }
  
  int notUsedMemory = 0;
  int memPointer = getPointerToStartOfFreeMemory();
  int free_memory;
  free_memory = reinterpret_cast<int>(&free_memory) - memPointer;

  if(!gui){
    Serial.print(F("memPointer address: "));
    Serial.println(memPointer);
    Serial.print(F("free memory address: "));
    Serial.println(reinterpret_cast<int>(&free_memory));
  }
  while(memPointer < reinterpret_cast<int>(&free_memory)) {
    if((*(byte*)(memPointer++)) == 0){
      notUsedMemory++;
    } else {
      break;
    }
  }
  if(!gui){
    Serial.print(F("actual free memory: "));
    Serial.print(free_memory);
    Serial.println(F(" Bytes"));
    Serial.print(F("not used memory: "));
    Serial.print(notUsedMemory);
    Serial.println(F(" Bytes"));
  } else {
    Serial.print(F("!"));
    Serial.write(11);
    Serial.print(F("freemem"));
    serialWriteInt(free_memory);
    serialWriteInt(notUsedMemory);
    Serial.println(F("#"));
  }
}

// function for increase stack test
int fibbonacci(unsigned long input){
  if(input <= 0)
    return 0;
  if(input == 1)
    return 1;
  return (fibbonacci(input - 1) + fibbonacci(input - 2));
}
#endif

#ifdef CALIBRATE_THERMISTORS
void setCalibrateRNominal(CommandParameter &parameters)
{
  tempNominal = parameters.NextParameterAsInteger( 255 );
  while(1){
    byte thermistorNumber = parameters.NextParameterAsInteger( 255 );

    if(tempNominal == 255 || thermistorNumber == 255)
      return;

    if(thermistorNumber >= 0 && thermistorNumber < NUMBER_OF_THERMISTORS && TConnected[thermistorNumber]){
      calibrateR[thermistorNumber] = 20;
    }
  }
}

void setCalibrateB(CommandParameter &parameters)
{
  tempNominal = parameters.NextParameterAsInteger( 255 );
  while(1){
    byte thermistorNumber = parameters.NextParameterAsInteger( 255 );

    if(tempNominal == 255 || thermistorNumber == 255)
      return;

    if(thermistorNumber >= 0 && thermistorNumber < NUMBER_OF_THERMISTORS && TConnected[thermistorNumber]){
      calibrateBeta[thermistorNumber] = 20;
    }
  }
}

void calibrateRNominal()
{
  int tempExpectedInt = tempNominal * 10;
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(calibrateR[i] > 0){
      calibrateR[i]--;
      int deltaT = Tint[i] - tempExpectedInt;
      thermistors(i).resistanceNominal = thermistors(i).resistanceNominal - (10 * deltaT);
      thermistors(i).tempNominal = tempNominal;
#ifdef USE_TEMP_CACHE
      cacheT[i].clear();
#endif
    }
  }
}

void calibrateB()
{
  int tempExpectedInt = tempNominal * 10;
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(calibrateBeta[i] > 0){
      calibrateBeta[i]--;
      int deltaT = Tint[i] - tempExpectedInt;
      if(tempNominal > thermistors(i).tempNominal){
        thermistors(i).bCoefficient = thermistors(i).bCoefficient + (10 * deltaT);
      } else {
        thermistors(i).bCoefficient = thermistors(i).bCoefficient - (10 * deltaT);
      }
#ifdef USE_TEMP_CACHE
      cacheT[i].clear();
#endif
    }
  }
}
#endif

#if HWversion == 2
void setICRn(CommandParameter &parameters)
{
  //default PWM frequency of 25000Hz (16000000 / (ICRn*2-2))
  unsigned int parameter = parameters.NextParameterAsInteger( 321 );
  // 800Hz - 80kHz
  if(parameter >= 101 && parameter <= 10001){
    setRatio((word)parameter);
    for(byte i = 0; i < NUMBER_OF_FANS; i++){
      #ifdef USE_PWM_CACHE
      cacheFan[i].clear();
      #endif
      writePwmValue(i, pwm[i]);
    }
  }
}
#endif

void Cmd_Unknown()
{
  if(gui){
    Serial.print(F("!"));
    Serial.write(15);
    Serial.print(F("Unknown command#"));
  } else {
    Serial.println(F("Unknown command"));
  }
}

