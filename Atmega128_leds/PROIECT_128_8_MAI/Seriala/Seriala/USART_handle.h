
#ifndef USART_HANDLE_H_
#define USART_HANDLE_H_

//------------------------------------------------------------
//fisiere pentru biblioteci
//----------------------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

//------------------------------------------------------------
//Definirea unei constante pentru PWM activat
//----------------------------------------------------------------
#define PWM_ON 1

//------------------------------------------------------------
//Nr max de registrii, constante pentru functii modbus 
//----------------------------------------------------------------
#define NR_MAX_REGISTERS 17
#define APP_REGISTERS_NUMBER 17
#define READ_HOLDING_REGISTERS 0x03
#define WRITE_MULTIPLE_REGISTERS 0x10
#define WRITE_SINGLE_REGISTERS 0x06
#define NR_BYTES_NR_COILS 0x02
#define NR_BYTES_START_ADDRESS 0x02

//------------------------------------------------------------
//Coduri de eroare Modbus
//----------------------------------------------------------------
#define ILLEGAL_FC 0x01
#define ILLEGAL_DATA_ADDRESS 0x02
#define ILLEGAL_VALUE_COILS 0x03


//------------------------------------------------------------
//Definirea lungimilor pentru mesajele Modbus
//----------------------------------------------------------------
#define MODBUS_HEADER_LENGTH 2
#define MODBUS_MAX_DATABLOCK_LENGTH 252
#define MODBUS_CRC_LENGTH 2
#define MODBUS_NDLEN_LENGTH 2
#define MODBUS_MAX_DATAGRAM_LENGTH MODBUS_HEADER_LENGTH + MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_CRC_LENGTH


//------------------------------------------------------------
//Adrese slave ?i master pentru Modbus
//----------------------------------------------------------------
#define ID_SLAVE 0x01
#define ID_MASTER 0x03

//------------------------------------------------------------
// Coduri de func?ii pentru citire ?i scriere coil-uri
//----------------------------------------------------------------
#define READ_COILS 1
#define WRITE_COILS 15

//------------------------------------------------------------
// Timere pentru recep?ia USART0 ?i USART1
//----------------------------------------------------------------
#define TIMER_RX_USART0 300 //30 ms
#define TIMER_RX_USART1 300 //30 ms

//------------------------------------------------------------
//Definirea pinilor LED pe PORTB
//----------------------------------------------------------------
#define PORT_LED0	PORTB
#define PIN_LED0	PINB0
#define PORT_LED1	PORTB
#define PIN_LED1	PINB1
#define PORT_LED2	PORTB
#define PIN_LED2    PINB2
#define PORT_LED3	PORTB
#define PIN_LED3	PINB3
#define PORT_LED4	PORTB
#define PIN_LED4	PINB4
#define PORT_LED5	PORTB
#define PIN_LED5	PINB5
#define PORT_LED6	PORTB
#define PIN_LED6	PINB6
#define PORT_LED7	PORTB
#define PIN_LED7	PINB7

//------------------------------------------------------------
//Definirea pinilor LED pe PORTC
//----------------------------------------------------------------
#define PORT_LED8	PORTC
#define PIN_LED8	PINC0
#define PORT_LED9	PORTC
#define PIN_LED9	PINC1
#define PORT_LED10	PORTC
#define PIN_LED10	PINC2
#define PORT_LED11	PORTC
#define PIN_LED11	PINC3
#define PORT_LED12	PORTC
#define PIN_LED12	PINC4
#define PORT_LED13	PORTC
#define PIN_LED13	PINC5
#define PORT_LED14	PORTC
#define PIN_LED14	PINC6
#define PORT_LED15	PORTC
#define PIN_LED15	PINC7

//------------------------------------------------------------
//Definirea pinilor pentru RE ?i DE pentru activare mod tx sau rx
//----------------------------------------------------------------
#define PORT_RE	PORTD
#define PIN_RE	PIND0
#define PORT_DE	PORTD
#define PIN_DE	PIND1

//------------------------------------------------------------
//flaguri pentru timer start/stop, variabile contor pentru timer si buffer
//----------------------------------------------------------------
uint16_t flag_start_timer_usart0;
uint16_t flag_stop_timer_usart0;
uint16_t contor_rx_timer_usart0;
uint8_t contor_buffer_usart0;

//------------------------------------------------------------
//Structura pentru gestionarea registrelor PWM Modbus
//----------------------------------------------------------------
typedef struct _Modbus_Pwm {
	uint16_t h_reg_address;
	uint16_t Value_pwm ;
} Modbus_Pwm_t;

//------------------------------------------------------------
//Structura pentru mesajul Modbus Protocol
//----------------------------------------------------------------
typedef struct _ModbusProtocolMessageStruct {
	uint8_t address;
	uint8_t function_code;
	uint8_t dataBlock[MODBUS_MAX_DATABLOCK_LENGTH];
	uint8_t nDLEN;
} ModbusProtocolMessageStruct_t;

//------------------------------------------------------------
//Uniune pentru conversia intre structura si tabloul de octeti
//----------------------------------------------------------------
typedef union _ModbusProtocolMessage {
	ModbusProtocolMessageStruct_t asStruct;
	uint8_t asArray[MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_NDLEN_LENGTH];
} ModbusProtocolMessage_t;

//------------------------------------------------------------
//Structura de gestionare a mesajelor Modbus
//----------------------------------------------------------------
typedef struct _ModbusProtocolHandle{
	ModbusProtocolMessage_t txMessage;
	ModbusProtocolMessage_t rxMessage;
} ModbusProtocolHandle_t;

//------------------------------------------------------------
//Declara?ii externe pentru structuri 
//----------------------------------------------------------------
extern ModbusProtocolHandle_t modbus_message;
extern Modbus_Pwm_t Registers_pwm[APP_REGISTERS_NUMBER];

extern uint16_t value_byte; // variabila pentru a stoca valorile de 1 byte pe 2 bytes

//------------------------------------------------------------
//masti pentru comanda de coils
//----------------------------------------------------------------
extern uint16_t start_stop_mask;
extern uint16_t not_start_stop_mask;
extern uint16_t offset_mask;
extern uint16_t portB_result;
extern uint16_t set_read_mask;
extern uint16_t result_read_mask;

//------------------------------------------------------------
//variabila care calculeaza nr de bytes necesari
//----------------------------------------------------------------
extern uint8_t byte_count;

//------------------------------------------------------------
//variabila care este verificata cu registrii pentru pwm
//----------------------------------------------------------------
extern uint8_t test_contor_pwm;

extern uint8_t flag_slave_full;
extern uint8_t nr_bytes_send;
uint8_t flag_adresa_nu_exista;
uint8_t flag_valoare_gresita_pwm;
uint8_t valoare_portb;
uint8_t valoare_portc;
uint16_t valoare_portb_portc;

//------------------------------------------------------------
//Functie care verifica adresa slave
//----------------------------------------------------------------
bool ModbusCheckAddressSlave(ModbusProtocolMessage_t * datagram);

//------------------------------------------------------------
//Functie care initializeaza registrii pwm
//----------------------------------------------------------------
void initializeRegisters(Modbus_Pwm_t *Registers, uint8_t startAddress, uint8_t numRegisters);

//------------------------------------------------------------
//Functii pentru procesarea mesajelor, calcul crc si exceptii
//----------------------------------------------------------------
void ModbusSlaveProcessComm(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg);
uint16_t ModbusComputeCRCTOT(ModbusProtocolMessage_t *datagram);
void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie);

//------------------------------------------------------------
//Functii pentru initializari
//----------------------------------------------------------------
void USART0_Init(unsigned int ubrr);
void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size);
void USART0_TX_CHAR(uint8_t data);
void init_contor_timp(void);
void init_timer0(void);
void init_devices();
void init_ports();

//------------------------------------------------------------
//Functie pentru comanda pwm off
//----------------------------------------------------------------
void PWM_OFF();


//------------------------------------------------------------
//Intreruperi USART1, USART0 si timer
//----------------------------------------------------------------
ISR(USART1_RX_vect);
ISR(USART0_RX_vect);
ISR(TIMER0_COMP_vect);

void pinToggle(volatile uint8_t *port, uint8_t pin);
void pinSet(volatile uint8_t *port, uint8_t pin);
void pinReset(volatile uint8_t *port, uint8_t pin);

#endif /* USART0_H_ */