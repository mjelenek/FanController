
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
    Serial.print(F("cache"));
    if(gui){
      for(short i = lowLimit; i <= highLimit; i++){
        byte buff[4];
        buff[0] = lowByte(i);
        buff[1] = lowByte(i >> 8);
 
        TData data = get(i);
        buff[2] = lowByte(data);
        buff[3] = lowByte(data >> 8);
        Serial.write(buff, 4);
      }
    } else {
      Serial.println(F(""));
      for(short i = lowLimit; i <= highLimit; i++){
        Serial.print(i);
        Serial.print(F(" - ["));
        Serial.print(i & mask);
        Serial.print(F("] - "));
        Serial.println(get(i));
      }
    }
  }
  
};

SlowlyChangingKeyCache<short, 6> cacheT0; // 64 records ~ 6°C
SlowlyChangingKeyCache<short, 5> cacheT1; // 32 records ~ 3°C


