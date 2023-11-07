// Programming an ADC

uint32_t ADCV;

void ADCInt(void)
{
    //disable
    ADCIntDisable(ADC0_BASE, 3);
    //clear
    ADCIntClear(ADC0_BASE, 3);
    //do stuff
    ADCSequenceDataGet(ADC0_BASE, 3, &ADCV);
//    ADCProcessorTrigger(ADC0_BASE, 3);
    //reenable
    ADCIntEnable(ADC0_BASE, 3);
}

void configure(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCClockConfigSet(AD0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL, 1);
    ADCHardwareOversampleConfigure(AD0_BASE,4);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR,0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0); 
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC0_BASE, 3);
    ADCIntRegister(ADC0_BASE, 3, ADCInt);
    ADCIntEnable(ADC0_BASE,3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_WPU);
}




int main(void)
{
    configure();
    
    while(1)
    {}
}