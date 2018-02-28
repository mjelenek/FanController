void printBCD(unsigned long bcd){ // value from 0 to 99999999
  if(bcd > 99999999){
      Serial.print(F("99999999"));
  }
  
  byte writable = 0;
  byte number;
  
  if(bcd >> 8 == 0){
    number = (bcd >> 4) & 0xF;
    if(number > 0){
      Serial.write(number + '0');
    }
    number = bcd & 0xF;
    Serial.write(number + '0');
    return;
  } 

  if(bcd >> 16 == 0){
    unsigned int bcdInt = (unsigned int)bcd;
    for(signed char a = 12; a >= 0; a-=4){
      number = (bcdInt >> a) & 0xF;
      if(number > 0) writable = 1;
      if(writable == 1 || a == 0){
        Serial.write(number + '0');
      }
    }
    return;
  } 
  
  for(signed char a = 28; a >= 0; a-=4){
    number = (bcd >> a) & 0xF;
    if(number > 0) writable = 1;
    if(writable == 1 || a == 0){
      Serial.write(number + '0');
    }
  }
}

void printlnBCD(unsigned long bcd){ // value from 0 to 99999999
  printBCD(bcd);
  Serial.println("");
}

void printStatus(){
  Serial.print(F("T0:"));
  if (T0Connected){
    unsigned int T0BCD = binaryToBCD(T0int);
    printBCD(T0BCD >> 4);
    Serial.print(F("."));
    printBCD(T0BCD & 0xF);
  } else {
    Serial.print(F("N/A"));
  }
  Serial.print(F(" T1:"));
  if (T1Connected){
    unsigned int T1BCD = binaryToBCD(T1int);
    printBCD(T1BCD >> 4);
    Serial.print(F("."));
    printlnBCD(T1BCD & 0xF);
  } else {
    Serial.println(F("N/A"));
  }
  Serial.print(F("RPM0:"));
  printBCD(binaryToBCDLong(rpm[0]));
  Serial.print(F(" RPM1:"));
  printBCD(binaryToBCDLong(rpm[1]));
  Serial.print(F(" RPM2:"));
  printBCD(binaryToBCDLong(rpm[2]));
  Serial.print(F(" RPM3:"));
  printBCD(binaryToBCDLong(rpm[3]));
  Serial.print(F(" RPM4:"));
  printBCD(binaryToBCDLong(rpm[4]));
  Serial.print(F(" RPM5:"));
  printlnBCD(binaryToBCDLong(rpm[5]));
}

void printFullStatus(){
  printStatus();
  Serial.print(F("PWM0:"));
  printBCD(binaryToBCD(pwm[0]));
  Serial.print(F(" PWM1:"));
  printBCD(binaryToBCD(pwm[1]));
  Serial.print(F(" PWM2:"));
  printBCD(binaryToBCD(pwm[2]));
  Serial.print(F(" PWM3:"));
  printBCD(binaryToBCD(pwm[3]));
  Serial.print(F(" PWM4:"));
  printBCD(binaryToBCD(pwm[4]));
  Serial.print(F(" PWM5:"));
  printlnBCD(binaryToBCD(pwm[5]));
  Serial.print(F(" pwm0Drive: "));
  printlnPwmDrive(ConfigurationPWM0.Data.m_UserData);
  Serial.print(F(" pwm1Drive: "));
  printlnPwmDrive(ConfigurationPWM1.Data.m_UserData);
  Serial.print(F(" pwm2Drive: "));
  printlnPwmDrive(ConfigurationPWM2.Data.m_UserData);
  Serial.print(F(" pwm3Drive: "));
  printlnPwmDrive(ConfigurationPWM3.Data.m_UserData);
  Serial.print(F(" pwm4Drive: "));
  printlnPwmDrive(ConfigurationPWM4.Data.m_UserData);
  Serial.print(F(" pwm5Drive: "));
  printlnPwmDrive(ConfigurationPWM5.Data.m_UserData);
  Serial.print(F(" hysteresis: "));
  printlnBCD(binaryToBCDLong(*hysteresis));
}  
  

void printlnPwmDrive(PWMConfiguration &conf){
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      Serial.println(F("analog input"));
      break;
    case 1:
      Serial.print(F("constant power, PWM="));
      printlnBCD(binaryToBCD(conf.constPwm));
      break;
    case 2:
      Serial.print(F("PWM by temperature, minPWM="));
      printBCD(binaryToBCD(conf.minPwm));
      Serial.print(F(", maxPWM="));
      printlnBCD(binaryToBCD(conf.maxPwm));
      break;
    case 3:
      Serial.print(F("constant speed, expected RPM="));
      printlnBCD(binaryToBCDLong(conf.constRpm));
      break;
    case 4:
      Serial.print(F("speed by temperature, minRPM="));
      printBCD(binaryToBCDLong(conf.minRpm));
      Serial.print(F(", maxRPM="));
      printlnBCD(binaryToBCDLong(conf.maxRpm));
      break;
  }
}

void printDelay(byte i, unsigned long d){
  if(!gui){
    Serial.print(F("!"));
    printBCD(binaryToBCD(i));
    Serial.print(F("-"));
    printlnBCD(binaryToBCDLong(d));
  } else {
    Serial.write(6);
    Serial.print(F("!"));
    Serial.write(i);
    serialWriteLong(d);
  }
}

void printDelayThreshold(){
    if(!gui){
      Serial.println(F("!delayed"));
    } else {
      Serial.write(7);
      Serial.print(F("delayed"));
    }
}

void printHelp(){
  Serial.println(F("Available commands:"));
  Serial.println(F("s - print status in human readable format"));
  Serial.println(F("fs - print full status in human readable format"));
  Serial.println(F("guiE - enable GUI mode. GUI mode means, that informations about temperatures and fan rotations are sending periodically in binary mode"));
  Serial.println(F("guiD - disable GUI mode"));
  Serial.println(F("guistat1 - send fan configurations in binary mode"));
  Serial.println(F("guiUpdate - send temperatures and fan rotations in binary mode"));
  Serial.println(F("setFan param1 param2 param3 param4 param5 param6 param7 param8 - set PWM configuration of fan, all parameters are mandatory"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - select the way of driving fan. 0 - driven by analog input, 1 - constant pwm, 2 - pwm driven by temperatures, 3 - constant rpm, 4 - rpm driven by temperatures"));
  Serial.println(F("    param3 - constant power in case of param2 == 1. Allowed values 0 - 255"));
  Serial.println(F("  next parameters are used in case of param2 == 2"));
  Serial.println(F("    param4 - select temperature source for driving by temperature. 0 - T0, 1 - T1, 2 - average value of T0 and T1"));
  Serial.println(F("    param5 - fan power at target temperature and below target temperature. Allowed values 0 - 255"));
  Serial.println(F("    param6 - fan power at maximum temperature and above maximal temperature. Allowed values 0 - 255"));
  Serial.println(F("    param7 - target temperature. Allowed values 0 - 60"));
  Serial.println(F("    param8 - maximal temperature. Allowed values 0 - 60. Param8 should be bigger than param7"));
  Serial.println(F("setPid param1 param2 param3 param4 param5 param6 param7 param8 param9 - set PID configuration of fan, all parameters are mandatory. Used when fan is driven by PID. Parameter drivingFan is set to 3(constant rpm) or 4 (rpm driven by temperatures)"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - const rpm value, used when parameter drivingFan is set to 3"));
  Serial.println(F("    param3 - rpm value at target temperature and below target temperature"));
  Serial.println(F("    param4 - rpm value at maximum temperature and above maximal temperature"));
  Serial.println(F("    param5 - target temperature. Allowed values 0 - 60"));
  Serial.println(F("    param6 - maximal temperature. Allowed values 0 - 60. Param6 should be bigger than param5"));
  Serial.println(F("    param7 - Kp parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param8 - Ki parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param9 - Kd parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("load - load fan configurations from internal memory"));
  Serial.println(F("save - save fan configurations into internal memory"));
  Serial.println(F("rpm - setRPMToMainboard"));
  Serial.println(F("h param1 - set temperature hysteresis. Allowed values 0(0°C) - 100(10°C)"));
  Serial.println(F("disableFan param1 param2 ... paramN - disable rotation of fan [param1, param3, param5...] for [param2, param4, param6...] seconds"));
  Serial.println(F("cacheStatus - print current state of caches with temperature values computed from thermistors"));
#ifdef TIMING_DEBUG
  Serial.println(F("timing -  - print measured processor time occupied by program"));
  Serial.println(F("mi - print measured processor time occupied by interrupt services"));
#endif
  Serial.println(F("help - this help"));
}

