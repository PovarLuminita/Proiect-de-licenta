 
 #include "USART_slave.h"
 //--------------------------------------------------------------------
 //variabile declarate extern
 //--------------------------------------------------------------------
 uint16_t adcValue;
 uint8_t nr_bytes_send;
 uint8_t flag_slave_full;
 uint8_t byte_count;
 ModbusProtocolHandle_t modbus_message;
 Modbus_Register_t Registers[APP_REGISTERS_NUMBER];

//--------------------------------------------------------------------
//functie pentru a calcula CRC-ul 
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
//functie care intializeaza registrii pentru modbus rtu
//--------------------------------------------------------------------
 void initializeRegisters(Modbus_Register_t *Registers, uint8_t startAddress, uint8_t numRegisters) {
	 
	 for(int i = 0; i < numRegisters; i++)
	 {
		 Registers[i].h_reg_address = startAddress+i; 
	 }
	 
	 //primii 5 registrii sunt pentru testare
	 Registers[0].Value = 0x23; 
	 Registers[1].Value = 0x24; 
	 Registers[2].Value = 0x25;	
	 Registers[3].Value = 0x26;	
	 Registers[4].Value = 0x27;	
	 
	 Registers[5].Value = 0; ///pentru citirea valorii adc
	 
 }
 
 //--------------------------------------------------------------------
 //Functie pentru exceptii
 //--------------------------------------------------------------------
 void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie)
 {
	 Modbushandle->txMessage.asStruct.address = Modbushandle->rxMessage.asStruct.address;
	 Modbushandle->txMessage.asStruct.function_code = Modbushandle->rxMessage.asStruct.function_code + 0x80;
	 Modbushandle->txMessage.asStruct.dataBlock[0] = exceptie; 
	 
	 nr_bytes_send = 3; //dimensiune 3 din cauza ca este formata din id+fc+exception
	 modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
	 
	 uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
	 uint8_t CRC_h = CRC >> 8;
	 uint8_t CRC_l = CRC & 0xff;

	 Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
	 Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
	 flag_slave_full = 1;
 }

//--------------------------------------------------------------------
//Functie pentru procesare mesaj de request
//--------------------------------------------------------------------
 void ModbusSlaveProcessComm(ModbusProtocolHandle_t * Modbushandle,  Modbus_Register_t * Reg)
 {
	 Reg[5].Value = adcValue; //initializez registrul cu valoarea din registrul pentru adc
	 
	 ///selectez in functie de fc ce response sa dau
	 switch(Modbushandle->rxMessage.asStruct.function_code)
	 {
		 case WRITE_SINGLE_REGISTERS: //0x06 : scriere pe un registru
		 {
			 uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; // first 2 bytes are the address of the register to write
			 uint16_t value_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; // second 2 bytes are value for register
			 
			 for(int j = 0 ; j < NR_MAX_REGISTERS; j++) ///parcurg toti registrii pe care ii am disponibili
			 {
				 flag_adresa_nu_exista = 1; //presupun ca adresa nu exista
				 
				 if(Reg[j].h_reg_address == start_address) ///daca adresa vreunui registru exista, atunci modifica valoarea
				 {
					 Reg[j].Value = value_registers;
					 flag_adresa_nu_exista = 0;
					 break;
				 }
			 }
			 
			 if(flag_adresa_nu_exista == 1) //daca adresa nu exista, exceptie
			 {
				 exceptions(&modbus_message, ILLEGAL_DATA_ADDRESS);
				 break;
			 }
			 
			 //--------------------------------------------------------------------
			 //Incarc response pentru gateway
			 //--------------------------------------------------------------------
			 Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			 Modbushandle->txMessage.asStruct.function_code = WRITE_SINGLE_REGISTERS;
			 
			 uint8_t nr_bytes_datablock = 0;
			 uint8_t contor_crc = 0;
			 for( int i = 0; i < 4; i++)
			 {
				 Modbushandle->txMessage.asStruct.dataBlock[i] = Modbushandle->rxMessage.asStruct.dataBlock[i];
				 contor_crc++;
				 nr_bytes_datablock++;
			 }
			 
			 //--------------------------------------------------------------------
			 //Calcul crc pentru mesajul de response
			 //--------------------------------------------------------------------
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
		 
		 case READ_HOLDING_REGISTERS: //0x03 : citirea registriilor
		 {
			 uint16_t start_address = (Modbushandle->rxMessage.asStruct.dataBlock[0]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[1]; // first 2 bytes are the address of the register to write
			 uint16_t quantity_of_registers = (Modbushandle->rxMessage.asStruct.dataBlock[2]<<8) | Modbushandle->rxMessage.asStruct.dataBlock[3]; // second 2 bytes are value for register
			 
			//arata cati bytes se folosesc pentru a scrie toate valorile registrilor
			 byte_count = Modbushandle->rxMessage.asStruct.dataBlock[3]*2;
			 
			//--------------------------------------------------------------------
			//Incarc raspuns pentru gateway
			//--------------------------------------------------------------------
			 Modbushandle->txMessage.asStruct.address = ID_SLAVE;
			 Modbushandle->txMessage.asStruct.function_code = READ_HOLDING_REGISTERS;
			 Modbushandle->txMessage.asStruct.dataBlock[0] = byte_count;
			 
			 
			 for(int i = 0 ; i < quantity_of_registers; i++) // trec prin cati registrii doresc sa citesc
			 {
				 flag_adresa_nu_exista = 1; ///presupun ca urmatoarea adresa nu exista
				 for(int j = 0 ; j < NR_MAX_REGISTERS; j++) ///parcurg toti registrii
				 {
					 if(Reg[j].h_reg_address == start_address+i) //daca adresa din registru este aceeasi cu adresa din request, atunci incarca valoarea
					 {
						 ///incarc valoarea de citit in modul big endian
						 uint8_t msb =  Reg[j].Value>>8; 
						 uint8_t lsb = (uint8_t)(Reg[j].Value &0xFF);
						 
						 Modbushandle->txMessage.asStruct.dataBlock[1+2*i] = msb;
						 Modbushandle->txMessage.asStruct.dataBlock[2+2*i] = lsb;
						 
						 flag_adresa_nu_exista = 0; //adresa a fost gasita
						 break;
					 }
				 }
				 
			 }
			 
			 ///daca adresa nu a fost gasita, exceptie
			 if(flag_adresa_nu_exista == 1)
			 {
				 exceptions(&modbus_message, ILLEGAL_DATA_ADDRESS);
				 break;
			 }
			 
			 ///calculez CRC
			 nr_bytes_send = MODBUS_HEADER_LENGTH + 1+ quantity_of_registers*2;
			 modbus_message.txMessage.asStruct.nDLEN = nr_bytes_send;
			 uint16_t CRC = ModbusComputeCRCTOT(&modbus_message.txMessage);
			 uint8_t CRC_h = CRC >> 8;
			 uint8_t CRC_l = CRC & 0xff;
			 
			 Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH] = CRC_l;
			 Modbushandle->txMessage.asStruct.dataBlock[nr_bytes_send - MODBUS_CRC_LENGTH +1] = CRC_h;
			 
			 flag_slave_full = 1; //buffer pentru raspuns este full
			 break;
		 }
	 }
 }

 bool ModbusCheckAddressSlave(ModbusProtocolMessage_t * datagram)
 {
	 return (datagram->asStruct.address == ID_SLAVE ); 
 }

 //The event represents that I received data on rx
 ISR(USART_RX_vect)
 {
	 modbus_message.rxMessage.asArray[contor_buffer_usart] = UDR0; //incarc bufferul pentru request
	 contor_rx_timer_usart = TIMER_RX_USART; ///la fiecare caracter astept maxim 20 ms
	 flag_start_timer_usart = 1; //activez flagul pentru timer
	 contor_buffer_usart++; //incrementez contorul
 }

//functie care trimite un sir de caractere
 void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size){
	 //daca ambii pini de si re sunt high --> TX mode
	 PORTD |= ( 1 << PIN_DE); /// PB3->  DE = High;
	 PORTD |= ( 1 << PIN_RE); /// PB2-> ~RE = High;
	 
	 for(int i = 0; i < size; i++)
	 {
		 USART0_TX_CHAR(string[i]);
	 }
 }

//functie care trimite doar un caracter
 void USART0_TX_CHAR(uint8_t data){
	 while(!(UCSR0A &(1<<UDRE0))); //asteapta pana cand bufferul de transmisie este gol
	 UDR0 = data; //pune datele in registru, trimite datele
	 
 }


 void init_porturi(void)
 {
	 DDRB  = 0xFF; // iesire
	 PORTB  = 0x00; // low
	 
	 DDRC  = 0x7E; // A0 ca intrare
	 PORTC = 0x00;
	 
	 DDRD  = 0xFF;
	 PORTD = 0x00;
 }

 void init_devices(void)
 {
	 cli();
	 init_porturi();
	 adc_init();
	 timer0_init();
	 sei();
 }

 void timer0_init(void){
	 TCCR0A |= (1 << WGM01); //modul ctc
	 
	 TCCR0B |= (0 << CS02)|(1 << CS01)|(1 << CS00); //prescaler 64

	 OCR0A = 0xF9; //249 in baza 10, valoarea este setata pentru un timer de 1ms
	 
	 TIMSK0 |= 1 << OCIE0A; 
 }

//initializez valoarea contorului
 void init_data(void){
	 contor_rx_timer_usart = TIMER_RX_USART ;
 }

//intrerupere de timer
 ISR(TIMER0_COMPA_vect){
	 
	 if(flag_start_timer_usart == 1){ //daca flagul este activ, atunci decrementez contorul
		 contor_rx_timer_usart--;
	 }
	 
	 if(contor_rx_timer_usart == 0){ //cand contorul ajunge la 0, inseamna ca cele 20 ms au trecut
		 flag_stop_timer_usart = 1;
		 contor_rx_timer_usart = TIMER_RX_USART;
	 }

 }


 void USART_Init(unsigned int ubrr)
 {
	 cli();
	 UBRR0H = (unsigned char) (ubrr>>8);// registru pentru baud rate
	 UBRR0L = (unsigned char) ubrr;
	 
	 UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);// intrerupere rx, receptie, transmisie
	 
	 //8 biti de date, 1 bit de stop
	 UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	 
	 sei();
 }
 
void adc_init(void)
{
	 ADMUX = 0x00;
	 ADCSRA = 0x00;

	 ADMUX |=  (1<<REFS0) ; //tensiune de alimentare ca referinta
	 ADCSRA |= (1<<ADIE)| (1<<ADATE); ///activez adc si intrerupere adc
	 ADCSRA |=  (1<<ADPS2) |(1<<ADPS1) | (1<<ADPS0) ;  //setez prescalarul 128
	 ADCSRB = (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0); //pentru modul de rulare libera
	 ADCSRA |= (1 << ADEN); //activare adc
	 
	 sei();
	 
	 ADCSRA |= (1<<ADSC);
	 
}
 
ISR(ADC_vect)
{
	 adcValue = ADCL;				 // 0x00ff
	 
	 adcValue |= (uint16_t)ADCH<<8;   // 0xff00 
}
 
void pinToggle(volatile uint8_t *port, uint8_t pin){
	 *port ^=  1 << pin;//
}

void pinSet(volatile uint8_t *port, uint8_t pin){
	 *port |=  1 << pin;//
}

void pinReset(volatile uint8_t *port, uint8_t pin){
	 *port &=  ~(1 << pin);//
}