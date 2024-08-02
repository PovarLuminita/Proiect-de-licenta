#ifndef F_CPU
#define F_CPU 16000000UL //FRECVENTA CEAS
#endif
#include <util/delay.h>
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD)-1
#include "USART_slave.h"

int main(void)
{
	//--------------------------------------------------------------------
	//apelez functiile de initializare
	//--------------------------------------------------------------------
	init_data();
	init_devices();
	USART_Init(MYUBRR);
	initializeRegisters(Registers, 0x00, 6); 
	flag_slave_full = 0;  // flagul folosit in cazul in care bufferul de response e full
	DDRD  |= (1<< PIN_DE) | (1<<PIN_RE); // Pinii DE si RE ii consider de iesire
	
	//Cand ambii pini sunt 0 --> RX
	PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
	PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
	do
	{
		//Cand ambii pini sunt 0 --> RX
		PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
		PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;

		//Daca s-a incheiat receptia pe usart 1
		if(flag_stop_timer_usart == 1)
		{
			//resetez flagurile pentru timer
			flag_stop_timer_usart = 0;
			flag_start_timer_usart = 0;
			
			///calculez crc-ul cu bufferul primit
			modbus_message.rxMessage.asStruct.nDLEN = contor_buffer_usart - MODBUS_CRC_LENGTH;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.rxMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			//verific daca crc-ul calculat este acelasi cu crc-ul primit
			if(modbus_message.rxMessage.asArray[contor_buffer_usart-MODBUS_CRC_LENGTH] == CRC_l && modbus_message.rxMessage.asArray[contor_buffer_usart-MODBUS_CRC_LENGTH+1] == CRC_h)
			{
				if(ModbusCheckAddressSlave(&modbus_message.rxMessage)) //apelez functia care verifica id
				{
					ModbusSlaveProcessComm(&modbus_message, &Registers); //procesez mesajul
					
					//--------------------------------------------------------------------
					//apelez functia pentru a trimite raspunsul la gateway
					//--------------------------------------------------------------------
					if(flag_slave_full == 1)
					{
						_delay_ms(3); 
						USART0_TX_SIR_SIZE(modbus_message.txMessage.asArray, nr_bytes_send+MODBUS_CRC_LENGTH);
						pinToggle(&PORT_LED0,PIN_LED0); //led verificare
						_delay_ms(3);
						
						
						//Cand ambii pini sunt 0 --> RX
						PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
						PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
						
						flag_slave_full = 0; //resetez flagul care verifica daca bufferul de response este full
					}
				}
				contor_buffer_usart = 0; //resetez contorul care se incrementeaza dupa fiecare caracter primit pe intreruperea de rx
			}
		}
	}
	while(1); //bucla infinita
	return 0;
}