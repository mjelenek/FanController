#define SlowlyChangingKeyCacheMacro(dataType, shift, nameAndNumberOfItems) SlowlyChangingKeyCache<dataType, B00000001 << shift, (B00000001 << shift) - 1> nameAndNumberOfItems
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
    memset(buffer, 0, buffSize * sizeof(TData));
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
      serialWriteShort(lowLimit);
      for(short i = lowLimit; i < lowLimit + buffSize; i++){
        serialWriteShort(get(i));
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
SlowlyChangingKeyCacheMacro(int, CACHE_T_SIZE, cacheT[NUMBER_OF_THERMISTORS]);
#endif

#ifdef USE_PWM_CACHE
SlowlyChangingKeyCacheMacro(unsigned short, CACHE_PWM_SIZE, cacheFan[NUMBER_OF_FANS]);
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
