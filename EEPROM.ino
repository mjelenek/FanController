void loadConfiguration(){
  ConfigurationPWM0.Load();
  ConfigurationPWM1.Load();
  ConfigurationPWM2.Load();
  ConfigurationPWM3.Load();
  ConfigurationPWM4.Load();
  ConfigurationPWM5.Load();
}

void saveConfiguration(){
  ConfigurationPWM0.Save();
  ConfigurationPWM1.Save();
  ConfigurationPWM2.Save();
  ConfigurationPWM3.Save();
  ConfigurationPWM4.Save();
  ConfigurationPWM5.Save();
  startWritingBufferByISR();
}

