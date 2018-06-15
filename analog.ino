// ADC conversion complete interrupt handler
static byte adcIndexStatic = 0;

void init_adc()
{
  DIDR0 = B11011111;                                   // Disable digital input buffer for ADC pins
  
  ADMUX = 0;                                          // VREF is EXTERNAL, channel 0
  ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // prescaler to 128
         | (1 << ADATE)                               // ADC Auto Trigger Enable
         | (1 << ADIE)                                // Enable ADC conversion complete interrupt
         | (1 << ADEN)                                // Enable ADC
         | (1 << ADSC);                               // Start conversion
}

#if HWversion == 1
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
      powerInADCAveraged[0] = ((powerInADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 1;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 2:
      break;
    case 3:
      powerInADCAveraged[1] = ((powerInADCAveraged[1] + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 4:
      break;
    case 5:
      powerInADCAveraged[2] = ((powerInADCAveraged[2] + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 6:
      break;
    case 7:
      powerInADCAveraged[3] = ((powerInADCAveraged[3] + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 8:
      break;
    case 9:
      powerInADCAveraged[4] = ((powerInADCAveraged[4] + sensorValue) >> 1);
      ADMUX = 6;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 10:
      break;
    case 11:
      thermistorADCAveraged[0] = ((thermistorADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 7;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 12:
      break;
    case 13:
      thermistorADCAveraged[1] = ((thermistorADCAveraged[1] + sensorValue) >> 1);
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
#endif

#if HWversion == 2
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
      thermistorADCAveraged[0] = ((thermistorADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 1;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 2:
      break;
    case 3:
      thermistorADCAveraged[1] = ((thermistorADCAveraged[1] + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 4:
      break;
    case 5:
      thermistorADCAveraged[2] = ((thermistorADCAveraged[2] + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 6:
      break;
    case 7:
      thermistorADCAveraged[3] = ((thermistorADCAveraged[3] + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 8:
      break;
    case 9:
      thermistorADCAveraged[4] = ((thermistorADCAveraged[4] + sensorValue) >> 1);
      ADMUX = 6;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 10:
      break;
    case 11:
      powerInADCAveraged[0] = ((powerInADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 7;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 12:
      break;
    case 13:
      powerInADCAveraged[1] = ((powerInADCAveraged[1] + sensorValue) >> 1);
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
#endif

