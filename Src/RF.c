/*
 * RF.c
 *
 *  Created on: 30 Ağu 2023
 *      Author: KRAKER
 */

#include "RF.h"

void RF_SendTelemetryDATA()
{
	uint16_t CRCval;
	if( RF_data.TXstatus == TX_completed)
	{
		// len + header
		RF_data.telemetrydata[0] = RF_DATASIZE;
		RF_data.telemetrydata[1] = RF_DATAHEADER;

		// CRC
		CRCval = RF_CalculateCRC();
		RF_data.telemetrydata[RF_DATASIZE-2] = (uint8_t) (CRCval & 0xFF);
		RF_data.telemetrydata[RF_DATASIZE-1] = (uint8_t) (CRCval >> 8);

		// send data
		RF_TX_START_IT();
	}
}

uint16_t RF_CalculateCRC(void)
{
	uint16_t CRCval;
	CRCval = 0;
	for (uint8_t i = 0; i < RF_DATAPAYLOADSIZE+2; ++i) {
		CRCval += RF_data.telemetrydata[i];
	}
	return CRCval;
}

void RF_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == RF_CHANNEL)
	{
		RF_data.TXstatus = TX_completed;
	}
}

void RF_TX_START_IT()
{
	if( HAL_UART_Transmit_IT(&hRF, (&RF_data)->telemetrydata, RF_DATASIZE) != HAL_OK)
	{
		RF_SendMsg("Error in RF_TX_START_IT\r\n");
	}
	RF_data.TXstatus = TX_started;
}

void RF_SendMsg(char *format,...)
{
	char str[40];
	char sendata[40];
	uint16_t str_size;

	sprintf(sendata,"%d%d ",strlen(format)+3,RF_MSGHEADER);
	strcat(sendata, format);

	/*Extract the the argument list using VA apis */
	va_list args;
	va_start(args, format);
	vsprintf(str, sendata,args);
	va_end(args);

	str_size = strlen(str);
	if( HAL_UART_Transmit(&hRF,(uint8_t *)str, str_size, (uint32_t)(str_size/10 + 2 )) != HAL_OK)
	{
		Error_Handler();
	}
}

void RF_Init(void)
{
	RF_data.payload_address = &RF_data.telemetrydata[2];
	RF_data.TXstatus = TX_completed;

	hRF.Instance 			= RF_CHANNEL;
	hRF.Init.BaudRate		= RF_BAUDRATE;
	hRF.Init.WordLength 	= UART_WORDLENGTH_8B;
	hRF.Init.StopBits 		= UART_STOPBITS_1;
	hRF.Init.Parity 		= UART_PARITY_NONE;
	hRF.Init.Mode 			= UART_MODE_TX_RX;
	hRF.Init.HwFlowCtl 		= UART_HWCONTROL_NONE;
	hRF.Init.OverSampling 	= UART_OVERSAMPLING_8;
	if ( HAL_UART_Init(&hRF) != HAL_OK )
	{
		//TODO: remove error handler
		Error_Handler();
	}
}

/* add to msp */
void RF_GPIOInit(UART_HandleTypeDef *huart)
{
	if(huart->Instance == RF_CHANNEL)
	{
		GPIO_InitTypeDef gpio_uart;

		//	1. enable peripheral clock usartx and gpiox
		__RF_CLK_ENABLE();

		//	2. pin muxing configurations (tx rx pins / 2gpios : alternate func. -> UART)
		gpio_uart.Pin   	= RF_TXPIN | RF_RXPIN;
		gpio_uart.Mode  	= GPIO_MODE_AF_PP;
		gpio_uart.Pull  	= GPIO_PULLUP;
		gpio_uart.Speed 	= GPIO_SPEED_FREQ_VERY_HIGH;
		gpio_uart.Alternate = RF_GPIO_AF;
		HAL_GPIO_Init( RF_GPIO, &gpio_uart);

		//	3. NVIC
		__RF_NVIC_ENABLE();
	}

}
