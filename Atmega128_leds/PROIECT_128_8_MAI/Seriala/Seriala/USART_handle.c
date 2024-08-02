
#include "USART_handle.h"
uint16_t value_byte;
uint16_t start_stop_mask;
uint16_t not_start_stop_mask;
uint16_t offset_mask;
uint16_t result_portb;
uint16_t set_read_mask;
uint16_t result_read_mask;
uint8_t byte_count;
uint8_t flag_slave_full;
uint8_t nr_bytes_send;
uint8_t test_contor_pwm;
ModbusProtocolHandle_t modbus_message;
Modbus_Pwm_t Registers_pwm[APP_REGISTERS_NUMBER];

//functie pwm off
void PWM_OFF()
{
	if(Registers_pwm[0].Value_pwm != 1)
	{
		PORTB = 0;
		PORTC = 0;
	}
}

//functie pentru exceptii
void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie)
{
	Modbushandle->txMessage.asStruct.address = Modbushandle->rxMessage.asStruct.address;
	Modbushandle->txMessage.asStruct.function_code = Modbushandle->rxMessage.asStruct.function_code + 0x80;
	Modbushandle->txMessage.asStruct.dataBlock[0] = exceptie; // ILLEGAL FC
	
	nr_bytes_send = 3; //id+fc+exception
	modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
	
	uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
	uint8_t CRC_h = CRC >> 8;
	uint8_t CRC_l = CRC & 0xff;

	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
	Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
	flag_slave_full = 1;
}

//functie care verifica adresa slave
bool ModbusCheckAddressSlave(ModbusProtocolMessage_t * datagram)
{
	return (datagram->asStruct.address == ID_SLAVE ); // verific adresa slave
}

//functie initializare registrii
void initializeRegisters(Modbus_Pwm_t *Registers, uint8_t startAddress, uint8_t numRegisters) {
	for(int i = 0; i < numRegisters; i++)
	{
		Registers[i].h_reg_address = startAddress+i;
		Registers[i].Value_pwm = 1 + i;
	}
}

//functie care calculeaza crc
uint16_t ModbusComputeCRCTOT(ModbusProtocolMessage_t *datagram)
{
	int j;
	int nDLEN;
	uint16_t CRC = 0xFFFF;
	
	nDLEN = datagram->asStruct.nDLEN; 
	
	for(j = 0; j<nDLEN; j++)
	{
		CRC ^= (uint16_t)*(datagram->asArray + j); 
		for (int i = 8; i != 0; i--)
		{    
			if ((CRC & 0x0001) != 0)
			{     
				CRC >>= 1;                   
				CRC ^= 0xA001;
			}
			else                        
			CRC >>= 1;                   
		}
	}
	
	return 	CRC;
}

//functie care proceseaza mesajele modbus
void ModbusSlaveProcessComm(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg)
{
	start_stop_mask = 0;
	not_start_stop_mask = 0;
	offset_mask = 0;
	set_read_mask =0 ;
	result_read_mask = 0;
	
	switch(Modbushandle->rxMessage.asStruct.function_code)
	{
		case READ_HOLDING_REGISTERS: //0x03
		{
			//primii 2 octeti din bufferul de date din cerere contin adresa de start
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; 
			//urmatorii 2 coteti din bufferul de date din cerere contin cantitatea de registrii
			uint16_t quantity_of_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; 
			
			//calcul pentru cantitatea de octeti
			byte_count = (uint8_t)quantity_of_registers*2;
			
			//incarca id-ul nodului
			Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			//incarca codul functiei
			Modbushandle->txMessage.asStruct.function_code = READ_HOLDING_REGISTERS;
			//incarca cantitatea de octeti
			Modbushandle->txMessage.asStruct.dataBlock[0] = byte_count;
		
			for(int i = 0 ; i < quantity_of_registers; i++) // trec prin cati registrii trebuie cititi din cerere
			{
				flag_adresa_nu_exista = 1; //presupun ca adresa nu este gasita
				for(int j = 0 ; j < NR_MAX_REGISTERS; j++) //parcurg toti registrii disponibili
				{
					if(Reg[j].h_reg_address == start_address+i) //verific daca adresa registrului disponibil este aceeasi cu adresa din cerere
					{
						uint8_t msb =  Reg[j].Value_pwm>>8; //deplasez valoarea din registru pentru a putea fi incarcata in 2 valori de cate un octet
						uint8_t lsb = (uint8_t)(Reg[j].Value_pwm &0xFF); //incarc celalalt octet din valoare registrului in lsb
						
						Modbushandle->txMessage.asStruct.dataBlock[1+2*i] = msb; //incarc bufferul de date pentru transmisie cu valoare registrului impartita in cele 2 valori de cate un octet
						Modbushandle->txMessage.asStruct.dataBlock[2+2*i] = lsb;
						flag_adresa_nu_exista = 0; //adresa a fost gasita
						break;
					}
				}
			}
			
			//exceptie daca adresa nu este gasita
			if(flag_adresa_nu_exista == 1)
			{
				exceptions(&modbus_message, 2);
				break;
			}
			
			//calculez cati octeti trebuie sa parcurg pentru a calcula crc-ul
			nr_bytes_send = MODBUS_HEADER_LENGTH + 1+ quantity_of_registers*2;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			//incarc crc-ul calculat
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			
			//verific registrul pentru pwm off
			PWM_OFF();
			
			//flag care indica faptul ca bufferul de transmisie este pregatit
			flag_slave_full = 1;
			break;
		}
		
		case WRITE_SINGLE_REGISTERS: //0x06
		{
			//primii 2 octeti din bufferul de date sunt pentru adresa de start si urmatorii 2 octeti sunt pentru valoarea registrului de scris
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; 
			uint16_t value_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; 
			
			//verific daca se respecta contorul intre 1-99 pentru pwm
			if(value_registers == 0 || value_registers > 99)
			{
				exceptions(&modbus_message, 3);
				break;
			}
			
			//parcurg toti registrii disponibili
			for(int j = 0 ; j < NR_MAX_REGISTERS; j++) 
			{
				flag_adresa_nu_exista = 1;
				
				//verific daca registrul disponibil are adresa din cerere
				if(Reg[j].h_reg_address == start_address)
				{
					Reg[j].Value_pwm = value_registers; //incarc registrul cu noua valoare din cerere
					flag_adresa_nu_exista = 0;
					break;
				}
			}
			
			if(flag_adresa_nu_exista == 1)
			{
				exceptions(&modbus_message, 2);
				break;
			}
			
			Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			Modbushandle->txMessage.asStruct.function_code = WRITE_SINGLE_REGISTERS;
			
			uint8_t nr_bytes_datablock = 0;
			uint8_t contor_crc = 0;
			for( int i = 0; i < 4; i++)
			{
				//incarc bufferul de date pentru transmisie cu octetii din cerere 
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
			
			PWM_OFF();
			
			flag_slave_full = 1;
			break;
		}
		
		case WRITE_MULTIPLE_REGISTERS: //0x10
		{
			//primii 2 octeti din bufferul de date din cerere sunt cu adresa de start, urmatorii 2 octeti sunt cu cantitate de registrii pentru scris
			uint16_t start_address = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; 
			uint16_t quantity_of_registers = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; 
			
			//calcul numar de octeti
			byte_count = Modbushandle->rxMessage.asStruct.dataBlock[4]*2;
			
			
			//parcurg cantitatea de registrii
			for(int i =0 ;i < quantity_of_registers; i++)
			{
				flag_adresa_nu_exista = 1;
				flag_valoare_gresita_pwm = 1;
				//parcurg toti registrii disponibili
				for(int j = 0 ; j < NR_MAX_REGISTERS; j++) 
				{
					//verific daca adresa din cerere se gaseste in registrul disponibil
					if(Reg[j].h_reg_address == start_address+i)
					{
						//valoarea de scris pe registru este incarcata intr-o variabila de 2 octeti
						uint16_t value = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[5+ 2*i] <<8) |(Modbushandle->rxMessage.asStruct.dataBlock[6+ 2*i]);
						
						flag_adresa_nu_exista = 0;
						
						if(value > 0 && value <= 0x63)
						{
							//valoarea de scris in registru este pusa in registrul disponibil la adresa din cerere
							Reg[j].Value_pwm = (uint16_t)(Modbushandle->rxMessage.asStruct.dataBlock[5+ 2*i] <<8) |((Modbushandle->rxMessage.asStruct.dataBlock[6+ 2*i])& 0x00FF);
							flag_valoare_gresita_pwm = 0;
							break;
						}
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
			
			Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			Modbushandle->txMessage.asStruct.function_code = WRITE_MULTIPLE_REGISTERS;
			
			uint8_t nr_bytes_datablock = 0;
			uint8_t contor_crc = 0;
			for( int i = 0; i < 4; i++)
			{
				//bufferul de date pentru transmisie se incarca cu octetii din cerere care contin adresa de start si cantitatea de registrii
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
			
			PWM_OFF();
			break;
		}
		
		case WRITE_COILS:
		{
			//valoarea de scris pe bobine este incarcata intr-o variabila de 2 octeti
			value_byte = ((uint16_t)Modbushandle->rxMessage.asStruct.dataBlock[6] << 8) | ((uint16_t)Modbushandle->rxMessage.asStruct.dataBlock[5]);
			
			//cantitatea de octeti este incarcata in variabila byte_count
			byte_count = Modbushandle->rxMessage.asStruct.dataBlock[4];
			
			//adresa de start corespunde cu primii 2 octeti din bufferul de date din cerere
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; 
			
			//numarul de bobine de scris corespunde cu urmatorii 2 octeti din bufferul de date din cerere
			uint16_t stop_coils = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; 
			
			//masca pentru setarea bitilor de scris din cerere in functie de adresa de start si cate bobine trebuie scrise
			for(int i = start_address; i < start_address+stop_coils; i++)
			{
				start_stop_mask |= (1 << i);
			}
			
			//masca cu bitii care nu trebuie modificati
			not_start_stop_mask = ~(start_stop_mask);
			
			//masca pentru deplasarea valorii de scris pe bobine cu adresa de start
			offset_mask = (value_byte << start_address);
			
			//masca pentru a avea valoarea pe bitii care trebuie scrisi
			start_stop_mask = start_stop_mask & offset_mask;
			
			//masca pentru a nu modifica bitii deja setati pe portul b
			PORTB &= not_start_stop_mask;
			
			//deplasare masca pentru a lua octetul superior pentru portul c
			not_start_stop_mask = not_start_stop_mask >> 8;
			PORTC &= not_start_stop_mask;
			
			//operatie sau logic pentru a pune valoarea de scris pe port
			PORTB |= start_stop_mask;
			start_stop_mask = start_stop_mask >> 8;
			PORTC |= start_stop_mask;
			
			valoare_portb = PORTB;
			valoare_portc = PORTC;
			//incarc valoarea de pe port intr-o variabila de 2 octeti pentru a o folosi la functia de citire a bobinelor
			valoare_portb_portc = (valoare_portc << 8) | valoare_portb;
			
			//------------------------------------------
			uint8_t nr_bytes_datablock = 0;
			Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			Modbushandle->txMessage.asStruct.function_code = WRITE_COILS;
			for(int i = 0; i < NR_BYTES_START_ADDRESS+NR_BYTES_NR_COILS; i++)
			{
				//bufferul de date din cerere contine adresa de start si numarul de bobine care trebuie scrise
				Modbushandle->txMessage.asStruct.dataBlock[i] = Modbushandle->rxMessage.asStruct.dataBlock[i];
				nr_bytes_datablock++;
			}
			
			nr_bytes_send = MODBUS_HEADER_LENGTH + nr_bytes_datablock;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			
			flag_slave_full = 1;
			break;
			//------------------------------------------
		}
		
		case READ_COILS:
		{
			//adresa de start se gaseste in primii 2 octeti din bufferul de date din cerere
			uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; 
			
			//numarul de bobine care trebuie citite se gaseste in urmatorii 2 octeti din bufferul de date
			uint16_t stop_coils = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; 
			
			//calcul pentru numarul de octeti
			uint8_t byte_count = stop_coils / 8;
			if (stop_coils % 8 != 0) {
				byte_count++;
			}
			
			//masca pentru bitii care trebuie cititi
			for(int i = start_address; i < start_address+stop_coils; i++)
			{
				set_read_mask |= (1 << i);
			}
			
			//masca pentru starea bitilor care trebuie cititi
			result_read_mask = set_read_mask & valoare_portb_portc;
			
			//rezultatul cu starile bobinelor pentru functia de citit 
			result_read_mask = (result_read_mask >> start_address);
			
			//---------------------------------------------------------------------------
			
			Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			Modbushandle->txMessage.asStruct.function_code = READ_COILS;
			Modbushandle->txMessage.asStruct.dataBlock[0] = byte_count;
			
			for(int i = 0; i < byte_count; i++) //for pentru numarul de octeti
			{
				//incarc primul octet din variabila care contine valoarea de citit a bobinelor 
				Modbushandle->txMessage.asStruct.dataBlock[i+1] = result_read_mask & 0xFF;
				
				//deplasez valoarea de citit pentru a lua si octetul superior
				result_read_mask = result_read_mask >> 8;
			}
			
			nr_bytes_send = 3 + byte_count;
			modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			uint16_t CRC = ModbusComputeCRCTOT(&modbus_message);
			uint8_t CRC_h = CRC >> 8;
			uint8_t CRC_l = CRC & 0xff;
			
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			
			flag_slave_full = 1;
			break;
		}
		
		default:
		{
			exceptions(&modbus_message, 1);
			break;
		}
	}
}

//intrerupere de receptie
ISR(USART0_RX_vect)
{
	modbus_message.rxMessage.asArray[contor_buffer_usart0] = UDR0;
	contor_rx_timer_usart0 = TIMER_RX_USART0;
	flag_start_timer_usart0 = 1;
	contor_buffer_usart0++;
}

///functie pentru trimiterea unui sir
 void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size){
	 
	 PORTD |= ( 1 << PIN_DE); /// PB3->  DE = High;
	 PORTD |= ( 1 << PIN_RE); /// PB2-> ~RE = High;
	 
	 for(int i = 0; i < size; i++)
	 {
		 USART0_TX_CHAR(string[i]);
	 }
	 
 }

//functie pentru trimiterea unui caracter
void USART0_TX_CHAR(uint8_t data){
	while(!(UCSR0A &(1<<UDRE0)));
	UDR0 = data; 
}

void USART0_Init(unsigned int ubrr){
	UBRR0H = (unsigned char) (ubrr>>8); //registru pentru baud rate
	UBRR0L = (unsigned char) ubrr;
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);  //intrerupere rx, receptie si transmisie
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);  // 8 biti
}

void init_contor_timp(void){
	contor_rx_timer_usart0 = TIMER_RX_USART0;
}

ISR(TIMER0_COMP_vect){
	// pwm pentru led-uri implementate cu timer
	if(Registers_pwm[0].Value_pwm == PWM_ON)
	{
		test_contor_pwm++;
		
		if(test_contor_pwm == Registers_pwm[1].Value_pwm)
		{
			pinReset(&PORT_LED0, PIN_LED0);
		}
		
		if(test_contor_pwm == Registers_pwm[2].Value_pwm)
		{
			pinReset(&PORT_LED1, PIN_LED1);
		}
		
		if(test_contor_pwm == Registers_pwm[3].Value_pwm)
		{
			pinReset(&PORT_LED2, PIN_LED2);
		}
		
		if(test_contor_pwm == Registers_pwm[4].Value_pwm)
		{
			pinReset(&PORT_LED3, PIN_LED3);
		}
		
		
		if(test_contor_pwm == Registers_pwm[5].Value_pwm)
		{
			pinReset(&PORT_LED4, PIN_LED4);
		}
		
		if(test_contor_pwm == Registers_pwm[6].Value_pwm)
		{
			pinReset(&PORT_LED5, PIN_LED5);
		}
		
		if(test_contor_pwm == Registers_pwm[7].Value_pwm)
		{
			pinReset(&PORT_LED6, PIN_LED6);
		}
		
		
		if(test_contor_pwm == Registers_pwm[8].Value_pwm)
		{
			pinReset(&PORT_LED7, PIN_LED7);
		}
		
		
		if(test_contor_pwm == Registers_pwm[9].Value_pwm)
		{
			pinReset(&PORT_LED8, PIN_LED8);
		}
		
		if(test_contor_pwm == Registers_pwm[10].Value_pwm)
		{
			pinReset(&PORT_LED9, PIN_LED9);
		}
		
		if(test_contor_pwm == Registers_pwm[11].Value_pwm)
		{
			pinReset(&PORT_LED10, PIN_LED10);
		}
		
		if(test_contor_pwm == Registers_pwm[12].Value_pwm)
		{
			pinReset(&PORT_LED11, PIN_LED11);
		}
		
		
		if(test_contor_pwm == Registers_pwm[13].Value_pwm)
		{
			pinReset(&PORT_LED12, PIN_LED12);
		}
		
		
		if(test_contor_pwm == Registers_pwm[14].Value_pwm)
		{
			pinReset(&PORT_LED13, PIN_LED13);
		}
		
		
		if(test_contor_pwm == Registers_pwm[15].Value_pwm)
		{
			pinReset(&PORT_LED14, PIN_LED14);
		}
		
		
		if(test_contor_pwm == Registers_pwm[16].Value_pwm)
		{
			pinReset(&PORT_LED15, PIN_LED15);
		}
		
		if(test_contor_pwm == 100)
		{
			PORTB = 0xFF;
			PORTC = 0xFF;
			test_contor_pwm = 0;
		}	
	}
	
	//decrementare contor daca flagul de start este 1
	if(flag_start_timer_usart0 == 1){
		contor_rx_timer_usart0--;
	}
	
	//receptia s-a incheiat
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
	OCR0 = 25; //timer 100us
	
	TIMSK = 0x00;
	TIMSK |= (1 << OCIE0); //activare timer
}

void init_ports()
{
	DDRB  = 0xFF; 
	PORTB  = 0x00; 

	DDRC  = 0xFF;
	PORTC = 0x00;

	DDRD  = 0xFF;
	PORTD = 0x00;
}

void init_devices()
{
	cli();
	init_ports();
	init_timer0();
	sei();
}

void pinToggle(volatile uint8_t *port, uint8_t pin){
	*port ^=  1 << pin;
}

void pinSet(volatile uint8_t *port, uint8_t pin){
	*port |=  1 << pin;
}

void pinReset(volatile uint8_t *port, uint8_t pin){
	*port &=  ~(1 << pin);
}

