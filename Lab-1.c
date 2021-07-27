/* mytest.c:
 *
 *  Test C to assembly interface 
 */

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

ADC_HandleTypeDef hadc; // globally declaring hadc

	// function created to initialize GPIO Pin
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

	// function created to configure ADC
	void ADCconfig()
	{
		HAL_StatusTypeDef rc;

		ADC_ChannelConfTypeDef config;

		/* Configure the selected channel */
		config.Channel = 6;
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

		/*else 
		{
			printf("ADC1 is initialized\n");
		}*/
		
	}

	


ParserReturnVal_t Cmdtemp(int mode)
{
  

  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

                uint32_t AdcRawdata;  				// to read adc raw data
		float Voltage;					// to store voltage value
		uint32_t Channel = 0;				// to store whhich channel to operate
		float temp = 0;
               HAL_StatusTypeDef rc;
              
               
              
              
                       ADCInit();
			ADCconfig(Channel); //adcconfig function call
						
			/* Start the ADC peripheral */
			rc = HAL_ADC_Start(&hadc); // adc start
			if(rc != HAL_OK) 
			{
				printf("ADC start failed with rc=%u\n",(unsigned)rc);
				return 0;
			}

			/* Wait for end of conversion */
			rc = HAL_ADC_PollForConversion(&hadc, 100); 
			if(rc != HAL_OK) 
			{
				printf("ADC poll for conversion failed with ""rc=%u\n",(unsigned)rc);
				return 0;
			}	

			/* Read the ADC converted values */
			AdcRawdata = HAL_ADC_GetValue(&hadc);
			//printf("Raw Data = %u \n" ,(unsigned)AdcRawdata); // printing raw data of ADC
			
			/*formula to convert RAW data to voltage 
			where 3.3 is supply volage 
			4096 is from 2^12 where 12 is number of bitof processor  */
			Voltage =(AdcRawdata*3.3)/4096;
			
			temp = (Voltage - 0.500 - 0.145) / 0.01;
			//printf("Volatge = %0.2f volts \n" ,(float)Voltage);
			printf("temp = %0.2f C\n",(float)temp);
			HAL_Delay(2000);				   // providing delay to dispaly message


			/* Stop the ADC peripheral */
			rc = HAL_ADC_Stop(&hadc);
			if(rc != HAL_OK) 
			{
				printf("ADC stop failed with ""rc=%u\n",(unsigned)rc);
				return 0;
			}
			
		

		 
  return CmdReturnOk;
}

ADD_CMD("temp", Cmdtemp,"<n>       this command will print temperature.");
