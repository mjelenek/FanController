byte countPWM(unsigned int temperature, unsigned int temperatureTarget, unsigned int temperatureMax, byte pwmMin, byte pwmMax){
  temperatureTarget = temperatureTarget  * 10;
  if(temperature < temperatureTarget){
    return pwmMin;
  }
  temperatureMax = temperatureMax * 10;
  if(temperature > temperatureMax){
    return pwmMax;
  }
  return (temperature - temperatureTarget) * (pwmMax - pwmMin) / (temperatureMax - temperatureTarget) + pwmMin;
}

byte getNewPwm(PWMConfiguration &conf, byte pwm, unsigned int sensorValueAveraged){
  // pwmDrive: 0 - const, 1 - analogInput, 2 - temperatures
  switch (conf.pwmDrive) {
    case 0:
      return conf.constPwm;
    case 1:
      return map(sensorValueAveraged, 0, 1023, 0, 255);
    case 2:
  // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
      switch (conf.tSelect) {
        case 0:
          if(T0Connected){
            return countPWM(T0int, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
          }
          return conf.maxPwm;
        case 1:
          if(T1Connected){
            return countPWM(T1int, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
          }
          return conf.maxPwm;
        case 2:
          if(T0Connected && T1Connected){
            return countPWM(T0int + T1int, conf.tempTarget << 1, conf.tempMax << 1, conf.minPwm, conf.maxPwm);
          }
          if(T0Connected){
            return countPWM(T0int, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
          }
          if(T1Connected){
            return countPWM(T1int, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
          }
        return conf.maxPwm;
      }
    default:
    return pwm;
  }
}

void setPwm0(){
  if(pwm0Disabled > 0){
    pwm0 = 0;
  } else {
    pwm0 = getNewPwm(ConfigurationPWM0.Data, pwm0, sensorValue0Averaged);
  }
  analogWrite(PWM0, 255 - pwm0);
}

void setPwm1(){
  if(pwm1Disabled > 0){
    pwm1 = 0;
  } else {
    pwm1 = getNewPwm(ConfigurationPWM1.Data, pwm1, sensorValue1Averaged);
  }
  analogWrite(PWM1, 255 - pwm1);
}

void setPwm2(){
  if(pwm2Disabled > 0){
    pwm2 = 0;
  } else {
    pwm2 = getNewPwm(ConfigurationPWM2.Data, pwm2, sensorValue2Averaged);
  }
  analogWrite(PWM2, 255 - pwm2);
}

void setPwm3(){
  if(pwm3Disabled > 0){
    pwm3 = 0;
  } else {
    pwm3 = getNewPwm(ConfigurationPWM3.Data, pwm3, sensorValue3Averaged);
  }
  analogWrite(PWM3, 255 - pwm3);
}

void setPwm4(){
  if(pwm4Disabled > 0){
    pwm4 = 0;
  } else {
    pwm4 = getNewPwm(ConfigurationPWM4.Data, pwm4, sensorValue4Averaged);
  }
  analogWrite(PWM4, 255 - pwm4);
}

void setPwm5(){
  if(pwm5Disabled > 0){
    pwm5 = 1;
  } else {
    pwm5 = getNewPwm(ConfigurationPWM5.Data, pwm5, sensorValue4Averaged);
    if(pwm5 == 0){
      analogWrite(PWM5, 1);
      return;
    }
    if(pwm5 == 255){
      analogWrite(PWM5, 254);
      return;
    }
  }
  analogWrite(PWM5, pwm5);
}

void setPwm(){
    setPwm0();
    setPwm1();
    setPwm2();
    setPwm3();
    setPwm4();
    setPwm5();
}

void decrementPwmDisabled(){
  if(pwm0Disabled > 0){
    pwm0Disabled--;
  }
  if(pwm1Disabled > 0){
    pwm1Disabled--;
  }
  if(pwm2Disabled > 0){
    pwm2Disabled--;
  }
  if(pwm3Disabled > 0){
    pwm3Disabled--;
  }
  if(pwm4Disabled > 0){
    pwm4Disabled--;
  }
  if(pwm5Disabled > 0){
    pwm5Disabled--;
  }
}


