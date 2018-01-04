// ADC conversion complete interrupt handler
ISR(ADC_vect)
{
  static byte adcIndex = 0;
  static byte channelSelect = 0;

  // Read the AD conversion result
  uint16_t sensorValue = ADC;
  
  // Process value and select next ADC input
  switch (adcIndex) {
    case 0:
      break;
    case 1:
      sensorValue0Averaged = ((sensorValue0Averaged + sensorValue) >> 1);
      channelSelect = 1;
      break;
    case 2:
      break;
    case 3:
      sensorValue1Averaged = ((sensorValue1Averaged + sensorValue) >> 1);
      channelSelect = 2;
      break;
    case 4:
      break;
    case 5:
      sensorValue2Averaged = ((sensorValue2Averaged + sensorValue) >> 1);
      channelSelect = 3;
      break;
    case 6:
      break;
    case 7:
      sensorValue3Averaged = ((sensorValue3Averaged + sensorValue) >> 1);
      channelSelect = 4;
      break;
    case 8:
      break;
    case 9:
      sensorValue4Averaged = ((sensorValue4Averaged + sensorValue) >> 1);
      channelSelect = 6;
      break;
    case 10:
      break;
    case 11:
      sensorValue6Averaged = ((sensorValue6Averaged + sensorValue) >> 1);
      channelSelect = 7;
      break;
    case 12:
      break;
    case 13:
      sensorValue7Averaged = ((sensorValue7Averaged + sensorValue) >> 1);
      adcIndex = -1;
      channelSelect = 0;
      break;
    default:
      adcIndex = -1;
  }
  adcIndex++;

  ADMUX = channelSelect;             // select next ADC channel, VREF is EXTERNAL

  // slow down ADC conversions
  ADCSRA &= ~(1 << ADEN);            // Disable ADC
  ADCSRA |= (1 << ADEN);             // Enable ADC

  ADCSRA |= (1 << ADSC);             // Start next conversion
}

