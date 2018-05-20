#ifndef TEMPERATURES_DEBUG
void countT0(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValueAveraged = sensorValue6Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt

  T0Connected = (sensorValueAveraged > 10);
  if(T0Connected == true){
#ifdef USE_TEMP_CACHE
    T0int = cacheT0.get(sensorValueAveraged);
    if(T0int == 0){
#endif
      T0int = countTemperature(RT0koeficient / sensorValueAveraged - RT0, thermistors);
#ifdef USE_TEMP_CACHE
      cacheT0.put(sensorValueAveraged, T0int);
    }
#endif
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
#ifdef USE_TEMP_CACHE
    T1int = cacheT1.get(sensorValueAveraged);
    if(T1int == 0){
#endif
      T1int = countTemperature(RT1koeficient / sensorValueAveraged - RT1, thermistors + 1);
#ifdef USE_TEMP_CACHE
      cacheT1.put(sensorValueAveraged, T1int);
    }
#endif
    T1WithHysteresisInt = countHysteresisTemperature(T1WithHysteresisInt, T1int);
  } else {
    T1int = 0;
  }
}

#else
void countT0(){
  T0Connected = 1;
  T0int = (int)((millis() / 100) % 600) + 100;
  T0WithHysteresisInt = countHysteresisTemperature(T0WithHysteresisInt, T0int);
}

void countT1(){
  T1Connected = 1;
  T1int = 700 - (int)((millis() / 100) % 600);
  T1WithHysteresisInt = countHysteresisTemperature(T1WithHysteresisInt, T1int);
}
#endif

int countTemperature(unsigned long thermistorResistance, ThermistorDefinition *tDef){
  float steinhart2;
  steinhart2 = (float)thermistorResistance / tDef -> resistanceNominal;// (R/Ro)
  steinhart2 = log(steinhart2);                                        // ln(R/Ro)
  steinhart2 /= tDef -> bCoefficient;                                  // 1/B * ln(R/Ro)
  steinhart2 += 1.0 / (tDef -> tempNominal + 273.15);                  // + (1/To)
  steinhart2 = 1.0 / steinhart2;                                       // Invert
  steinhart2 -= 273.15;                                                // convert to C
  return (int)(steinhart2 * 10);
}

int countHysteresisTemperature(int tWithHysteresisInt, int tInt){
  if((tInt - tWithHysteresisInt) > *hysteresis){
    return(tInt - *hysteresis);
  }
  if((tWithHysteresisInt - tInt) > *hysteresis){
    return(tInt + *hysteresis);
  }
  return tWithHysteresisInt;
}

