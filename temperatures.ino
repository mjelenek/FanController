void countT0(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValueAveraged = sensorValue6Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt

  T0Connected = (sensorValueAveraged > 10);
  if(T0Connected == true){
    T0int = cacheT0.get(sensorValueAveraged);
    if(T0int == 0){
      T0int = countTemperature(RT0koeficient / sensorValueAveraged - RT0);
      cacheT0.put(sensorValueAveraged, T0int);
    }
    T0WithHysteresisInt = countHysteresisTemperature(T0WithHysteresisInt, T0int);
  } else {
    T0int = 0;
  }
}

void countT1(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValueAveraged = sensorValue7Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt

  T1Connected = (sensorValueAveraged > 10);
  if(T1Connected == true){
    T1int = cacheT1.get(sensorValueAveraged);
    if(T1int == 0){
      T1int = countTemperature(RT1koeficient / sensorValueAveraged - RT1);
      cacheT1.put(sensorValueAveraged, T1int);
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

