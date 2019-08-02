
template <class TData, byte buffSize,  byte mask> class SlowlyChangingKeyCache
{
  struct Record
  {
    TData value;
  } record;

  Record buffer[buffSize];
  short lowLimit = 0;
  short highLimit = 0;

public:
  void put(short key, TData value){
    if(key > highLimit){
      if(key - highLimit >= buffSize){
        clear();
        highLimit = key;
        lowLimit = key;
      } else {
        while(highLimit < key){
          highLimit++;
          buffer[highLimit & mask].value = 0;
        }
        if(highLimit - lowLimit >= buffSize){
          lowLimit = highLimit - buffSize + 1;
        }
      }
    }
    if(key < lowLimit){
      if(lowLimit - key >= buffSize){
        clear();
        highLimit = key;
        lowLimit = key;
      } else {
        while(lowLimit > key){
          lowLimit--;
          buffer[lowLimit & mask].value = 0;
        }
        if(highLimit - lowLimit >= buffSize){
          highLimit = lowLimit + buffSize - 1;
        }
      }
    }
    buffer[key & mask].value = value;
//    printStatus();
  };

  void clear(){
    for(byte i = 0; i < buffSize; i++){
      buffer[i].value = 0;
    }
    highLimit = 0;
    lowLimit = 0;
  }
  
  TData get(short key){
    if(key >= lowLimit && key <= highLimit){
      return buffer[key & mask].value;
    }
    return 0;
  };

  void printStatus(){
    if(gui){
      Serial.print(F("!"));
      Serial.write(2 * buffSize + 8);
      Serial.print(F("cache"));
      Serial.write(buffSize);
      serialWriteInt(lowLimit);
      for(short i = lowLimit; i < lowLimit + buffSize; i++){
        serialWriteInt(get(i));
      }
      Serial.print(F("#"));
    } else {
      for(short i = lowLimit; i < lowLimit + buffSize; i++){
        Serial.print(i);
        Serial.print(F(" - ["));
        Serial.print(i & mask);
        Serial.print(F("] - "));
        Serial.println(get(i));
      }
    }
  }
  
};

#ifdef USE_TEMP_CACHE
SlowlyChangingKeyCache<int, B00000001 << CACHE_T_SIZE, B00000001 << CACHE_T_SIZE - 1> cacheT[NUMBER_OF_THERMISTORS];
#endif

#ifdef USE_PWM_CACHE
SlowlyChangingKeyCache<unsigned short, B00000001 << CACHE_PWM_SIZE, B00000001 << CACHE_PWM_SIZE - 1> cacheFan[NUMBER_OF_FANS];
#endif

void cacheStatus(){
#ifdef USE_TEMP_CACHE
if(gui){
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    cacheT[i].printStatus();
  }
} else {
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    Serial.print(F("cache T"));
    Serial.println(i);
    cacheT[i].printStatus();
  }
}
#endif
#ifdef USE_PWM_CACHE
if(gui){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    cacheFan[i].printStatus();
  }
} else {
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F("cache RPMbyTemp["));
    Serial.print(i);
    Serial.println(F("]"));
    cacheFan[i].printStatus();
  }
}
#endif
}

