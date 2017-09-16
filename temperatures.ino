void countT0(){
  sensorValue6Averaged = readAnalogValueAndSmooth(sensorValue6Averaged, TEMPINPUT0);
  T0Connected = (sensorValue6Averaged > 10);
  if(T0Connected == true){
    T0int = cacheT0.get(sensorValue6Averaged);
    if(T0int == 0){
      T0int = countTemperature(RT0koeficient / sensorValue6Averaged - RT0);
      cacheT0.put(sensorValue6Averaged, T0int);
    }
    T0WithHysteresisInt = countHysteresisTemperature(T0WithHysteresisInt, T0int);
  } else {
    T0int = 0;
  }
}

void countT1(){
  sensorValue7Averaged = readAnalogValueAndSmooth(sensorValue7Averaged, TEMPINPUT1);
  T1Connected = (sensorValue7Averaged > 10);
  if(T1Connected == true){
    T1int = cacheT1.get(sensorValue7Averaged);
    if(T1int == 0){
      T1int = countTemperature(RT1koeficient / sensorValue7Averaged - RT1);
      cacheT1.put(sensorValue7Averaged, T1int);
    }
    T1WithHysteresisInt = countHysteresisTemperature(T1WithHysteresisInt, T1int);
  } else {
    T1int = 0;
  }
}

int countTemperature(unsigned long thermistorResistance){
  float steinhart2;
  steinhart2 = (float)thermistorResistance / THERMISTORNOMINAL;     // (R/Ro)
  steinhart2 = log(steinhart2);                                     // ln(R/Ro)
  steinhart2 /= BCOEFFICIENT;                                       // 1/B * ln(R/Ro)
  steinhart2 += 1.0 / (TEMPERATURENOMINAL + 273.15);                // + (1/To)
  steinhart2 = 1.0 / steinhart2;                                    // Invert
  steinhart2 -= 273.15;                                             // convert to C
  return (int)(steinhart2 * 10);
}

int countHysteresisTemperature(int tWithHysteresisInt, int tInt){
  if((tWithHysteresisInt - tInt) < HYSTERESIS && (tInt - tWithHysteresisInt) < HYSTERESIS){
    return tWithHysteresisInt;
  }
  if(tInt > tWithHysteresisInt){
    return(tInt - HYSTERESIS);
  }
  return(tInt + HYSTERESIS);
}

