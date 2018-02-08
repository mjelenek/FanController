byte countPWM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber){
  unsigned int temperatureTarget = conf.tempTarget * 10;
  if(temperature <= temperatureTarget){
    return conf.minPwm;
  }
  unsigned int temperatureMax = conf.tempMax * 10;
  if(temperature >= temperatureMax){
    return conf.maxPwm;
  }
  return ((unsigned long)(temperature - temperatureTarget) * (conf.maxPwm - conf.minPwm)) / (temperatureMax - temperatureTarget) + conf.minPwm;
}

unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber){
  unsigned short temperatureTarget = conf.tempTargetRpm * 10;
  if(temperature <= temperatureTarget){
    return conf.minRpm;
  }
  unsigned short temperatureMax = conf.tempMaxRpm * 10;
  if(temperature >= temperatureMax){
    return conf.maxRpm;
  }
  return (((unsigned long)(temperature - temperatureTarget)) * (conf.maxRpm - conf.minRpm)) / (temperatureMax - temperatureTarget) + conf.minRpm;
}

// 3, 7, 11, 15, 19, 23
#define TIME_TO_COMPUTE_PWM part_32 == ((fanNumber << 2) + 3)
// 4, 8, 12, 16, 20, 24
#define TIME_TO_COMPUTE_PID part_32 == ((fanNumber << 2) + 4)

byte getNewPwm(PWMConfiguration &conf, byte pwm, unsigned short sensorValueAveraged, byte fanNumber){
  if(pwmDisabled[fanNumber] > 0){
    return 0;
  }
  // pwmDrive: 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      return map(sensorValueAveraged, 0, 1023, 0, 255);
    case 1:
      return conf.constPwm;
    case 2:
      // compute once every 32 cycles (64ms)
      if(TIME_TO_COMPUTE_PWM){
    
      // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
        switch (conf.tSelect) {
          case 0:
            if(T0Connected){
              return countPWM(conf, T0WithHysteresisInt, fanNumber);
            }
            return conf.maxPwm;
          case 1:
            if(T1Connected){
              return countPWM(conf, T1WithHysteresisInt, fanNumber);
            }
            return conf.maxPwm;
          case 2:
            if(T0Connected && T1Connected){
              return countPWM(conf, (T0WithHysteresisInt + T1WithHysteresisInt) >> 1, fanNumber);
            }
            if(T0Connected){
              return countPWM(conf, T0WithHysteresisInt, fanNumber);
            }
            if(T1Connected){
              return countPWM(conf, T1WithHysteresisInt, fanNumber);
            }
          return conf.maxPwm;
        }
      } else {
        return pwm;
      }
    case 3:
      // compute once every 32 cycles (64ms)
      if(TIME_TO_COMPUTE_PID){
        setpointPid[fanNumber] = conf.constRpm;
        if(pidCompute(fanNumber)){
          return (byte)outputPid;
        } else {
          return pwm;
        }
      } else {
        return pwm;
      }
    case 4:
      // compute once every 32 cycles (64ms)
      if(TIME_TO_COMPUTE_PWM){
  
        // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
        switch (conf.tSelect) {
          case 0:
            if(T0Connected){
              setpointPid[fanNumber] = countExpectedRPM(conf, T0WithHysteresisInt, fanNumber);
            } else {
              setpointPid[fanNumber] = conf.maxRpm;
            }
            break;
          case 1:
            if(T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(conf, T1WithHysteresisInt, fanNumber);
            } else {
              setpointPid[fanNumber] = conf.maxRpm;
            }
            break;
          case 2:
            if(T0Connected && T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(conf, (T0WithHysteresisInt + T1WithHysteresisInt) >> 1, fanNumber);
              break;
            }
            if(T0Connected){
              setpointPid[fanNumber] = countExpectedRPM(conf, T0WithHysteresisInt, fanNumber);
              break;
            }
            if(T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(conf, T1WithHysteresisInt, fanNumber);
              break;
            }
            setpointPid[fanNumber] = conf.maxRpm;
            break;
          default:
            setpointPid[fanNumber] = conf.maxRpm;
        }
      }
      if(TIME_TO_COMPUTE_PID){
        if(pidCompute(fanNumber)){
          return (byte)outputPid;
        } else {
          return pwm;
        }
      } else {
        return pwm;
      }
    default:
      return pwm;
  }
}

void setPwmi(byte i, unsigned short *sensorValueVolatile){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = *sensorValueVolatile;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[i];
  pwm[i] = getNewPwm(*ConfigurationPWM[i], pwm[i], sensorValue, i);
  if(pwmOld != pwm[i]){
    analogWrite(PWMOUT[i], 255 - pwm[i]);
  }
  pidUpdate(i, *ConfigurationPWM[i]);
}

void setPwm(){
  setPwmi(0, &sensorValue4Averaged);
  setPwmi(1, &sensorValue3Averaged);
  setPwmi(2, &sensorValue2Averaged);
  setPwmi(3, &sensorValue1Averaged);
  setPwmi(4, &sensorValue0Averaged);
  setPwmi(5, &sensorValue0Averaged);
}

void decrementPwmDisabled(){
  for(int i = 0; i <= 5; i++){
    if(pwmDisabled[i] > 0){
      pwmDisabled[i]--;
    }
  }
}

boolean pidCompute(byte fanNumber){
  inputPid = rpm[fanNumber];
  return pid[fanNumber].Compute();
}


