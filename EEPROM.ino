void loadConfiguration(){
  ControllerConfigurationHolder.Load();
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    ConfigurationPWMHolder[i].Load();
  }
}

void saveConfiguration(){
  #ifdef SAVE_DEBUG
  Serial.print(F("Before save - "));
  printBufferToStoreDebug();
  #endif
  ControllerConfigurationHolder.Save();
  #ifdef SAVE_DEBUG
  Serial.print(F("After save configuration - "));
  printBufferToStoreDebug();
  #endif
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    ConfigurationPWMHolder[i].Save();
    #ifdef SAVE_DEBUG
    Serial.print(F("After save "));
    Serial.print(i);
    Serial.print(F(" - "));
    printBufferToStoreDebug();
    #endif
  }
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

