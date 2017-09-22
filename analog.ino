// ADC conversion complete interrupt handler
ISR(ADC_vect)
{
  static byte adcIndex = 0;

  byte low = ADCL;
  byte high = ADCH;
  
  // Read the AD conversion result
  unsigned short sensorValue = (high << 8) | low;
  
  // Process value and select next ADC input
  switch (adcIndex) {
    case 0:
      sensorValue0Averaged = ((sensorValue0Averaged + sensorValue) >> 1);
      adcIndex = 1;
      break;
    case 1:
      sensorValue1Averaged = ((sensorValue1Averaged + sensorValue) >> 1);
      adcIndex = 2;
      break;
    case 2:
      sensorValue2Averaged = ((sensorValue2Averaged + sensorValue) >> 1);
      adcIndex = 3;
      break;
    case 3:
      sensorValue3Averaged = ((sensorValue3Averaged + sensorValue) >> 1);
      adcIndex = 4;
      break;
    case 4:
      sensorValue4Averaged = ((sensorValue4Averaged + sensorValue) >> 1);
      adcIndex = 6;
      break;
    case 6:
      sensorValue6Averaged = ((3 * sensorValue6Averaged + sensorValue) >> 2);
      adcIndex = 7;
      break;
    case 7:
      sensorValue7Averaged = ((3 * sensorValue7Averaged + sensorValue) >> 2);
      adcIndex = 0;
      break;
    default:
      adcIndex = 0;
  }

  ADMUX = adcIndex;             // select next ADC channel, VREF is EXTERNAL

// Start next conversion - triggered by Timer1
//  ADCSRA |= (1 << ADSC); 
}

