byte countPWM(PWMConfiguration &conf, unsigned int input){
  if(conf.pwmDrive == 0){
    byte partSelect = getPowerInPartSelect(conf.powerInValue, input);
    unsigned short powerInMin = conf.powerInValue[partSelect];
    if(input <= powerInMin){
      return conf.powerInPwm[partSelect];
    }
    unsigned short powerInMax = conf.powerInValue[partSelect + 1];
    if(input >= powerInMax){
      return conf.powerInPwm[partSelect + 1];
    }
    return ((long)(input - powerInMin) * ((int)conf.powerInPwm[partSelect + 1] - (int)conf.powerInPwm[partSelect])) / (powerInMax - powerInMin) + conf.powerInPwm[partSelect];
  }
  if(conf.pwmDrive == 2){
    byte partSelect = getTemperaturePartSelect(conf.tPwm, input, CURVE_PWM_POINTS);
    unsigned int temperatureTarget = conf.tPwm[partSelect] * 10;
    if(input <= temperatureTarget){
      return conf.pwm[partSelect];
    }
    unsigned int temperatureMax = conf.tPwm[partSelect + 1] * 10;
    if(input >= temperatureMax){
      return conf.pwm[partSelect + 1];
    }
    return ((long)(input - temperatureTarget) * ((int)conf.pwm[partSelect + 1] - (int)conf.pwm[partSelect])) / (temperatureMax - temperatureTarget) + conf.pwm[partSelect];
  }
  return conf.constPwm;
}

#ifdef USE_PWM_CACHE
byte countPWM(PWMConfiguration &conf, unsigned int input, byte fanNumber){
    byte pwm = cacheFan[fanNumber].get(input);
    if(pwm == 0){
      pwm = countPWM(conf, input);
      cacheFan[fanNumber].put(input, pwm);
    }
  return pwm;
}
#endif

unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature){
  byte partSelect = getTemperaturePartSelect(conf.tRpm, temperature, CURVE_RPM_POINTS);
  unsigned short temperatureTarget = conf.tRpm[partSelect] * 10;
  if(temperature <= temperatureTarget){
    return conf.rpm[partSelect];
  }
  unsigned short temperatureMax = conf.tRpm[partSelect + 1] * 10;
  if(temperature >= temperatureMax){
    return conf.rpm[partSelect + 1];
  }
  return (((long)(temperature - temperatureTarget)) * ((int)conf.rpm[partSelect + 1] - (int)conf.rpm[partSelect])) / (temperatureMax - temperatureTarget) + conf.rpm[partSelect];
}

#ifdef USE_PWM_CACHE
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber){
    unsigned short rpm = cacheFan[fanNumber].get(temperature);
    if(rpm == 0){
      rpm = countExpectedRPM(conf, temperature);
      cacheFan[fanNumber].put(temperature, rpm);
    }
  return rpm;
}
#endif

// 3, 7, 11, 15, 19, 23 (... 27, 31, 35, 39, 43, 47, 51, 53)
#define TIME_TO_COMPUTE_PWM part_64 == ((fanNumber << 2) + 3)
// 4, 8, 12, 16, 20, 24 (... 28, 32, 36, 40, 44, 48, 52, 54)
#define TIME_TO_COMPUTE_PWM_BY_PID part_64 == ((fanNumber << 2) + 4)

void setPwm(byte fanNumber){
  unsigned short sensorValue;
  byte pwmOld = pwm[fanNumber];
  if(pwmDisabled[fanNumber] == 0){
    PWMConfiguration &conf = ConfigurationPWM(fanNumber);
    // pwmDrive: 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
    switch (conf.pwmDrive) {
      case 0:
        ADCSRA &= ~(1 << ADIE);               // Disable ADC conversion complete interrupt
        sensorValue = powerInADCAveraged[conf.powerInNumber];
        ADCSRA |= (1 << ADIE);                // Enable ADC conversion complete interrupt
        
        pwm[fanNumber] = countPWM(conf, sensorValue USE_FAN_NUMBER);
        break;
      case 1:
      case 2:
        if(TIME_TO_COMPUTE_PWM){
          pwm[fanNumber] = countPWM(conf, countEffectiveTemperature(conf.tSelect) USE_FAN_NUMBER);  
        }
        break;
      case 3:
        if(TIME_TO_COMPUTE_PWM_BY_PID){
          pwm[fanNumber] = getNewPwmByConstRpm(conf, pwmOld, fanNumber);
        }
        break;
      case 4:
        if(TIME_TO_COMPUTE_PWM){
          setpointPid[fanNumber] = countExpectedRPM(conf, countEffectiveTemperature(conf.tSelect) USE_FAN_NUMBER);
        }
        if(TIME_TO_COMPUTE_PWM_BY_PID){
          if(pidCompute(fanNumber)){
             pwm[fanNumber] = (byte)outputPid;
          }
        }
        break;
    }
  } else {
    pwm[fanNumber] = 0;
  }

  if(pwmOld != pwm[fanNumber]){
    writeAnalogValue(PWMOUT(fanNumber), 255 - pwm[fanNumber]);
  }
  pidUpdate(fanNumber, ConfigurationPWM(fanNumber));
}

byte getNewPwmByConstRpm(PWMConfiguration &conf, byte pwmOld, byte fanNumber){
  setpointPid[fanNumber] = conf.constRpm;
  if(pidCompute(fanNumber)){
    return (byte)outputPid;
  }
  return pwmOld;
}

byte getTemperaturePartSelect(byte temperatures[], unsigned int temperature, byte len){
  byte i = 0;
  while (i < (len - 2) && temperatures[i+2] != 0 && ((unsigned short)temperatures[i+1] * 10) <= temperature){
    i++;
  }
  return i;
}

byte getPowerInPartSelect(unsigned short powerInPoints[], unsigned short powerIn){
  byte i = 0;
  while (i < (CURVE_ANALOG_POINTS - 2) && powerInPoints[i+2] != 0 && powerInPoints[i+1] <= powerIn){
    i++;
  }
  return i;
}

void setPwm(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    setPwm(i);
  }
}

void decrementPwmDisabled(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    if(pwmDisabled[i] > 0){
      pwmDisabled[i]--;
    }
  }
}

boolean pidCompute(byte fanNumber){
  inputPid = rpm[fanNumber];
  return pid[fanNumber].Compute();
}


