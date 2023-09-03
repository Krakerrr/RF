/*
 * RF.h
 *
 *  Created on: 30 Ağu 2023
 *      Author: Administrator
 */

#ifndef INC_RF_H_
#define INC_RF_H_

#include "main_app.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define RF_CHANNEL					USART2
#define RF_BAUDRATE					115200
#define RF_GPIO						GPIOD
#define RF_TXPIN					GPIO_PIN_5
#define RF_RXPIN					GPIO_PIN_6
#define RF_GPIO_AF        			GPIO_AF7_USART2
#define __RF_CLK_ENABLE()   		do { \
											__HAL_RCC_USART2_CLK_ENABLE(); \
											__HAL_RCC_GPIOD_CLK_ENABLE(); \
										} while(0U)
#define __RF_NVIC_ENABLE()   		do { \
											HAL_NVIC_SetPriority(USART2_IRQn, 10, 0); \
											HAL_NVIC_EnableIRQ(USART2_IRQn); \
										} while(0U)

#define RF_DATASIZE					60
#define RF_DATAPAYLOADSIZE			56
#define RF_DATAHEADER				0x60
#define RF_MSGHEADER				0x55

typedef enum
{
    TX_completed	= 0,
	TX_started
}RF_TXstatus_ENUM;

typedef struct
{
	uint8_t                 telemetrydata[RF_DATASIZE];
	uint8_t*				payload_address;
	RF_TXstatus_ENUM		TXstatus;
}sRF;

extern sRF RF_data;

void RF_Init(void);
void RF_GPIOInit(UART_HandleTypeDef *huart);
void RF_TX_START_IT();
void RF_SendMsg(char *format,...);
void RF_TxCpltCallback(UART_HandleTypeDef *huart);
void RF_SendTelemetryDATA(void);
uint16_t RF_CalculateCRC(void);

UART_HandleTypeDef hRF;


#endif /* INC_RF_H_ */
