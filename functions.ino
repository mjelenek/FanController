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
  ConfigurationPWM0.Data.guiStat();
  ConfigurationPWM1.Data.guiStat();
  ConfigurationPWM2.Data.guiStat();
}

void guistat2(){
  Serial.print(F("!!"));
  Serial.write(62);
  Serial.print(F("guistat2"));
  ConfigurationPWM3.Data.guiStat();
  ConfigurationPWM4.Data.guiStat();
  ConfigurationPWM5.Data.guiStat();
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

  serialWriteInt(roundRPM(rpm0));
  serialWriteInt(roundRPM(rpm1));
  serialWriteInt(roundRPM(rpm2));
  serialWriteInt(roundRPM(rpm3));
  serialWriteInt(roundRPM(rpm4));
  serialWriteInt(roundRPM(rpm5));
/*
  serialWriteInt(rpm0);
  serialWriteInt(rpm1);
  serialWriteInt(rpm2);
  serialWriteInt(rpm3);
  serialWriteInt(rpm4);
  serialWriteInt(rpm5);
*/
  serialWriteInt(T0int);
  serialWriteInt(T1int);
//  serialWriteInt(T0WithHysteresisInt);
//  serialWriteInt(T1WithHysteresisInt);
//  serialWriteInt(sensorValue6Averaged);
//  serialWriteInt(sensorValue7Averaged);

//  serialWriteInt(cnt2);
//  serialWriteInt(T0WithHysteresisInt + T1WithHysteresisInt);
//  serialWriteInt(OutputInt);
}

void pidUpdate(byte fan, unsigned short desiredRpm, unsigned short rpm, byte pwm){
  Serial.print(F("!!"));
  Serial.write(15);
  Serial.print(F("pidUpdate"));
  Serial.write(fan);
  serialWriteInt(desiredRpm);
  serialWriteInt(rpm);
  Serial.write(pwm);
}

unsigned int roundRPM(double rpm){
  unsigned int rpmRounded = (rpm + 5) / 10;
  return rpmRounded * 10;
}

void setPwmConfiguration(CommandParameter &parameters){
  
//  Serial.println(F("setPwmConfiguration"));

  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte minPwm = parameters.NextParameterAsInteger();
  byte maxPwm = parameters.NextParameterAsInteger();
  byte tempTarget = parameters.NextParameterAsInteger();
  byte tempMax = parameters.NextParameterAsInteger();
/*
  Serial.print(F("channel: "));
  Serial.println(pwmChannel);
  Serial.print(F("drive: "));
  Serial.println(pwmDrive);
  Serial.print(F("const: "));
  Serial.println(constPwm);
  Serial.print(F("min: "));
  Serial.println(minPwm);
  Serial.print(F("max: "));
  Serial.println(maxPwm);
  Serial.print(F("tTarget: "));
  Serial.println(tempTarget);
  Serial.print(F("tMax: "));
  Serial.println(tempMax);
*/
  if(pwmChannel >= 0 && pwmChannel <= 5){
    switch (pwmChannel) {
    case 0:
      ConfigurationPWM0.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
      break;
    case 1:
      ConfigurationPWM1.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
      break;
    case 2:
      ConfigurationPWM2.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
      break;
    case 3:
      ConfigurationPWM3.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
      break;
    case 4:
      ConfigurationPWM4.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
      break;
    case 5:
      ConfigurationPWM5.Data.set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    }
  
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
    switch (pwmChannel) {
    case 0:
      ConfigurationPWM0.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
      break;
    case 1:
      ConfigurationPWM1.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
      break;
    case 2:
      ConfigurationPWM2.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
      break;
    case 3:
      ConfigurationPWM3.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
      break;
    case 4:
      ConfigurationPWM4.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
      break;
    case 5:
      ConfigurationPWM5.Data.setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd);
    }
//    pid[pwmChannel].SetTunings((double) kp / 100, (double) ki / 100, (double) kd / 100);
  }
}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;
    
    switch (pwmChannel) {
    case 0:
      pwm0Disabled = delayParam;
      break;
    case 1:
      pwm1Disabled = delayParam;
      break;
    case 2:
      pwm2Disabled = delayParam;
      break;
    case 3:
      pwm3Disabled = delayParam;
      break;
    case 4:
      pwm4Disabled = delayParam;
      break;
    case 5:
      pwm5Disabled = delayParam;
    }
  }
}

void setRPMToMainboard(CommandParameter &parameters){
  byte select = parameters.NextParameterAsInteger();
  if(select >= 0 && select <= 5){
    rmpToMainboard = select;
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

