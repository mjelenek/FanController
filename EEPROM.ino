void loadConfiguration(){
  ControllerConfigurationHolder.Load();
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    ConfigurationPWMHolder[i].Load();
  }
}

void saveConfiguration(){
  ControllerConfigurationHolder.Save();
  configurationToSave = 0;
}

void checkSave(){
  if(configurationToSave < NUMBER_OF_FANS){
    ConfigurationPWMHolder[configurationToSave].Save();
    configurationToSave++;
    if(configurationToSave == NUMBER_OF_FANS){
      startWritingBufferByISR();
      configurationToSave = 255;
    }
  }
}

