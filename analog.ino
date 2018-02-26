// ADC conversion complete interrupt handler
static byte adcIndexStatic = 0;
ISR(ADC_vect)
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  byte adcIndex = adcIndexStatic;
 
  // Read the AD conversion result
  uint16_t sensorValue = ADC;
  
  // Process value and select next ADC input
  switch (adcIndex) {
    case 0:
      break;
    case 1:
      sensorValue0Averaged = ((sensorValue0Averaged + sensorValue) >> 1);
      ADMUX = 1;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 2:
      break;
    case 3:
      sensorValue1Averaged = ((sensorValue1Averaged + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 4:
      break;
    case 5:
      sensorValue2Averaged = ((sensorValue2Averaged + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 6:
      break;
    case 7:
      sensorValue3Averaged = ((sensorValue3Averaged + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 8:
      break;
    case 9:
      sensorValue4Averaged = ((sensorValue4Averaged + sensorValue) >> 1);
      ADMUX = 6;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 10:
      break;
    case 11:
      sensorValue6Averaged = ((sensorValue6Averaged + sensorValue) >> 1);
      ADMUX = 7;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 12:
      break;
    case 13:
      sensorValue7Averaged = ((sensorValue7Averaged + sensorValue) >> 1);
      adcIndex = -1;
      ADMUX = 0;                    // select next ADC channel, VREF is EXTERNAL
      break;
    default:
      adcIndex = -1;
  }
  adcIndex++;
  adcIndexStatic = adcIndex;

  // slow down ADC conversions
  ADCSRA &= ~(1 << ADEN);            // Disable ADC
  ADCSRA |= (1 << ADEN);             // Enable ADC

  ADCSRA |= (1 << ADSC);             // Start next conversion
}

