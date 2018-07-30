void serialWriteInt(unsigned int i);

class PWMConfiguration
{
public:
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  byte pwmDrive;
  // number of input connector from motherboard when pwmDrive == 0. Value must be between 0 and NUMBER_OF_MAINBOARD_CONNECTORS
  byte powerInNumber;
  // settings when pwmDrive == 0: map input value to pwm
  unsigned short powerInValue[CURVE_ANALOG_POINTS];
  byte powerInPwm[CURVE_ANALOG_POINTS];
  // pwm when pwmDrive == 1
  byte constPwm;
  // settings when pwmDrive == 2 or pwmDrive == 4
  byte tSelect; //every bit represents one temperature. Count average value of thermistors signed by bit set to 1.
  // settings when pwmDrive == 2: map temperature to pwm
  byte tPwm[CURVE_PWM_POINTS];
  byte pwm[CURVE_PWM_POINTS];
  // settings when pwmDrive == 3
  unsigned short constRpm;
  // settings when pwmDrive == 4: map temperature to rpm
  byte tRpm[CURVE_RPM_POINTS];
  unsigned short rpm[CURVE_RPM_POINTS];
  // pid parameters, real value is parameter / 200
  byte kp;
  byte ki;
  byte kd;
  byte minPidPwm; // minimal value of PWM when speed is driven by PID. Can not be 0, because fans with pwm power=0 instantly indicates 0rpm.

  void Reset()
  {
    pwmDrive = 1;
    powerInNumber = 0;
    tSelect = 1;
    powerInValue[0] = 0;
    powerInPwm[0] = 0;
    powerInValue[1] = 1023;
    powerInPwm[1] = 255;
    powerInValue[0] = 0;
    constPwm = 120;
    powerInPwm[0] = 0;
    tPwm[0] = 32;
    pwm[0] = 90;
    tPwm[1] = 50;
    pwm[1] = 200;
    constRpm = 700;
    tRpm[0] = 32;
    rpm[0] = 600;
    tRpm[1] = 50;
    rpm[1] = 1400;
    // PID parameters (real value is parameter value / 200)
    kp = 40;
    ki = 30;
    kd = 5;
    minPidPwm = 20;
  }

  void set(byte pwmDrive1, byte powerInNumber1, byte constPwm1, byte tSelect1, unsigned short constRpm1, byte kp1, byte ki1, byte kd1, byte minPidPwm1)
  {
    if(powerInNumber1 >= 0 && powerInNumber1 < NUMBER_OF_MAINBOARD_CONNECTORS){
      powerInNumber = powerInNumber1;
    }
    if(pwmDrive1 >= 0 && pwmDrive1 <= 4){
      pwmDrive = pwmDrive1;
    }
    constPwm = constPwm1;
    if(tSelect1 >= 0 && tSelect1 < (1 << NUMBER_OF_THERMISTORS)){
      tSelect = tSelect1;
    }
    constRpm = constRpm1;
    kp = kp1;
    ki = ki1;
    kd = kd1;
    minPidPwm = minPidPwm1;
  }

  void setPowerInCurve(byte count, unsigned short pInNew[], byte pwmNew[]){
    if(count > CURVE_ANALOG_POINTS) return;
    for(byte i = 0; i < CURVE_ANALOG_POINTS; i++){
      powerInValue[i] = 0;
      powerInPwm[i] = 0;
    }
    byte lastPowerIn = 0;
    for(byte i = 0; i < count; i++){
      if(pInNew[i] <= 1023 && lastPowerIn <= pInNew[i]){
        lastPowerIn = pInNew[i];
        powerInValue[i] = pInNew[i];
        powerInPwm[i] = pwmNew[i];
      }
    }
  }


  void setPWMCurve(byte count, byte tNew[], byte pwmNew[]){
    if(count > CURVE_PWM_POINTS) return;
    for(byte i = 0; i < CURVE_PWM_POINTS; i++){
      tPwm[i] = 0;
      pwm[i] = 0;
    }
    byte lastT = 0;
    for(byte i = 0; i < count; i++){
      if(tNew[i] <= MAX_ALLOWED_TEMP && lastT <= tNew[i]){
        lastT = tNew[i];
        tPwm[i] = tNew[i];
        pwm[i] = pwmNew[i];
      }
    }
  }

  void setRPMCurve(byte count, byte tNew[], unsigned short rpmNew[]){
    if(count > CURVE_RPM_POINTS) return;
    for(byte i = 0; i < CURVE_RPM_POINTS; i++){
      tRpm[i] = 0;
      rpm[i] = 0;
    }
    byte lastT = 0;
    for(byte i = 0; i < count; i++){
      if(tNew[i] <= MAX_ALLOWED_TEMP && lastT <= tNew[i]){
        lastT = tNew[i];
        tRpm[i] = tNew[i];
        rpm[i] = rpmNew[i];
      }
    }
  }

  void guiStat(){
    Serial.write(pwmDrive);
    Serial.write(powerInNumber);
    for(byte i = 0; i < CURVE_ANALOG_POINTS; i++){
      serialWriteInt(powerInValue[i]);
      Serial.write(powerInPwm[i]);
    }
    Serial.write(constPwm);
    Serial.write(tSelect);
    for(byte i = 0; i < CURVE_PWM_POINTS; i++){
      Serial.write(tPwm[i]);
      Serial.write(pwm[i]);
    }
    serialWriteInt(constRpm);
    for(byte i = 0; i < CURVE_RPM_POINTS; i++){
      Serial.write(tRpm[i]);
      serialWriteInt(rpm[i]);
    }
    Serial.write(kp);
    Serial.write(ki);
    Serial.write(kd);
    Serial.write(minPidPwm);
  }
};

class ThermistorDefinition
{
  public:
  // temp. for nominal resistance (almost always 25 C)
  unsigned char tempNominal;
  // resistance at nominal temperature
  unsigned short resistanceNominal;
  // The beta coefficient of the thermistor (usually 3000-4000)
  unsigned short bCoefficient;

  void Reset()
  {
    tempNominal = 25;
    resistanceNominal = 10000;
    bCoefficient = 3850;
  }

  void Set(unsigned char t, unsigned short r, unsigned short b){
    if(t < 0 || t >= 100) return;
    if(r < 3000 || r >= 50000) return;
    if(b < 1000 || b >= 10000) return;
    tempNominal = t;
    resistanceNominal = r;
    bCoefficient = b;
  }

  void sendDefinition(){
    Serial.write(tempNominal);
    serialWriteInt(resistanceNominal);
    serialWriteInt(bCoefficient);
  }
};

template <byte numberOfThermistors, byte numberOfRpmToMainboard> class ControllerConfiguration
{
public:
  //settings profile
  byte profile;
  // sensor to mainboard
  byte rmpToMainboard[numberOfRpmToMainboard];
  // hysteresis * 10°C -> value 10 means +- 1°C
  byte hysteresis;
  //Thermistor definitions
  ThermistorDefinition thermistors[numberOfThermistors];
  
  void Reset()
  {
    profile = 0;
    hysteresis = 10;
    for(byte i = 0; i < numberOfRpmToMainboard; i++){
      rmpToMainboard[i] = i;
    }
    for(byte i = 0; i < numberOfThermistors; i++){
      thermistors[i].Reset();
    }
  }
};


