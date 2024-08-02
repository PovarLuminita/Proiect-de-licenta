
#ifndef F_CPU
#define F_CPU 16000000UL //FRECVENTA CEAS
#endif

#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD)-1

#include "USART_handle.h"

int main()
{
	//initializari
	init_contor_timp();
	init_devices();

	USART0_Init(MYUBRR);
	
	initializeRegisters(Registers_pwm, 5, 17);
	test_contor_pwm = 0;
	//rx
	PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
	PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
	
	do{
		//rx
		PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
		PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
		
		//verific daca s-a incheiat receptia de la gateway
		if(flag_stop_timer_usart0 == 1)
		{
			flag_stop_timer_usart0 = 0;
			flag_start_timer_usart0 = 0;
			
			modbus_message.rxMessage.asStruct.nDLEN = contor_buffer_usart0 - MODBUS_CRC_LENGTH;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.rxMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			if(modbus_message.rxMessage.asArray[contor_buffer_usart0-MODBUS_CRC_LENGTH] == CRC_l && modbus_message.rxMessage.asArray[contor_buffer_usart0-MODBUS_CRC_LENGTH+1] == CRC_h)
			{
				//verific id-ul
				if(ModbusCheckAddressSlave(&modbus_message.rxMessage)) 
				{
					ModbusSlaveProcessComm(&modbus_message, &Registers_pwm);
					if(flag_slave_full == 1)
					{
						_delay_ms(3);
						//trimite raspunsul
						USART0_TX_SIR_SIZE(modbus_message.txMessage.asArray, nr_bytes_send+MODBUS_CRC_LENGTH);
						_delay_ms(3);
						//rx
						PORTD &= ~(1<<PIN_DE);            // PD3->  DE = Low;
						PORTD &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
						
						flag_slave_full = 0;
					}
				}
			}
			contor_buffer_usart0 = 0;		
		}
	}while(1);
	return 0;
}