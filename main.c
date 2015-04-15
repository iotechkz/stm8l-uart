
#include "iostm8l152c6.h"
#include <intrinsics.h>  	//����� ������� ������� __enable_interrupt ()

/**
 * �������� ������� ��������� �������� CLK_DIVR, ������������� ������ �������
 * 3 ���, ������� 5 ��� �� ������������
 */
#define CLK_DIV_1     0x00
#define CLK_DIV_2     0x01
#define CLK_DIV_4     0x02
#define CLK_DIV_8     0x03
#define CLK_DIV_16    0x04
#define CLK_DIV_32    0x05
#define CLK_DIV_64    0x06
#define CLK_DIV_128   0x07

/**
 * ��������� ��������� ������������, ��������� � �������� CLK_SWR, ���� �������-
 * ��� ��� SWBSY ���� ������� ������� �� ������. Reset value HSI.
 */
#define HSI   0x01
#define LSI   0x02
#define HSE   0x04
#define LSE   0x08

char flag = 0;
int main( void )
{
  	/* ��������� GPIO */
  	PC_DDR &= ~(1 << 2);	// ������ Rx ������
	PC_DDR |= (1 << 3);		// ������ Tx �������
	// ������� �������� ��������� PC7
	PC_DDR |= (1 << 7);
	PC_CR1 |= (1 << 7);
	PC_ODR |= (1 << 7);
  
    /* ��������� ������������ */
    CLK_CKDIVR = CLK_DIV_1;   // ������� �� �������, �������� ����� 1 
    CLK_SWCR_bit.SWEN = 1;    // ��������� �������������� ������������ �������
    //CLK_SWR = HSE;            // �������� �������� ������������ HSE
	CLK_SWR = HSI;            // �������� �������� ������������ HSE
    
    /* ���� ���� �� ���������� ������������ HSE, ���� ���-�� ��������� � HSE, ��
       ����� ��������� �������� */
    while (CLK_SWCR_bit.SWBSY == 1);
    
    /** ��������� ������ ������������ �� ���������, ������� CLK_PCKENR1
     * PCKEN17 -- DAC
     * PCKEN16 -- BEEP
     * PCKEN15 -- USART1
     * PCKEN14 -- SPI1
     * PCKEN13 -- I2C1
     * PCKEN12 -- TIM4
     * PCKEN11 -- TIM3
     * PCKEN10 -- TIM2
     */  
    CLK_PCKENR1_bit.PCKEN15 = 1;   	// ������ ������������ �� USART1
    
    /* ��������� UART */
	USART1_CR1_bit.M = 0;			// ��������� word length 0: 8 ���, 1: 9 ���
	USART1_CR3_bit.STOP1 = 0;		// ����������� ���������� ���� ���
	USART1_CR3_bit.STOP0 = 0;		// 00 - 1 ���� ���, 01 - Reserved, 10 - 2, 11 - 1.5 
	
	/* ������������ �������� �������� ������� UART ��� �������� ������ 9600 bps 
	 * 16000000 / 9600 = 1666.6 (0x0683 � 16-������ � �����������)
	 */
	USART1_BRR2 = 0x03;
	USART1_BRR1 = 0x68;
	
	/* ��������� ������� UART � ����� Tx � Rx ������ GPIO � �������� USART1_CR2 */
	USART1_CR2_bit.TEN = 1;	// ��������
	USART1_CR2_bit.REN = 1;	// �����
	
	USART1_CR2_bit.RIEN = 1;	// ��������� ���������� �� ������
	__enable_interrupt();		// ��������� ���������� ���������
	
	while (1) {}
}

/* ���������� ���������� �� ������ UART */
#pragma vector = USART_R_RXNE_vector
__interrupt void uart_rx_interrupt(void) {
	int data = USART1_DR;
	
	/* �������� ������� */
	if (flag) {
		PC_ODR |= (1 << 7);
	}
	else PC_ODR &= ~(1 << 7);
	flag = ~flag;
}
