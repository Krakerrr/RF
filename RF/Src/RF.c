/*
 * RF.c
 *
 *  Created on: 30 Ağu 2023
 *      Author: KRAKER
 */

#include "RF.h"

void RF_SendDATA(uint8_t* sendata)
{
	uint16_t CRCval;
	if( RF_data.TXstatus == TX_completed)
	{
		// len + header
		RF_data.sendata[0] = RF_DATASIZE;
		RF_data.sendata[1] = RF_HEADER;

		// payload
		memcpy( &RF_data.sendata[2], sendata, RF_DATAPAYLOADSIZE);

		// CRC
		CRCval = calculateCRC();
		RF_data.sendata[RF_DATASIZE-2] = (uint8_t) (CRCval & 0xFF);
		RF_data.sendata[RF_DATASIZE-1] = (uint8_t) (CRCval >> 8);

		// send data
		RF_TX_START_IT();
	}
}

uint16_t calculateCRC(void)
{
	uint16_t CRCval;
	CRCval = 0;
	for (uint8_t i = 0; i < RF_DATAPAYLOADSIZE+2; ++i) {
		CRCval += RF_data.sendata[i];
	}
	return CRCval;
}

void RF_TxCpltCallback(UART_HandleTypeDef *huart)
{
	RF_data.TXstatus = TX_completed;
}

void RF_TX_START_IT()
{
	if( HAL_UART_Transmit_IT(&hRF, (&RF_data)->sendata, RF_DATASIZE) != HAL_OK)
	{
		RF_SendMsg("Error in RF_TX_START_IT\r\n");
	}
	RF_data.TXstatus = TX_started;
}

void RF_SendMsg(char *format,...)
{
	char str[40];
	uint16_t str_size;

	/*Extract the the argument list using VA apis */
	va_list args;
	va_start(args, format);
	vsprintf(str, format,args);

	str_size = strlen(str);
	if( HAL_UART_Transmit(&hRF,(uint8_t *)str, str_size, (uint32_t)(str_size/10 + 2 )) != HAL_OK)
	{
		Error_Handler();
	}
	va_end(args);
}

void RF_Init(void)
{
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
