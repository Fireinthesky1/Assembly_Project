// Programming an ADC

uint32_t ADCV;

void ADCInt(void)
{
    //disable
    //clear
    //do stuff
    //reenable
    ADCIntClear();
    ADCSequenceDataGet(ADC0_BASE, 3, &ADCV);
}

void configure(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // sampling as fast as possible
    // adc0
    // pin E3
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCClockConfigSet(AD0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL, 1);
    ADCHardwareOversampleConfigure(AD0_BASE,4);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR,0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0); 
    ADCSequenceEnable(ADC0_BASE, 3);
    
    ADCProcessorTrigger(ADC0_BASE, 3);
    ADCIntRegister(ADC0_BASE, 3, ADCInt);
    ADCIntEnable(ADC0_BASE,3);
}




int main(void)
{
    configure();
    
    while(1)
    {}
}