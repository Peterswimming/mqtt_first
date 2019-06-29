#ifndef _BSP_IIC_GPIO_H
#define _BSP_IIC_GPIO_H
#include "stm32f10x.h"

/**************************���Ƿָ���********************************/
//����IO��Ҫ���ĵĵط�
//I2C_SCL			PB10
//I2C_SDA			PB11
#define RCC_I2C_PORT 	RCC_APB2Periph_GPIOB		/* GPIO�˿�ʱ�� */

#define PORT_I2C_SCL	GPIOB			/* GPIO�˿� */
#define PIN_I2C_SCL		GPIO_Pin_6		/* GPIO���� */

#define PORT_I2C_SDA	GPIOB			/* GPIO�˿� */
#define PIN_I2C_SDA		GPIO_Pin_7		/* GPIO���� */
/**************************���Ƿָ���********************************/

/* �����дSCL��SDA�ĺ� */
#define I2C_SCL_1()  PORT_I2C_SCL->BSRR = PIN_I2C_SCL				/* SCL = 1 */
#define I2C_SCL_0()  PORT_I2C_SCL->BRR = PIN_I2C_SCL				/* SCL = 0 */

#define I2C_SDA_1()  PORT_I2C_SDA->BSRR = PIN_I2C_SDA				/* SDA = 1 */
#define I2C_SDA_0()  PORT_I2C_SDA->BRR = PIN_I2C_SDA				/* SDA = 0 */

#define I2C_SDA_READ()  ((PORT_I2C_SDA->IDR & PIN_I2C_SDA) != 0)	/* ��SDA����״̬ */
#define I2C_SCL_READ()  ((PORT_I2C_SCL->IDR & PIN_I2C_SCL) != 0)	/* ��SCL����״̬ */

#define I2C_WR	0		/* д����bit */
#define I2C_RD	1		/* ������bit */

void Bsp_InitI2C(void);
void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);
#endif
