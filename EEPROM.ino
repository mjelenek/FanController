void loadConfiguration(){
  ConfigurationPWM0.Load();
  ConfigurationPWM1.Load();
  ConfigurationPWM2.Load();
  ConfigurationPWM3.Load();
  ConfigurationPWM4.Load();
  ConfigurationPWM5.Load();
}

void saveConfiguration(){
  #ifdef SAVE_DEBUG
  Serial.print(F("Before save - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM0.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 0 - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM1.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 1 - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM2.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 2 - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM3.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 3 - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM4.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 4 - "));
  printBufferToStoreDebug();
  #endif
  ConfigurationPWM5.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save 5 - "));
  printBufferToStoreDebug();
  #endif
  startWritingBufferByISR();
}

#ifdef SAVE_DEBUG
void printBufferToStoreDebug(){
  Serial.print(F(" pointerActual:"));
  Serial.print(bufferToStoreActual);
  Serial.print(F(", pointerLast:"));
  Serial.print(bufferToStoreLast);
  Serial.print(F(", bufferFull:"));
  Serial.println(eeprom_buffer_full);
}
#endif

