void serialWriteInt(unsigned int i){
  Serial.write(lowByte(i));
  Serial.write(highByte(i));
}

void serialWriteLong(unsigned long l){
  Serial.write(lowByte(l));
  Serial.write(lowByte(l >> 8));
  Serial.write(lowByte(l >> 16));
  Serial.write(lowByte(l >> 32));
}

void configuration(){
  Serial.print(F("!"));
  Serial.write(NUMBER_OF_THERMISTORS * 5 + 7);
  Serial.print(F("conf"));
  Serial.write((byte)HWversion);
  Serial.write(rmpToMainboard);
  Serial.write(hysteresis);
  thermistors -> sendDefinition();
  (thermistors+1) -> sendDefinition();
  Serial.print(F("#"));
}

void guistat1(){
  Serial.print(F("!"));
  Serial.write(106);
  Serial.print(F("gui1"));
  ConfigurationPWM(0).guiStat();
  ConfigurationPWM(1).guiStat();
  ConfigurationPWM(2).guiStat();
  Serial.print(F("#"));
}

void guistat2(){
  Serial.print(F("!"));
  Serial.write(106);
  Serial.print(F("gui2"));
  ConfigurationPWM(3).guiStat();
  ConfigurationPWM(4).guiStat();
  ConfigurationPWM(5).guiStat();
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
    unsigned long s = ms / 1000;
    Serial.println(us);    
    Serial.println(ms);    
    Serial.println(s);    
  }
#endif

void guiUpdate(){
  Serial.print(F("!"));
  Serial.write(4 + NUMBER_OF_THERMISTORS * 4 + NUMBER_OF_FANS * 2);
  Serial.print(F("guiU"));

  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    serialWriteInt(rpm[i]);
  }
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    serialWriteInt(Tint[i]);
    serialWriteInt(TWithHysteresisInt[i]);
  }
  Serial.print(F("#"));
}

byte pidUpdate(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0 && (((fanNumber << 2) + i) & B00011111) == 0){
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

void setPwmConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte t0 = parameters.NextParameterAsInteger( 0 );
  byte pwm0 = parameters.NextParameterAsInteger( 0 );
  byte t1 = parameters.NextParameterAsInteger( 0 );
  byte pwm1 = parameters.NextParameterAsInteger( 0 );
  byte t2 = parameters.NextParameterAsInteger( 0 );
  byte pwm2 = parameters.NextParameterAsInteger( 0 );
  byte t3 = parameters.NextParameterAsInteger( 0 );
  byte pwm3 = parameters.NextParameterAsInteger( 0 );
  byte t4 = parameters.NextParameterAsInteger( 0 );
  byte pwm4 = parameters.NextParameterAsInteger( 0 );

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM(pwmChannel).set(pwmDrive, constPwm, tSelect, t0, pwm0, t1, pwm1, t2, pwm2, t3, pwm3, t4, pwm4);
#ifdef USE_PWM_CACHE
    cacheRMPbyTemp[pwmChannel].clear();
#endif    
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
  }
}

void setPidConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  unsigned short constRPM = parameters.NextParameterAsInteger();
  byte kp = parameters.NextParameterAsInteger();
  byte ki = parameters.NextParameterAsInteger();
  byte kd = parameters.NextParameterAsInteger();
  byte minPidPwm = parameters.NextParameterAsInteger();
  byte t0 = parameters.NextParameterAsInteger( 0 );
  unsigned short rpm0 = parameters.NextParameterAsInteger( 0 );
  byte t1 = parameters.NextParameterAsInteger( 0 );
  unsigned short rpm1 = parameters.NextParameterAsInteger( 0 );
  byte t2 = parameters.NextParameterAsInteger( 0 );
  unsigned short rpm2 = parameters.NextParameterAsInteger( 0 );
  byte t3 = parameters.NextParameterAsInteger( 0 );
  unsigned short rpm3 = parameters.NextParameterAsInteger( 0 );
  byte t4 = parameters.NextParameterAsInteger( 0 );
  unsigned short rpm4 = parameters.NextParameterAsInteger( 0 );

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM(pwmChannel).setPid(constRPM, kp, ki, kd, minPidPwm, t0, rpm0, t1, rpm1, t2, rpm2, t3, rpm3, t4, rpm4);
    pid[pwmChannel].SetTunings((double) kp / 200, (double) ki / 200, (double) kd / 200);
    pid[pwmChannel].SetOutputLimits(ConfigurationPWM(pwmChannel).minPidPwm, 255);
#ifdef USE_PWM_CACHE
    cacheRMPbyTemp[pwmChannel].clear();
#endif    
  }
}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel <= 5){
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

    if(pwmChannel >= 0 && pwmChannel <= 5){
      updatesRTToSend[pwmChannel] = numberOfUpdates;
    }
  }
}

void setConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  byte select = parameters.NextParameterAsInteger();
  byte h = parameters.NextParameterAsInteger();

  if(select >= 0 && select <= 5){
    rmpToMainboard = select;
  }
  
  if(h >= 0 && h <= 100){
    hysteresis = h;
  }
}

void setThermistor(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  byte thermistorNumber = parameters.NextParameterAsInteger();
  if(thermistorNumber < 0 || thermistorNumber >= NUMBER_OF_THERMISTORS) return;
  
  byte tempNominal = parameters.NextParameterAsInteger(25);
  unsigned short resistanceNominal = parameters.NextParameterAsInteger(10000);
  unsigned short bCoefficient = parameters.NextParameterAsInteger(3950);
  (thermistors + thermistorNumber) -> Set(tempNominal, resistanceNominal, bCoefficient);

 #ifdef USE_TEMP_CACHE
    cacheT[thermistorNumber].clear();
 #endif    
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
    cacheRMPbyTemp[i].printStatus();
  }
} else {
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F("cache RPMbyTemp["));
    Serial.print(i);
    Serial.println(F("]"));
    cacheRMPbyTemp[i].printStatus();
  }
}
#endif
}

#ifdef FREE_MEMORY_DEBUG
void freeMem(CommandParameter &parameters)
{
  int f = parameters.NextParameterAsInteger();
  if(f > 0){
    Serial.print(F("fibbonacci: "));
    Serial.println(fibbonacci(f));
  }
  
  int memPointer;
  extern int  __bss_end;
  extern int* __brkval;
  int notUsedMemory = 0;
  int free_memory;
  if (reinterpret_cast<int>(__brkval) == 0) {
    // if no heap use from end of bss section
    free_memory = reinterpret_cast<int>(&free_memory) - reinterpret_cast<int>(&__bss_end);
    memPointer = reinterpret_cast<int>(&__bss_end);
  } else {
    // use from top of stack to heap
    free_memory = reinterpret_cast<int>(&free_memory) - reinterpret_cast<int>(__brkval);
    memPointer = reinterpret_cast<int>(__brkval);
  }
  while(memPointer < &free_memory) {
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
int fibbonacci(int input){
  if(input <= 0)
    return 0;
  if(input == 1)
    return 1;
  return (fibbonacci(input - 1) + fibbonacci(input - 2));
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

