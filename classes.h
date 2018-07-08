void serialWriteInt(unsigned int i);

class PWMConfiguration
{
public:
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  byte pwmDrive;
  // number of input connector from motherboard when pwmDrive == 0. Value must be between 0 and NUMBER_OF_MAINBOARD_CONNECTORS
  byte powerInNumber;
  // settings when pwmDrive == 0: map input value to pwm
  //byte powerInValue[5];
  //byte powerInPwm[5];
  // pwm when pwmDrive == 1
  byte constPwm;
  // settings when pwmDrive == 2 or pwmDrive == 4
  byte tSelect; // select temperature sensor 0 - T0, 1 - T1, 2  - (T1+T2)/2
  // settings when pwmDrive == 2: map temperature to pwm
  byte tPwm[5];
  byte pwm[5];
  // settings when pwmDrive == 3
  unsigned short constRpm;
  // settings when pwmDrive == 4: map temperature to rpm
  byte tRpm[5];
  unsigned short rpm[5];
  // pid parameters, real value is parameter / 200
  byte kp;
  byte ki;
  byte kd;
  byte minPidPwm; // minimal value of PWM when speed is driven by PID. Can not be 0, because fans with pwm power=0 instantly indicates 0rpm.

  void Reset()
  {
    pwmDrive = 1;
    powerInNumber = 0;
    constPwm = 120;
    tSelect = 0;
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
    minPidPwm = 15;
  }

  void set(byte pwmDrive1, byte powerInNumber1, byte constPwm1, byte tSelect1,
    byte t0, byte pwm0,
    byte t1, byte pwm1,
    byte t2, byte pwm2,
    byte t3, byte pwm3,
    byte t4, byte pwm4)
  {
    if(powerInNumber1 >= 0 && powerInNumber1 < NUMBER_OF_MAINBOARD_CONNECTORS){
      powerInNumber = powerInNumber1;
    }
    if(pwmDrive1 >= 0 && pwmDrive1 <= 4){
      pwmDrive = pwmDrive1;
    }
    constPwm = constPwm1;
    if(tSelect1 >= 0 && tSelect1 <= 2){
      tSelect = tSelect1;
    }
    if(t0 <= 60 && t0 <= t1){
      tPwm[0] = t0;
      pwm[0] = pwm0;
    }
    if(t1 <= 60 && t0 <= t1){
      tPwm[1] = t1;
      pwm[1] = pwm1;
    }
    tPwm[2] = tPwm[3] = tPwm[4] = 0;
    pwm[2] = pwm[3] = pwm[4] = 0;
    
    if(t2 <= 60 && (t1 <= t2)){
      tPwm[2] = t2;
      pwm[2] = pwm2;
    } else {
      return;
    }
    if(t3 <= 60 && (t2 <= t3)){
      tPwm[3] = t3;
      pwm[3] = pwm3;
    } else {
      return;
    }
    if(t4 <= 60 && (t3 <= t4)){
      tPwm[4] = t4;
      pwm[4] = pwm4;
    }
  }

  void setPid(unsigned short constRpm1, byte kp1, byte ki1, byte kd1, byte minPidPwm1,
    byte t0, unsigned short rpm0,
    byte t1, unsigned short rpm1,
    byte t2, unsigned short rpm2,
    byte t3, unsigned short rpm3,
    byte t4, unsigned short rpm4)
  {
    constRpm = constRpm1;
    kp = kp1;
    ki = ki1;
    kd = kd1;
    minPidPwm = minPidPwm1;
    if(t0 <= 60 && t0 <= t1){
      tRpm[0] = t0;
      rpm[0] = rpm0;
    }
    if(t0 <= 60 && t0 <= t1){
      tRpm[1] = t1;
      rpm[1] = rpm1;
    }
    tRpm[2] = tRpm[3] = tRpm[4] = 0;
    rpm[2] = rpm[3] = rpm[4] = 0;
    
    if(t2 <= 60 && t1 <= t2){
      tRpm[2] = t2;
      rpm[2] = rpm2;
    } else {
      return;
    }
    if(t3 <= 60 && t2 <= t3){
      tRpm[3] = t3;
      rpm[3] = rpm3;
    } else {
      return;
    }
    if(t4 <= 60 && t3 <= t4){
      tRpm[4] = t4;
      rpm[4] = rpm4;
    }
  }

  void guiStat(){
    Serial.write(pwmDrive);
    Serial.write(powerInNumber);
    Serial.write(constPwm);
    Serial.write(tSelect);
    for(byte i = 0; i <= 4; i++){
      Serial.write(tPwm[i]);
      Serial.write(pwm[i]);
    }
    serialWriteInt(constRpm);
    for(byte i = 0; i <= 4; i++){
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
  // sensor to mainboard
  byte rmpToMainboard[numberOfRpmToMainboard];
  // hysteresis * 10°C -> value 10 means +- 1°C
  byte hysteresis;
  //Thermistor definitions
  ThermistorDefinition thermistors[numberOfThermistors];
  
  void Reset()
  {
    rmpToMainboard[0] = 5;
    hysteresis = 10;
    for(byte i = 0; i < numberOfThermistors; i++){
      thermistors[i].Reset();
    }
  }
};


