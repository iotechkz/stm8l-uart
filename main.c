
#include "iostm8l152c6.h"
#include <intrinsics.h>  	//Здесь описана функция __enable_interrupt ()

/**
 * Делители частоты настройка регистра CLK_DIVR, настраиваются только младшие
 * 3 бит, старшие 5 бит не записываются
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
 * Установки источника тактирования, настройка в регистре CLK_SWR, пока установ-
 * лен бит SWBSY этот регистр защищен от записи. Reset value HSI.
 */
#define HSI   0x01
#define LSI   0x02
#define HSE   0x04
#define LSE   0x08

char flag = 0;
int main( void )
{
  	/* Настройка GPIO */
  	PC_DDR &= ~(1 << 2);	// Делаем Rx входом
	PC_DDR |= (1 << 3);		// Делаем Tx выходом
	// Тестово зажигаем светодиод PC7
	PC_DDR |= (1 << 7);
	PC_CR1 |= (1 << 7);
	PC_ODR |= (1 << 7);
  
    /* Настройка тактирования */
    CLK_CKDIVR = CLK_DIV_1;   // Частота не делится, делитель равен 1 
    CLK_SWCR_bit.SWEN = 1;    // Разрешаем автоматическое переключение частоты
    //CLK_SWR = HSE;            // Выбираем источник тактирования HSE
	CLK_SWR = HSI;            // Выбираем источник тактирования HSE
    
    /* Ждем пока не произойдет переключение HSE, если что-то случилось с HSE, то
       здесь программа зависнет */
    while (CLK_SWCR_bit.SWBSY == 1);
    
    /** Настройка подачи тактирование на периферию, регистр CLK_PCKENR1
     * PCKEN17 -- DAC
     * PCKEN16 -- BEEP
     * PCKEN15 -- USART1
     * PCKEN14 -- SPI1
     * PCKEN13 -- I2C1
     * PCKEN12 -- TIM4
     * PCKEN11 -- TIM3
     * PCKEN10 -- TIM2
     */  
    CLK_PCKENR1_bit.PCKEN15 = 1;   	// Подаем тактирование на USART1
    
    /* Настройка UART */
	USART1_CR1_bit.M = 0;			// Установка word length 0: 8 бит, 1: 9 бит
	USART1_CR3_bit.STOP1 = 0;		// Настраиваем количество стоп бит
	USART1_CR3_bit.STOP0 = 0;		// 00 - 1 стоп бит, 01 - Reserved, 10 - 2, 11 - 1.5 
	
	/* Рассчитываем делитель тактовой частоты UART для скорости обмена 9600 bps 
	 * 16000000 / 9600 = 1666.6 (0x0683 в 16-ричной с округлением)
	 */
	USART1_BRR2 = 0x03;
	USART1_BRR1 = 0x68;
	
	/* Установка выводов UART в режим Tx и Rx вместо GPIO в регистре USART1_CR2 */
	USART1_CR2_bit.TEN = 1;	// Передача
	USART1_CR2_bit.REN = 1;	// Прием
	
	USART1_CR2_bit.RIEN = 1;	// Разрешаем прерывание по приему
	__enable_interrupt();		// Разрешаем прерывания глобально
	
	while (1) {}
}

/* Обработчик прерывания по приему UART */
#pragma vector = USART_R_RXNE_vector
__interrupt void uart_rx_interrupt(void) {
	int data = USART1_DR;
	
	/* Тестовая мигалка */
	if (flag) {
		PC_ODR |= (1 << 7);
	}
	else PC_ODR &= ~(1 << 7);
	flag = ~flag;
}
