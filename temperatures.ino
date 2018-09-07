void countT(byte tNumber){
  if(fakeTemp[tNumber] > 0){
    TConnected[tNumber] = true;
    Tint[tNumber] = fakeTemp[tNumber] * 10;
    TWithHysteresisInt[tNumber] = Tint[tNumber];
    return;
  }
  
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValueAveraged = thermistorADCAveraged[tNumber];
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt

  TConnected[tNumber] = (sensorValueAveraged > 10);
  if(TConnected[tNumber]){
#ifdef USE_TEMP_CACHE
    Tint[tNumber] = cacheT[tNumber].get(sensorValueAveraged);
    if(Tint[tNumber] == 0){
#endif
      unsigned long RT = RT(tNumber);
      unsigned long RTkoeficient = (RT << 10) - RT;
      Tint[tNumber] = countTemperature(RTkoeficient / sensorValueAveraged - RT, thermistors(tNumber));
#ifdef USE_TEMP_CACHE
      cacheT[tNumber].put(sensorValueAveraged, Tint[tNumber]);
    }
#endif
    TWithHysteresisInt[tNumber] = countHysteresisTemperature(TWithHysteresisInt[tNumber], Tint[tNumber]);
  } else {
    Tint[tNumber] = 0;
  }
}

int countTemperature(unsigned long thermistorResistance, ThermistorDefinition tDef){
  float steinhart2;
  steinhart2 = (float)thermistorResistance / tDef.resistanceNominal;   // (R/Ro)
  steinhart2 = log(steinhart2);                                        // ln(R/Ro)
  steinhart2 /= tDef.bCoefficient;                                     // 1/B * ln(R/Ro)
  steinhart2 += 1.0 / (tDef.tempNominal + 273.15);                     // + (1/To)
  steinhart2 = 1.0 / steinhart2;                                       // Invert
  steinhart2 -= 273.15;                                                // convert to C
  return (int)(steinhart2 * 10);
}

int countHysteresisTemperature(int tWithHysteresisInt, int tInt){
  if((tInt - tWithHysteresisInt) > hysteresis){
    return(tInt - hysteresis);
  }
  if((tWithHysteresisInt - tInt) > hysteresis){
    return(tInt + hysteresis);
  }
  return tWithHysteresisInt;
}

// tSelect: every bit represents one temperature. Count average value of thermistors signed by bit set to 1.
int countEffectiveTemperature(byte tSelect){
  if (tSelect == 0){
    return MAX_ALLOWED_TEMP;
  }
  int sumOfTemperatures = 0;
  unsigned char numberOfTemperatures = 0;
  for(unsigned char i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if((tSelect >> i) & 1){
      if(TConnected[i]){
        sumOfTemperatures = sumOfTemperatures + TWithHysteresisInt[i];
      } else {
        sumOfTemperatures = sumOfTemperatures + MAX_ALLOWED_TEMP;
      }
      numberOfTemperatures++;
    }
  }
  if(numberOfTemperatures == 1){
    return sumOfTemperatures;
  }
  if(numberOfTemperatures == 2){
    return (sumOfTemperatures >> 1);
  }
  return sumOfTemperatures/numberOfTemperatures;
}

