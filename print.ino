void printStatus(){
  Serial.print(F("T0:"));
  if (T0Connected){
    Serial.print(T0int / 10);
    Serial.print(F("."));
    Serial.print(T0int % 10);
  } else {
    Serial.print(F("N/A"));
  }
  Serial.print(F(" T1:"));
  if (T1Connected){
    Serial.print(T1int / 10);
    Serial.print(F("."));
    Serial.println(T1int % 10);
  } else {
    Serial.println(F("N/A"));
  }
  Serial.print(F("RPM0:"));
  Serial.print(roundRPM(rpm[0]));
  Serial.print(F(" RPM1:"));
  Serial.print(roundRPM(rpm[1]));
  Serial.print(F(" RPM2:"));
  Serial.print(roundRPM(rpm[2]));
  Serial.print(F(" RPM3:"));
  Serial.print(roundRPM(rpm[3]));
  Serial.print(F(" RPM4:"));
  Serial.print(roundRPM(rpm[4]));
  Serial.print(F(" RPM5:"));
  Serial.println(roundRPM(rpm[5]));
}

void printFullStatus(){
  printStatus();
  Serial.print(F("PWM0:"));
  Serial.print(pwm[0]);
  Serial.print(F(" PWM1:"));
  Serial.print(pwm[1]);
  Serial.print(F(" PWM2:"));
  Serial.print(pwm[2]);
  Serial.print(F(" PWM3:"));
  Serial.print(pwm[3]);
  Serial.print(F(" PWM4:"));
  Serial.print(pwm[4]);
  Serial.print(F(" PWM5:"));
  Serial.println(pwm[5]);
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
  Serial.println(*hysteresis);
}  
  

void printlnPwmDrive(PWMConfiguration &conf){
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      Serial.println(F("analog input"));
      break;
    case 1:
      Serial.print(F("constant power, PWM="));
      Serial.println(conf.constPwm);
      break;
    case 2:
      Serial.print(F("PWM by temperature, t0="));
      Serial.print(conf.tPwm[0]);
      Serial.print(F(", pwm0="));
      Serial.print(conf.pwm[0]);
      Serial.print(F(" t1="));
      Serial.print(conf.tPwm[1]);
      Serial.print(F(", pwm1="));
      Serial.print(conf.pwm[1]);
      Serial.print(F(" t2="));
      Serial.print(conf.tPwm[2]);
      Serial.print(F(", pwm2="));
      Serial.print(conf.pwm[2]);
      Serial.print(F(" t3="));
      Serial.print(conf.tPwm[3]);
      Serial.print(F(", pwm3="));
      Serial.print(conf.pwm[3]);
      Serial.print(F(" t4="));
      Serial.print(conf.tPwm[4]);
      Serial.print(F(", pwm4="));
      Serial.println(conf.pwm[4]);
      break;
    case 3:
      Serial.print(F("constant speed, expected RPM="));
      Serial.println(conf.constRpm);
      break;
    case 4:
      Serial.print(F("RPM by temperature, t0="));
      Serial.print(conf.tRpm[0]);
      Serial.print(F(", rpm0="));
      Serial.print(conf.rpm[0]);
      Serial.print(F(" t1="));
      Serial.print(conf.tRpm[1]);
      Serial.print(F(", rpm1="));
      Serial.print(conf.rpm[1]);
      Serial.print(F(" t2="));
      Serial.print(conf.tRpm[2]);
      Serial.print(F(", rpm2="));
      Serial.print(conf.rpm[2]);
      Serial.print(F(" t3="));
      Serial.print(conf.tRpm[3]);
      Serial.print(F(", rpm3="));
      Serial.print(conf.rpm[3]);
      Serial.print(F(" t4="));
      Serial.print(conf.tRpm[4]);
      Serial.print(F(", rpm4="));
      Serial.println(conf.rpm[4]);
      break;
  }
}

void printDelay(byte i, unsigned long d){
  if(!gui){
    Serial.print(F("!"));
    Serial.print(i);
    Serial.print(F("-"));
    Serial.println(d);
  } else {
    Serial.print(F("!"));
    Serial.write(10);
    Serial.print(F("delay"));
    Serial.write(i);
    serialWriteLong(d);
    Serial.print(F("#"));
  }
}

void printDelayThreshold(){
    if(!gui){
      Serial.println(F("!delayed"));
    } else {
    Serial.print(F("!"));
    Serial.write(7);
    Serial.print(F("delayed"));
    Serial.print(F("#"));
    }
}

void printHelp(){
  Serial.println(F("Available commands:"));
  Serial.println(F("s - print status in human readable format"));
  Serial.println(F("fs - print full status in human readable format"));
  Serial.println(F("guiE - enable GUI mode. GUI mode means, that informations about temperatures and fan rotations are sending periodically in binary mode"));
  Serial.println(F("guiD - disable GUI mode"));
  Serial.println(F("guistat1 - send fan configurations of fan0-fan2 in binary mode"));
  Serial.println(F("guistat2 - send fan configurations of fan3-fan5 in binary mode"));
  Serial.println(F("    message format of guistat1 and guistat2(character \"|\" is just a separator here) - \"!\"(starting char) | (byte)106(length of message) | \"gui1\" | fan0Status | fan1Status | fan2Status | \"#\"(ending char)"));
  Serial.println(F("    ... where fanXStatus is - (byte)driving | (byte)constPwm | (byte)temperature select | (byte)tPwm0 | (byte)pwm0 | (byte)tPwm1 | (byte)pwm1 | (byte)tPwm2 | (byte)pwm2 | (byte)tPwm3 | (byte)pwm3 | (byte)tPwm4 | (byte)pwm4 | (int)constRpm | (byte)tRpm0 | (int)rpm0 | (byte)tRpm1 | (int)rpm1 | (byte)tRpm2 | (int)rpm2 | (byte)tRpm3 | (int)rpm3 | (byte)tRpm4 | (int)rpm4 | (byte)kp | (byte)ki | (byte)kd | (byte)minPidPwm"));
  Serial.println(F("    ... (byte) means value in one byte, (int) means value in two bytes - lowByte | highByte"));
  Serial.println(F("guiUpdate - send temperatures and fan rotations in binary mode"));
  Serial.println(F("    message format of guiUpdate(character \"|\" is just a separator here) - \"!\"(starting char) | (byte)24(length of message) | \"guiU\" | (int)rpm0 | (int)rpm1 | (int)rpm2 | (int)rpm3 | (int)rpm4 | (int)rpm5 | (int)T0*10 | (int)T1*10 | (int)T0WithHysteresis*10 | (int)T1WithHysteresis*10 | \"#\"(ending char)"));
  Serial.println(F("setFan param1 ... param14 - set PWM configuration of fan, parameters param1 - param8 are mandatory"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - select the way of driving fan. 0 - driven by analog input, 1 - constant pwm, 2 - pwm driven by temperatures, 3 - constant rpm, 4 - rpm driven by temperatures"));
  Serial.println(F("    param3 - constant power in case of param2 == 1. Allowed values 0 - 255"));
  Serial.println(F("  next parameters are used in case of param2 == 2"));
  Serial.println(F("    param4 - select temperature source for driving by temperature. 0 - T0, 1 - T1, 2 - average value of T0 and T1"));
  Serial.println(F("    param5 - temperature t0. Allowed values 0 - 60"));
  Serial.println(F("    param6 - fan power at temperature t0 and below temperature t0. Allowed values 0 - 255"));
  Serial.println(F("    param7 - temperature t1. Allowed values 0 - 60, t1 shall be bigger than t0"));
  Serial.println(F("    param8 - fan power at temperature t1, and above temperature t1 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param9 - temperature t2. Allowed values 0 - 60, t2 shall be bigger than t1"));
  Serial.println(F("    param10 - fan power at temperature t2, and above temperature t2 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param11 - temperature t3. Allowed values 0 - 60, t3 shall be bigger than t2"));
  Serial.println(F("    param12 - fan power at temperature t3, and above temperature t3 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param13 - temperature t4. Allowed values 0 - 60, t4 shall be bigger than t3"));
  Serial.println(F("    param14 - fan power at temperature t4, and above temperature t4 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("setPid param1 ... param16 - set PID configuration of fan, parameters param1 - param10 are mandatory. Used when fan is driven by PID. Parameter drivingFan is set to 3(constant rpm) or 4 (rpm driven by temperatures)"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - const rpm value, used when parameter drivingFan is set to 3"));
  Serial.println(F("    param3 - Kp parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param4 - Ki parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param5 - Kd parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param6 - minPidPwm parameter of PID. Allowed values 0 - 255. When fan is driven by PID, pwm value is in range minPidPwm - 255"));
  Serial.println(F("    param7 - temperature t0. Allowed values 0 - 60"));
  Serial.println(F("    param8 - rpm value at temperature t0 and below t0"));
  Serial.println(F("    param9 - temperature t1. Allowed values 0 - 60, t1 shall be bigger than t0"));
  Serial.println(F("    param10 - rpm value at temperature t1, and above temperature t1 if next temperature is not defined"));
  Serial.println(F("    param11 - temperature t2. Allowed values 0 - 60, t2 shall be bigger than t1"));
  Serial.println(F("    param12 - rpm value at temperature t2, and above temperature t2 if next temperature is not defined"));
  Serial.println(F("    param13 - temperature t3. Allowed values 0 - 60, t3 shall be bigger than t2"));
  Serial.println(F("    param14 - rpm value at temperature t3, and above temperature t3 if next temperature is not defined"));
  Serial.println(F("    param15 - temperature t4. Allowed values 0 - 60, t4 shall be bigger than t3"));
  Serial.println(F("    param16 - rpm value at temperature t4, and above temperature t4 if next temperature is not defined"));
  Serial.println(F("load - load fan configurations from internal memory"));
  Serial.println(F("save - save fan configurations into internal memory"));
  Serial.println(F("rpm - setRPMToMainboard"));
  Serial.println(F("h param1 - set temperature hysteresis. Allowed values 0(0°C) - 100(10°C)"));
  Serial.println(F("disableFan param1 param2 ... paramN - disable rotation of fan [param1, param3, param5...] for [param2, param4, param6...] seconds"));
  Serial.println(F("cacheStatus - print current state of caches with temperature values computed from thermistors"));
#ifdef TIMING_DEBUG
  Serial.println(F("time -  - print time from start - microsecond, milliseconds, seconds"));
  Serial.println(F("timing -  - print measured processor time occupied by program"));
  Serial.println(F("mi - print measured processor time occupied by interrupt services"));
#endif
#ifdef FREE_MEMORY_DEBUG
  Serial.println(F("freemem - print lowest measured free memory over the stack"));
#endif
  Serial.println(F("help - this help"));
}

#ifdef TIMING_DEBUG
void printTimingResult(){
  if(!gui){
    Serial.print(F("Time in code: "));
    Serial.print(100 * (float)timeInCode / (float)timeTotal, 2);
    Serial.println(F("%"));
    Serial.print(F("Average delay: "));
    Serial.println((unsigned long)timeInCode >> 9);
    Serial.print(F("<400-"));
    Serial.println(to400);
    Serial.print(F("<600-"));
    Serial.println(to600);
    Serial.print(F("<800-"));
    Serial.println(to800);
    if(to1000 > 0){
      Serial.print(F("<1000-"));
      Serial.println(to1000);
    }
    if(to1200 > 0){
      Serial.print(F("<1200-"));
      Serial.println(to1200);
    }
    if(over1200 > 0){
      Serial.print(F(">1200-"));
      Serial.println(over1200);
    }
  } else {
    Serial.print(F("!"));
    Serial.write(24);
    Serial.print(F("timing"));
    serialWriteInt((unsigned int)(10000 * (float)timeInCode / (float)timeTotal));
    serialWriteLong((unsigned long)timeInCode >> 9);
    serialWriteInt(to400);
    serialWriteInt(to600);
    serialWriteInt(to800);
    serialWriteInt(to1000);
    serialWriteInt(to1200);
    serialWriteInt(over1200);
    Serial.print(F("#"));
  }
}
#endif


