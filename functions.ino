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

void guistat1(){
  Serial.print(F("!!"));
  Serial.write(62);
  Serial.print(F("guistat1"));
  ConfigurationPWM0.Data.m_UserData.guiStat();
  ConfigurationPWM1.Data.m_UserData.guiStat();
  ConfigurationPWM2.Data.m_UserData.guiStat();
}

void guistat2(){
  Serial.print(F("!!"));
  Serial.write(62);
  Serial.print(F("guistat2"));
  ConfigurationPWM3.Data.m_UserData.guiStat();
  ConfigurationPWM4.Data.m_UserData.guiStat();
  ConfigurationPWM5.Data.m_UserData.guiStat();
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
#endif

void guiUpdate(){
  Serial.print(F("!!"));
  Serial.write(25);
  Serial.print(F("guiUpdate"));

  serialWriteInt(roundRPM(rpm[0]));
  serialWriteInt(roundRPM(rpm[1]));
  serialWriteInt(roundRPM(rpm[2]));
  serialWriteInt(roundRPM(rpm[3]));
  serialWriteInt(roundRPM(rpm[4]));
  serialWriteInt(roundRPM(rpm[5]));
  serialWriteInt(T0int);
  serialWriteInt(T1int);
//  serialWriteInt(cnt2);
//  serialWriteInt(T0WithHysteresisInt);
//  serialWriteInt(T1WithHysteresisInt);
//  serialWriteInt(sensorValue6Averaged);
//  serialWriteInt(sensorValue7Averaged);

//  serialWriteInt(cnt2);
//  serialWriteInt(T0WithHysteresisInt + T1WithHysteresisInt);
//  serialWriteInt(OutputInt);
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
    Serial.print(F("!!"));
    Serial.write(8);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    serialWriteInt(expectedRpm);
    serialWriteInt(rpm[fanNumber]);
    Serial.write(pwm[fanNumber]);
    updatesRTToSend[fanNumber]--;
  }
}

unsigned int roundRPM(double rpm){
  unsigned int rpmRounded = (rpm + 5) / 10;
  return rpmRounded * 10;
}

void setPwmConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte minPwm = parameters.NextParameterAsInteger();
  byte maxPwm = parameters.NextParameterAsInteger();
  byte tempTarget = parameters.NextParameterAsInteger();
  byte tempMax = parameters.NextParameterAsInteger();

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM[pwmChannel] -> set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
  
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
  unsigned short minRPM = parameters.NextParameterAsInteger();
  unsigned short maxRPM = parameters.NextParameterAsInteger();
  byte tempTargetRPM = parameters.NextParameterAsInteger();
  byte tempMaxRPM = parameters.NextParameterAsInteger();
  byte kp = parameters.NextParameterAsInteger();
  byte ki = parameters.NextParameterAsInteger();
  byte kd = parameters.NextParameterAsInteger();

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM[pwmChannel] -> setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
    pid[pwmChannel].SetTunings((double) kp / 100, (double) ki / 100, (double) kd / 100);
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

void sendPidUpates(CommandParameter &parameters){
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

void setRPMToMainboard(CommandParameter &parameters){
  byte select = parameters.NextParameterAsInteger();
  if(select >= 0 && select <= 5){
    *rmpToMainboard = select;
  }
}

void tempCacheStatus(){
  cacheT0.printStatus();
  cacheT1.printStatus();
}

void Cmd_Unknown()
{
  if(gui){
    Serial.write(15);
    Serial.print(F("Unknown command"));
  } else {
    Serial.println(F("Unknown command"));
  }
}

