#ifndef USART_SLAVE_H_
#define USART_SLAVE_H_

//--------------------------------------------------------------------
//Bibliotecile incluse
//--------------------------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

//--------------------------------------------------------------------
//LED verificare
//--------------------------------------------------------------------
#define PORT_LED0	PORTB
#define PIN_LED0	PINB0

//--------------------------------------------------------------------
//Pinii RE si DE pentru convertor rs485
//--------------------------------------------------------------------
#define PORT_RE	PORTD
#define PIN_RE	PIND2
#define PORT_DE	PORTD
#define PIN_DE	PIND3

#define ID_SLAVE 5  // ID SLAVE 2

//--------------------------------------------------------------------
//Functii modbus RTU folosite
//--------------------------------------------------------------------
#define READ_HOLDING_REGISTERS 0x03
#define WRITE_SINGLE_REGISTERS 0x06

#define APP_REGISTERS_NUMBER 6 //nr maxim de registrii pentru testare, unul este pentru adc
#define NR_MAX_REGISTERS 6 // nr maxim de registrii 

//--------------------------------------------------------------------
//exceptie
//--------------------------------------------------------------------
#define ILLEGAL_DATA_ADDRESS 2

//--------------------------------------------------------------------
//dimensiune maxima pentru modbus rtu
//--------------------------------------------------------------------
#define MODBUS_HEADER_LENGTH 2
#define MODBUS_MAX_DATABLOCK_LENGTH 252
#define MODBUS_CRC_LENGTH 2
#define MODBUS_NDLEN_LENGTH 2
#define MODBUS_MAX_DATAGRAM_LENGTH MODBUS_HEADER_LENGTH + MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_CRC_LENGTH

//--------------------------------------------------------------------
//valoarea cu care se initializeaza controul
//--------------------------------------------------------------------
#define TIMER_RX_USART 20

uint8_t flag_adresa_nu_exista; //este o variabila care verifica daca adresa se gaseste in adresele disponibile

//--------------------------------------------------------------------
//flaguri si contor pentru timer
//--------------------------------------------------------------------
uint16_t flag_start_timer_usart;
uint16_t flag_stop_timer_usart;
uint16_t contor_rx_timer_usart;

//--------------------------------------------------------------------
//contor pentru indexul bufferului din intreruperea de rx
//--------------------------------------------------------------------
uint8_t contor_buffer_usart;

//--------------------------------------------------------------------
//structura pentru registrii modbus rtu
//--------------------------------------------------------------------
typedef struct _Modbus_Register {
	uint16_t h_reg_address;
	uint16_t Value ;
} Modbus_Register_t;

//--------------------------------------------------------------------
//structura pentru forma mesajului
//--------------------------------------------------------------------
typedef struct _ModbusProtocolMessageStruct {
	uint8_t address;
	uint8_t function_code;
	uint8_t dataBlock[MODBUS_MAX_DATABLOCK_LENGTH];
	uint8_t nDLEN;
} ModbusProtocolMessageStruct_t;

//--------------------------------------------------------------------
//structura pentru gestionarea unui array in functie de structura mesajului
//--------------------------------------------------------------------
typedef union _ModbusProtocolMessage {
	ModbusProtocolMessageStruct_t asStruct;
	uint8_t asArray[MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_NDLEN_LENGTH];
} ModbusProtocolMessage_t;

//--------------------------------------------------------------------
//structura care contine request de la master si ce response incarc 
//--------------------------------------------------------------------
typedef struct _ModbusProtocolHandle{
	ModbusProtocolMessage_t txMessage;
	ModbusProtocolMessage_t rxMessage;
} ModbusProtocolHandle_t;

extern ModbusProtocolHandle_t modbus_message; //obiect din structura protocol_handle
extern Modbus_Register_t Registers[APP_REGISTERS_NUMBER]; //array de obiecte pentru gestionarea registrilor

extern uint8_t flag_slave_full; // flag care arata cand bufferul de response este full
extern uint8_t nr_bytes_send; // arata dimensiunea necesara pentru calculare crc
extern uint8_t byte_count; //variabila care calculeaza bc pentru response
extern uint16_t adcValue; //variabila care pastreaza valoarea din registru pt adc

//--------------------------------------------------------------------
//functii pentru initializari
//--------------------------------------------------------------------
void init_devices(void); 
void init_porturi(void);
void adc_init(void);
void timer0_init(void);
void init_data(void);
void USART_Init(unsigned int ubrr);
void initializeRegisters(Modbus_Register_t *registers, uint8_t startAddress, uint8_t numRegisters);

//--------------------------------------------------------------------
//functie pentru exceptii
//--------------------------------------------------------------------
void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie);

//--------------------------------------------------------------------
//functii pentru toggle/ on/ off la led
//--------------------------------------------------------------------
void pinToggle(volatile uint8_t *port, uint8_t pin);
void pinSet(volatile uint8_t *port, uint8_t pin);
void pinReset(volatile uint8_t *port, uint8_t pin);

//--------------------------------------------------------------------
//functie pentru procesarea request modbus, calcul crc si verificare id 
//--------------------------------------------------------------------
void ModbusSlaveProcessComm(ModbusProtocolHandle_t * Modbushandle, Modbus_Register_t * Reg);
uint16_t ModbusComputeCRCTOT(ModbusProtocolMessage_t *datagram);
bool ModbusCheckAddressSlave(ModbusProtocolMessage_t * datagram);

//--------------------------------------------------------------------
//functii pentru transmiterea unui caracter sau a unui sir de caractere
//--------------------------------------------------------------------
void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size);
void USART0_TX_CHAR(uint8_t data);


//--------------------------------------------------------------------
//intreruperi usart, timer si adc
//--------------------------------------------------------------------
ISR(USART_RX_vect);
ISR(TIMER0_COMPA_vect);
ISR(ADC_vect);

#endif /* USART_SLAVE_H_ */