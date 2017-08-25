void readT0(){
  sensorValue6Averaged = readAnalogValueAndSmooth(sensorValue6Averaged, TEMPINPUT0);
  if(sensorValue6Averaged > 10){
    T0Connected = true;
    thermistorResistance0 = RT0koeficient / sensorValue6Averaged - RT0;
  } else {
    T0Connected = false;
  }
}

void readT1(){
    sensorValue7Averaged = readAnalogValueAndSmooth(sensorValue7Averaged, TEMPINPUT1);
    if(sensorValue7Averaged > 10){
      T1Connected = true;
      thermistorResistance1 = RT1koeficient / sensorValue7Averaged - RT1;
    } else {
      T1Connected = false;
    }
}

void countT0(){
  if(T0Connected == true){
    short result = cacheT0.get(sensorValue6Averaged);
    if(result != 0){
       T0int = result;
    } else {
      T0int = countTemperature(thermistorResistance0);
      cacheT0.put(sensorValue6Averaged, T0int);
    }
  } else {
    T0int = 0;
  }
}

void countT1(){
  if(T1Connected == true){
    short result = cacheT1.get(sensorValue7Averaged);
    if(result != 0){
      T1int = result;
    } else {
      T1int = countTemperature(thermistorResistance1);
      cacheT1.put(sensorValue7Averaged, T1int);
    }
  } else {
    T1int = 0;
  }
}

unsigned int countTemperature(unsigned long thermistorResistance){
  float steinhart2;
  steinhart2 = (float)thermistorResistance / THERMISTORNOMINAL;     // (R/Ro)
  steinhart2 = log(steinhart2);                  // ln(R/Ro)
  steinhart2 /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart2 += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart2 = 1.0 / steinhart2;                 // Invert
  steinhart2 -= 273.15;                         // convert to C
  return (unsigned int)(steinhart2 * 10);
}


