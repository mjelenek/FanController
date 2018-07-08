void countT(){
  byte tNumber = part_64 >> 3;
  // true if part_64 == 1, 9 (17, 25, 33 ...)
  if(tNumber < NUMBER_OF_THERMISTORS && ((tNumber << 3) + 1 == part_64)){
    countT(tNumber);
  }
}

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

// tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
int countEffectiveTemperature(byte tSelect){
  switch (tSelect) {
    case 0:
      if(TConnected[0]){
        return TWithHysteresisInt[0];
      }
      break;
    case 1:
      if(TConnected[1]){
        return TWithHysteresisInt[1];
      }
      break;
    case 2:
      if(TConnected[0] && TConnected[1]){
        return (TWithHysteresisInt[0] + TWithHysteresisInt[1]) >> 1;
      }
      if(TConnected[0]){
        return TWithHysteresisInt[0];
      }
      if(TConnected[1]){
        return TWithHysteresisInt[1];
      }
      return MAX_ALLOWED_TEMP;
      break;
    default:
      return MAX_ALLOWED_TEMP;
  }
  return MAX_ALLOWED_TEMP;
}

