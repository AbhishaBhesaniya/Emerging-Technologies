/***********************************************************************************************************
  * File Name          : Lab2.c
  * Description        : this program turn on Timer and at the time of call back function execution it will
  *                      read the sensor data connected with ADC chennel no and we can change the timer period 
  *                      entered by the user. after doing calibration calculationit will print the current
  *                      temperature on serial port.
  * 
  * Author             : Abhisha Bhesaniya              
  * Date               : 17th February, 2020			 
  **********************************************************************************************************
*/

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
//#include  <stm32f3xx_hal_flash.h>
#include "common.h"

//declaration of the global handle varibles for ADC and timer17
TIM_HandleTypeDef htim17;
//TIM_HandleTypeDef htim17;
ADC_HandleTypeDef hadc; // globally declaring hadc
uint32_t count = 0;
//Function declartion
void Gpioinit();
void ADCconfig( int channel);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);	// function created to initialize GPIO Pin
void Timerconfig(uint32_t period);
void ADCInit();
void TIM17_IRQHandler(void);
void temp_read();
/*
void Flash_Init(void)
{        
    FLASH_EraseInitTypeDef pEraseInit;

    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.NbSectors = 1;
    pEraseInit.Sector = FLASH_SECTOR_11;
    pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;

    if(HAL_FLASH_Unlock() == HAL_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
        HAL_FLASHEx_Erase(&pEraseInit,0);
        HAL_FLASH_Lock();
    }
}
uint32_t flash_read(uint32_t address){
    return *(uint32_t*)address;
}

void flash_write(uint32_t address, uint32_t data){
    HAL_FLASH_Unlock();
    FLASH_Erase_Sector(FLASH_SECTOR_11,VOLTAGE_RANGE_1);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,address,data);
    HAL_FLASH_Lock();
}*/

/*******************************************************************************
  * Function           : Gpioinit()
  * Description        : This function will Init the GPIO pin 0 of PORT C and configure it.  
  *                      this function. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ****************************************************************************/
void Gpioinit()
{
		GPIO_InitTypeDef  GPIO_InitStruct;
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = 0;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/*******************************************************************************
  * Function           : ADCconfig
  * Description        : This function will configure the ADC for chennel 6.  
  *                      this function. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ****************************************************************************/
void ADCconfig( int channel)
{
		HAL_StatusTypeDef rc;

		ADC_ChannelConfTypeDef config;

		/* Configure the selected channel */
		config.Channel = channel;
		config.Rank = 1;  /* Rank needs to be 1, otherwise no conversion is done */
		config.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
		config.SingleDiff = ADC_SINGLE_ENDED;
		config.OffsetNumber = ADC_OFFSET_NONE;
		config.Offset = 0;
		rc = HAL_ADC_ConfigChannel(&hadc,&config);


	       if(rc != HAL_OK) 
		{
			printf("ADC channel configure failed with rc=%u\n",(unsigned)rc);
		}
}
	
/*******************************************************************************
  * Function           : timerconfig
  * Description        : This function will initialize timer 17 when we will call this function. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ******************************************************************************
*/	
void Timerconfig(uint32_t period)
{
		__HAL_RCC_TIM17_CLK_ENABLE();
		
		HAL_StatusTypeDef rc = HAL_OK;//status declaration 
		
		htim17.Instance=TIM17;
		htim17.Init.Prescaler=(HAL_RCC_GetPCLK2Freq()/10000)-1;
		htim17.Init.CounterMode =TIM_COUNTERMODE_UP;
		htim17.Init.Period=0x2710;
		htim17.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
		htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
		htim17.Init.RepetitionCounter=(period>0)?period:1;
		
		rc = HAL_TIM_Base_Init(&htim17);//initialize the channel 
		
		if(rc != HAL_OK)
		printf("timer17 configure failed with rc=%u\n",(unsigned)rc);//print fail messga is initializing the channel fails
}

/*******************************************************************************
  * Function           : ADCInit()
  * Description        : This function will initialize ADC1 when we will call this function. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ******************************************************************************
*/
void ADCInit()
{

		HAL_StatusTypeDef rc;
		Gpioinit(); 	// calling GPIO init function

		/*initialize ADC*/
		__HAL_RCC_ADC1_CLK_ENABLE();
		hadc.Instance = ADC1;
		hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
		hadc.Init.Resolution = ADC_RESOLUTION_12B;
		hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
		hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
		hadc.Init.LowPowerAutoWait = DISABLE;
		hadc.Init.ContinuousConvMode = DISABLE;
		hadc.Init.NbrOfConversion = 1;

		hadc.Init.DiscontinuousConvMode = DISABLE;
		hadc.Init.NbrOfDiscConversion = 0;
		hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
		hadc.Init.ExternalTrigConvEdge = 0;
		hadc.Init.DMAContinuousRequests = DISABLE;
		hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

		rc = HAL_ADC_Init(&hadc);
	
		if(rc != HAL_OK) 
		{
			printf("ADC1 initialization failed with rc=%u\n",rc);
		}

			
}




ParserReturnVal_t cmdtemp(int action)
{
	  uint32_t Channel = 6;	
	  uint32_t delay = 0;
	  
           fetch_uint32_arg(&delay); //scanning delay time value 
	   ADCInit();
	   ADCconfig(Channel); //adcconfig function call
	   Timerconfig(delay);
	   HAL_NVIC_EnableIRQ(TIM17_IRQn);
	   HAL_TIM_Base_Start_IT(&htim17);
	   
	   return CmdReturnOk;
}

ADD_CMD("temp",cmdtemp,"To start ADC conversion and add channel no for ADC");

/*******************************************************************************
  * Function           : temp_read()
  * Description        : This function will read data from the ADC and perform calculation
  *                      to calibrate the sensor and send the data to the searial monitor. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ******************************************************************************
*/
void temp_read()
{
        uint32_t AdcRawdata;  				// to read adc raw data
        float Voltage;					// to store voltage value
	//uint32_t Channel = 0;				// to store whhich channel to operate
	float temp = 0;
        HAL_StatusTypeDef rc;
                       
                       /* Start the ADC peripheral */
			rc = HAL_ADC_Start(&hadc); // adc start
			if(rc != HAL_OK) 
			{
				printf("ADC start failed with rc=%u\n",(unsigned)rc);
				//return 0;
			}

			/* Wait for end of conversion */
			rc = HAL_ADC_PollForConversion(&hadc, 100); 
			if(rc != HAL_OK) 
			{
				printf("ADC poll for conversion failed with ""rc=%u\n",(unsigned)rc);
				//return 0;
			}	

			/* Read the ADC converted values */
			AdcRawdata = HAL_ADC_GetValue(&hadc);
			//printf("Raw Data = %u \n" ,(unsigned)AdcRawdata); // printing raw data of ADC

			/*formula to convert RAW data to voltage 
			where 3.3 is supply volage 
			4096 is from 2^12 where 12 is number of bitof processor  */
			Voltage =(AdcRawdata*3.3)/4096;
			
			temp = (Voltage - 0.500 - 0.160) / 0.01;
			//printf("Volatge = %0.2f volts \n" ,(float)Voltage);
			//printf("no of sample = %ld\n", (uint32_t)count);
			printf("$,%ld,%0.2f,#\n",(uint32_t)count,(float)temp);
			//HAL_Delay(2000);				   // providing delay to dispaly message


			/* Stop the ADC peripheral */
			rc = HAL_ADC_Stop(&hadc);
			if(rc != HAL_OK) 
			{
				printf("ADC stop failed with ""rc=%u\n",(unsigned)rc);
				//return 0;
			}
			count++;	
}


/*******************************************************************************
  * Function           : TIM17_IRQHandler()
  * Description        : This function will execute the code when there is intrup 
  *                      generated by timer. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ******************************************************************************
*/
void TIM17_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim17);
}

/*******************************************************************************
  * Function           : HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
  * Description        : This function will execute the code when there is periodElapsedCallback 
  *                      generated by timer. 
  * 
  * Author             : Abhisha Bhesaniya 
  * Date               : 15th July, 2020				 
  ******************************************************************************
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
        
              
	if(htim == &htim17)
	{
		temp_read();			
	}
}



