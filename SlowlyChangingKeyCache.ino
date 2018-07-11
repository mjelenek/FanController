
template <class TData, byte sizeShift> class SlowlyChangingKeyCache
{
  struct Record
  {
    TData value;
  } record;

  byte buffSize = B00000001 << sizeShift;
  byte mask = buffSize - 1;
  Record buffer[B00000001 << sizeShift];
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
    for(byte i = 0; i < B00000001 << sizeShift; i++){
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
SlowlyChangingKeyCache<int, 6> cacheT[NUMBER_OF_THERMISTORS]; // 64 records ~ 6°C
#endif

#ifdef USE_PWM_CACHE
SlowlyChangingKeyCache<unsigned short, 2> cacheFan[NUMBER_OF_FANS]; // 4 records
#endif

