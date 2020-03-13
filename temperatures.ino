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

void setThermistor(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  byte thermistorNumber = parameters.NextParameterAsInteger();
  if(thermistorNumber < 0 || thermistorNumber >= NUMBER_OF_THERMISTORS) return;
  
  byte tempNominal = parameters.NextParameterAsInteger(25);
  unsigned short resistanceNominal = parameters.NextParameterAsInteger(10000);
  unsigned short bCoefficient = parameters.NextParameterAsInteger(3950);
  (thermistors(thermistorNumber)).Set(tempNominal, resistanceNominal, bCoefficient);

 #ifdef USE_TEMP_CACHE
    cacheT[thermistorNumber].clear();
 #endif    
}

void setTemp(CommandParameter &parameters){
  while(1){
    byte thermistorNumber = parameters.NextParameterAsInteger(255);
    if(thermistorNumber < 0 || thermistorNumber >= NUMBER_OF_THERMISTORS) return;
    
    unsigned char fakeTemperature = parameters.NextParameterAsInteger(0);
    if(fakeTemperature >= 0 && fakeTemperature <= MAX_ALLOWED_TEMP){
      fakeTemp[thermistorNumber] = fakeTemperature;
    }
  }
}

#ifdef CALIBRATE_THERMISTORS
void setCalibrateRNominal(CommandParameter &parameters)
{
  tempExpectedInt = 10 * parameters.NextParameterAsInteger( 255 );
  while(1){
    byte thermistorNumber = parameters.NextParameterAsInteger( 255 );

    if(tempExpectedInt == 2550 || thermistorNumber == 255)
      return;

    if(thermistorNumber >= 0 && thermistorNumber < NUMBER_OF_THERMISTORS && TConnected[thermistorNumber]){
      calibrateR[thermistorNumber] = 20;
    }
  }
}

void setCalibrateB(CommandParameter &parameters)
{
  tempExpectedInt = 10 * parameters.NextParameterAsInteger( 255 );
  while(1){
    byte thermistorNumber = parameters.NextParameterAsInteger( 255 );

    if(tempExpectedInt == 2550 || thermistorNumber == 255)
      return;

    if(thermistorNumber >= 0 && thermistorNumber < NUMBER_OF_THERMISTORS && TConnected[thermistorNumber]){
      calibrateBeta[thermistorNumber] = 20;
    }
  }
}

void calibrateRNominal()
{
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(calibrateR[i] > 0){
      calibrateR[i]--;
      int deltaT = Tint[i] - tempExpectedInt;
      thermistors(i).resistanceNominal = thermistors(i).resistanceNominal - (10 * deltaT);
      thermistors(i).tempNominal = (unsigned char)(tempExpectedInt / 10);
      if(calibrateR[i] == 0){
        sendThermistorConfiguration(i);
      }
#ifdef USE_TEMP_CACHE
      cacheT[i].clear();
#endif
    }
  }
}

void calibrateB()
{
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(calibrateBeta[i] > 0){
      calibrateBeta[i]--;
      int deltaT = Tint[i] - tempExpectedInt;
      if(((unsigned char)(tempExpectedInt / 10)) > thermistors(i).tempNominal){
        thermistors(i).bCoefficient = thermistors(i).bCoefficient + (10 * deltaT);
      } else {
        thermistors(i).bCoefficient = thermistors(i).bCoefficient - (10 * deltaT);
      }
      if(calibrateBeta[i] == 0){
        sendThermistorConfiguration(i);
      }
#ifdef USE_TEMP_CACHE
      cacheT[i].clear();
#endif
    }
  }
}

void sendThermistorConfiguration(byte numberOfThermistor){
  Serial.print(F("!"));
  Serial.write(11);
  Serial.print(F("tConf"));
  Serial.write(numberOfThermistor);
  thermistors(numberOfThermistor).sendDefinition();
  Serial.print(F("#"));
}
#endif
