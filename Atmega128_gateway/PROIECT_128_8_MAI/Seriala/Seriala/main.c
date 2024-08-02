
#ifndef F_CPU
#define F_CPU 16000000UL 
#endif

#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD)-1

#include "USART_handle.h"

int main()
{
	init_contor_timp(); // Initializeaza contoarele pentru timer
	init_devices(); // Initializeaza porturile si timerul

	USART0_Init(MYUBRR); // Initializeaza USART0 cu baud rate-ul calculat
	USART1_Init(MYUBRR);
	
	initializeRegisters(Registers_pwm, 5, 17); // Initializeaza registrii PWM
	
	// Configuratii pentru modul de receptie
	PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
	PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
	
	do{
		// Configuratii pentru modul de receptie
		PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
		PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
		
		//verifica daca cererea de la aplicatie a fost receptionata
		if(flag_stop_timer_usart0 == 1)
		{
			flag_stop_timer_usart0 = 0;
			flag_start_timer_usart0 = 0;
			
			//verifica daca id-ul este pentru gateway
			if(modbus_message.rxMessage.asStruct.address == 3)
			{
				//proceseaza cererea
				ModbusGatewayProcessComm(&modbus_message, &Registers_pwm);
				if(flag_slave_full == 1)
				{
					_delay_ms(3);
					//trimite raspunsul la aplicatie
					USART0_TX_SIR_SIZE(modbus_message.txMessage.asArray, nr_bytes_send+MODBUS_CRC_LENGTH);
					_delay_ms(3);
					//rx
					PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
					PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
					
					flag_slave_full = 0;
				}
			}
			else
			{
				//verifica daca id-ul este pentru unul dintre noduri
				if((modbus_message.rxMessage.asStruct.address == 1) || (modbus_message.rxMessage.asStruct.address == 5))
				{
					// Modul trimitere
					PORTB |= ( 1 << PIN_DE); /// PB3->  DE = High;
					PORTB |= ( 1 << PIN_RE); /// PB2-> ~RE = High;
					_delay_ms(3);
					//trimite cererea la nod
					USART1_TX_SIR_SIZE(modbus_message.rxMessage.asArray, contor_buffer_usart0);
					_delay_ms(3);
					
					//rx
					PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
					PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low
				}
			}
			contor_buffer_usart0 = 0;	
		}
		
		//verifica daca registrul pentru reglare automata are valoarea 0x10 pentru a fi on
		if(Registers_pwm[0].Value_pwm == 0x10)
		{
			//incarca comanda pentru citirea valorii senzorului
			TrimitComandaSenzor(&modbus_message, &Registers_pwm);
			_delay_ms(3);
			//trimite comanda la nod
			USART1_TX_SIR_SIZE(modbus_message.txMessage.asArray, 8);
			_delay_ms(3);
			
			//rx
			PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
			PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
			
			pinToggle(&PORT_LED0, PIN_LED0);
			_delay_ms(500);	
		}
		
		//verifica daca s-a receptionat raspunsul de la nod
		if(flag_stop_timer_usart1 == 1)
		{
			//verifica daca este on registrul pentru reglarea automata
			if(Registers_pwm[0].Value_pwm == 0x10)
			{
				//incarca comanda pentru pwm-ul led-urilor in functie de valoarea citita de la nodul cu senzorul
				TrimitComandaLED(&modbus_message, &Registers_pwm);
				_delay_ms(3);
				//trimite comanda la nodul cu led-uri
				USART1_TX_SIR_SIZE(modbus_message.txMessage.asArray, 39 + MODBUS_CRC_LENGTH);
				_delay_ms(3);
				//rx
				PORTB &= ~(1<<PIN_DE);            // PD3->  DE = Low;
				PORTB &= ~(1<<PIN_RE);            // PD2-> ~RE = Low;
				_delay_ms(500);
			}
			else
			{
				//trimite raspunsul la aplicatia desktop
				USART0_TX_SIR_SIZE(modbus_message.rxMessage.asArray, contor_buffer_usart1);
			}
			
			flag_stop_timer_usart1 = 0;
			flag_start_timer_usart1 = 0;
			contor_buffer_usart1 = 0;	
		}
	}while(1);
	return 0;
}


