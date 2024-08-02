
#include "USART_handle.h"
uint8_t byte_count;
uint8_t flag_slave_full;
uint8_t nr_bytes_send;
uint8_t flag_adresa_nu_exista;
ModbusProtocolHandle_t modbus_message;
Modbus_Pwm_t Registers_pwm[APP_REGISTERS_NUMBER];
uint8_t send_leds_command[200];

//------------------------------------------------------------------------
//functie care incarca comanda pentru a citi registrul adc
//---------------------------------------------------------------------------
void TrimitComandaSenzor(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg)
{
	Modbushandle->txMessage.asStruct.address = 0x05;
	Modbushandle->txMessage.asStruct.function_code = 0x03;
	Modbushandle->txMessage.asStruct.dataBlock[0] = 0x00;
	Modbushandle->txMessage.asStruct.dataBlock[1] = 0x05;
	Modbushandle->txMessage.asStruct.dataBlock[2] = 0x00;
	Modbushandle->txMessage.asStruct.dataBlock[3] = 0x01;
	Modbushandle->txMessage.asStruct.dataBlock[4] = 0x95;
	Modbushandle->txMessage.asStruct.dataBlock[5] = 0x8F;
}

//------------------------------------------------------------------------
//in functie de valoarea citita de senzor incarc valorile pentru pwm si trimit comanda
//---------------------------------------------------------------------------
void TrimitComandaLED(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg)
{
	///raspuns: 05 03 02 02 9B crc crc
	uint16_t valoare_pwm = (Modbushandle->rxMessage.asStruct.dataBlock[1]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[2]; // first 2 bytes are the address of the register to write
	
	if(valoare_pwm >= 670)
	{
		for(int i = 8; i <= 38; i= i+2)
		{
			send_leds_command[i-1] = 0x00;
			send_leds_command[i] = 0x02;
		}	
	}
	else if(valoare_pwm >= 640 && valoare_pwm < 670)
	{
		for(int i = 8; i <= 38; i= i+2)
		{
			send_leds_command[i-1] = 0x00;
			send_leds_command[i] = 0x10;
		}
		
	}else if(valoare_pwm >= 590 && valoare_pwm < 640)
	{
		for(int i = 8; i <= 38; i= i+2)
		{
			send_leds_command[i-1] = 0x00;
			send_leds_command[i] = 0x30;
		}
	}
	else if(valoare_pwm >= 570 && valoare_pwm < 590)
	{
		for(int i = 8; i <= 38; i= i+2)
		{
			send_leds_command[i-1] = 0x00;
			send_leds_command[i] = 0x40;
		}
	}
	else if(valoare_pwm >= 510 && valoare_pwm < 570)
	{
		for(int i = 8; i <= 38; i= i+2)
		{
			send_leds_command[i-1] = 0x00;
			send_leds_command[i] = 0x60;
		}
	}
	
	send_leds_command[0] = 0x01;
	send_leds_command[1] = 0x10;
	send_leds_command[2] = 0x00;
	send_leds_command[3] = 0x06;
	send_leds_command[4] = 0x00;
	send_leds_command[5] = 0x10;
	send_leds_command[6] = 0x20;
	
	for(int i = 0; i <= 38; i++)
	{
		Modbushandle->txMessage.asArray[i] = send_leds_command[i];
	}

	nr_bytes_send = 39;
	modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
	uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
	uint8_t CRC_h = CRC >> 8;
	uint8_t CRC_l = CRC & 0xff;

	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
	
}

//functie care initializeaza valorile pentru registrii pwm
void initializeRegisters(Modbus_Pwm_t *Registers, uint8_t startAddress, uint8_t numRegisters) {
	for(int i = 0; i < numRegisters; i++)
	{
		Registers[i].h_reg_address = startAddress+i;
		Registers[i].Value_pwm = 1 + i;
	}
}

//functie care trimite exceptiile
void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie)
{
	Modbushandle->txMessage.asStruct.address = Modbushandle->rxMessage.asStruct.address;
	Modbushandle->txMessage.asStruct.function_code = Modbushandle->rxMessage.asStruct.function_code + 0x80;
	Modbushandle->txMessage.asStruct.dataBlock[0] = exceptie; 
	
	nr_bytes_send = 3; //id+fc+exception
	modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
	
	uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
	uint8_t CRC_h = CRC >> 8;
	uint8_t CRC_l = CRC & 0xff;

	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
	flag_slave_full = 1;
}


///functie care calculeaza CRC
uint16_t ModbusComputeCRCTOT(ModbusProtocolMessage_t *datagram)
{
	int j;
	int nDLEN;
	uint16_t CRC = 0xFFFF;
	
	nDLEN = datagram->asStruct.nDLEN; // ia lungimea fara cei 2 bytes de crc
	
	for(j = 0; j<nDLEN; j++)
	{
		CRC ^= (uint16_t)*(datagram->asArray + j); // XOR cu byte-ul în cel mai pu?in semnificativ byte al CRC
		for (int i = 8; i != 0; i--)
		{    // Parcurge fiecare bit
			if ((CRC & 0x0001) != 0)
			{      // Dac? LSB este setat
				CRC >>= 1;                    // Shift dreapta ?i XOR cu 0xA001
				CRC ^= 0xA001;
			}
			else                            // Altfel, LSB nu este setat
			CRC >>= 1;                      // Doar shift dreapta
		}
	}
	
	return 	CRC;
}

///functie care proceseaza cererea
void ModbusGatewayProcessComm(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg)
{
	switch(Modbushandle->rxMessage.asStruct.function_code)
	{
		case READ_HOLDING_REGISTERS: //0x03
		{
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; // first 2 bytes are the address of the register to write
			uint16_t quantity_of_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; // second 2 bytes are value for register
			
			byte_count = (uint8_t)quantity_of_registers*2;
			
			Modbushandle->txMessage.asStruct.address = ID_GATEWAY;
			Modbushandle->txMessage.asStruct.function_code = READ_HOLDING_REGISTERS;
			Modbushandle->txMessage.asStruct.dataBlock[0] = byte_count;
			
			
			for(int i = 0 ; i < quantity_of_registers; i++) // trec prin cati registrii doresc sa citesc
			{
				flag_adresa_nu_exista = 1;
				for(int j = 0 ; j < NR_MAX_REGISTERS; j++) 
				{
					if(Reg[j].h_reg_address == start_address+i)
					{
						uint8_t msb =  Reg[j].Value_pwm>>8;
						uint8_t lsb = (uint8_t)(Reg[j].Value_pwm &0xFF);
						
						Modbushandle->txMessage.asStruct.dataBlock[1+2*i] = msb;
						Modbushandle->txMessage.asStruct.dataBlock[2+2*i] = lsb;
						flag_adresa_nu_exista = 0; //adresa a fost gasita
						break;
					}
				}
			}
			
			if(flag_adresa_nu_exista == 1)
			{
				exceptions(&modbus_message, 2);
				break;
			}
			
			nr_bytes_send = MODBUS_HEADER_LENGTH + 1+ quantity_of_registers*2;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
	
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			
			flag_slave_full = 1;
			break;
		}
		
		case WRITE_SINGLE_REGISTERS: //0x06
		{
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; // first 2 bytes are the address of the register to write
			uint16_t value_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; // second 2 bytes are value for register
			
			for(int j = 0 ; j < NR_MAX_REGISTERS; j++) 
			{
				flag_adresa_nu_exista = 1;
				
				if(Reg[j].h_reg_address == start_address)
				{
					Reg[j].Value_pwm = value_registers;
					flag_adresa_nu_exista = 0;
					break;
				}
			}
			
			if(flag_adresa_nu_exista == 1)
			{
				exceptions(&modbus_message, 2);
				break;
			}
			
			Modbushandle->txMessage.asStruct.address = ID_GATEWAY;
			Modbushandle->txMessage.asStruct.function_code = WRITE_SINGLE_REGISTERS;
			
			uint8_t nr_bytes_datablock = 0;
			uint8_t contor_crc = 0;
			for( int i = 0; i < 4; i++)
			{
				Modbushandle->txMessage.asStruct.dataBlock[i] = Modbushandle->rxMessage.asStruct.dataBlock[i];
				contor_crc++;
				nr_bytes_datablock++;
			}
			
			nr_bytes_send = MODBUS_HEADER_LENGTH + nr_bytes_datablock;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
				
			flag_slave_full = 1;
			break;	
		}
		
		
		case WRITE_MULTIPLE_REGISTERS: //0x10
		{
			uint16_t start_address = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; // first 2 bytes are the address of the register to write
			uint16_t quantity_of_registers = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; // second 2 bytes are value for register
			
			byte_count = Modbushandle->rxMessage.asStruct.dataBlock[4]*2;
			
			for(int i =0 ;i < quantity_of_registers; i++)
			{
				flag_adresa_nu_exista = 1;
				for(int j = 0 ; j < NR_MAX_REGISTERS; j++) 
				{
					if(Reg[j].h_reg_address == start_address+i)
					{
						uint16_t value = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[5+ 2*i] <<8) |(Modbushandle->rxMessage.asStruct.dataBlock[6+ 2*i]);
						flag_adresa_nu_exista = 0;
						Reg[j].Value_pwm = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[5+ 2*i] <<8) |((Modbushandle->rxMessage.asStruct.dataBlock[6+ 2*i])& 0x00FF);
						break;
					}
				}
			}
			
			if(flag_adresa_nu_exista == 1)
			{
				exceptions(&modbus_message, 2);
				break;
			}
			
			if(flag_valoare_gresita_pwm == 1)
			{
				exceptions(&modbus_message, 3);
				break;
			}
			
			Modbushandle->txMessage.asStruct.address = ID_GATEWAY;
			Modbushandle->txMessage.asStruct.function_code = WRITE_MULTIPLE_REGISTERS;
			
			uint8_t nr_bytes_datablock = 0;
			uint8_t contor_crc = 0;
			for( int i = 0; i < 4; i++)
			{
				Modbushandle->txMessage.asStruct.dataBlock[i] = Modbushandle->rxMessage.asStruct.dataBlock[i];
				contor_crc++;
				nr_bytes_datablock++;
			}
			
			nr_bytes_send = MODBUS_HEADER_LENGTH + nr_bytes_datablock;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			flag_slave_full = 1;
			break;
		}
	}
}

// Intrerupere pentru USART0 la receptia unui byte
ISR(USART0_RX_vect)
{
	modbus_message.rxMessage.asArray[contor_buffer_usart0] = UDR0;// Salveaza byte-ul primit in buffer
	flag_start_timer_usart0 = 1;// Activeaza semnalizarea inceperii timer-ului
	contor_buffer_usart0++;// Incrementare contor buffer
}

ISR(USART1_RX_vect)
{
	modbus_message.rxMessage.asArray[contor_buffer_usart1] = UDR1;
	flag_start_timer_usart1 = 1;
	contor_buffer_usart1++;
}

// Transmitere sir de caractere prin USART0 de dimensiune specificata
void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size){
	for(int i = 0; i < size; i++)
	{
		USART0_TX_CHAR(string[i]); // Transmitere fiecare caracter din sir
	}
}


// Transmitere caracter prin USART0
void USART0_TX_CHAR(uint8_t data){
	while(!(UCSR0A &(1<<UDRE0))); // Asteapta pana cand registrul UDR0 este gol
	UDR0 = data; // Incarca caracterul in registrul de transmisie UDR0
}

//functie pentru a trimite un sir de caractere
void USART1_TX_SIR_SIZE(uint8_t *string, uint8_t size){
	PORTB |= ( 1 << PIN_DE); /// PB3->  DE = High;
	PORTB |= ( 1 << PIN_RE); /// PB2-> ~RE = High;
	for(int i = 0; i < size; i++)
	{
		USART1_TX_CHAR(string[i]);
	}	
}

//functie pentru a trimite un caracter
void USART1_TX_CHAR(uint8_t data){
	while(!(UCSR1A &(1<<UDRE1))); 
	UDR1 = data; 
}

// Initializare USART0 
void USART0_Init(unsigned int ubrr){
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); // Activare intrerupere receptie, activare receptie si transmisie
	
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00); // Setare dimensiune frame: 8 biti de date, fara bit de paritate, 1 bit de stop
	
}


void USART1_Init(unsigned int ubrr){
	UBRR1H = (unsigned char) (ubrr>>8); //registru pentru baud rate
	UBRR1L = (unsigned char) ubrr;

	UCSR1B |= (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1); //intrerupere rx, receptie si transmisie

	UCSR1C |= (1<<UCSZ11)|(1<<UCSZ10); // 8 biti
}

// Initializare contoare pentru timer
void init_contor_timp(void){
	contor_rx_timer_usart0 = TIMER_RX_USART0;
	contor_rx_timer_usart1 = TIMER_RX_USART1;
}

ISR(TIMER0_COMP_vect){
	if(flag_start_timer_usart1 == 1){
		contor_rx_timer_usart1--; 
	}
	
	if(contor_rx_timer_usart1 == 0){
		flag_stop_timer_usart1 = 1;
		contor_rx_timer_usart1 = TIMER_RX_USART1;
	}
	
	if(flag_start_timer_usart0 == 1){
		contor_rx_timer_usart0--;
	}
	
	
	if(contor_rx_timer_usart0 == 0){
		flag_stop_timer_usart0 = 1;
		contor_rx_timer_usart0 = TIMER_RX_USART0;
	}
}

void init_timer0(void){
	TCCR0 = 0x00;
	TCCR0 |= (1 << WGM01)|(0<<WGM00); // Setare mod de operare CTC
	TCCR0 |= (1 << CS02)|(0 << CS01)|(0 << CS00); //prescalar 64

	OCR0 = 0x00;
	OCR0 = 0xF9; //valoarea 249 - timer 1ms
	
	TIMSK = 0x00;
	TIMSK |= (1 << OCIE0); //activare timer
}

void init_ports()
{
	DDRB  = 0xFF; // Setare port B ca output
	PORTB  = 0x00; // Initializare port B cu valori low

	DDRC  = 0x7F;
	PORTC = 0x00;

	DDRD  = 0xFF;
	PORTD = 0x00;
}

// Initializare dispozitive
void init_devices()
{
	cli(); // Dezactivare intreruperi globale
	init_ports();
	init_timer0();
	sei(); // Activare intreruperi globale
}

void pinToggle(volatile uint8_t *port, uint8_t pin){
	*port ^=  1 << pin;
}
