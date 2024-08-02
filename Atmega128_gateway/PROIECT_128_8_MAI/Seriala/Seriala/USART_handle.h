
#ifndef USART_HANDLE_H_
#define USART_HANDLE_H_

//------------------------------------------------------------------------------------------------------
//Include fisiere de antet necesare pentru functionarea programului
//-------------------------------------------------------------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


//------------------------------------------------------------------------------------------------------
//define-uri pentru numarul maxim de registre si coduri de functii Modbus
//-------------------------------------------------------------------------------------------------------
#define NR_MAX_REGISTERS 17
#define APP_REGISTERS_NUMBER 17
#define READ_HOLDING_REGISTERS 0x03
#define WRITE_MULTIPLE_REGISTERS 0x10
#define WRITE_SINGLE_REGISTERS 6
#define DEVICE_FAILURE 4

//------------------------------------------------------------------------------------------------------
//Definire macro pentru lungimea antetului Modbus si alte dimensiuni specifice protocolului
//-------------------------------------------------------------------------------------------------------
#define MODBUS_HEADER_LENGTH 2
#define MODBUS_MAX_DATABLOCK_LENGTH 252
#define MODBUS_CRC_LENGTH 2
#define MODBUS_NDLEN_LENGTH 2
#define MODBUS_MAX_DATAGRAM_LENGTH MODBUS_HEADER_LENGTH + MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_CRC_LENGTH

//------------------------------------------------------------------------------------------------------
//Definire macro pentru ID-urile slave si gateway
//-------------------------------------------------------------------------------------------------------
#define ID_SLAVE 0x01
#define ID_GATEWAY 0x03

//------------------------------------------------------------------------
//Timeout pentru receptia RX
//---------------------------------------------------------------------------
#define TIMER_RX_USART0 100
#define TIMER_RX_USART1 100

//------------------------------------------------------------------------------------------------------
//Led pentru verificare transmisie tx
//-------------------------------------------------------------------------------------------------------
#define PORT_LED0	PORTC
#define PIN_LED0	PINC0

//------------------------------------------------------------------------------------------------------
//daca pinii DE si RE sunt 1 logic, atunci e TX on si daca pinii DE si RE sunt 0 logic atunci sunt pe modul RX
//-------------------------------------------------------------------------------------------------------
#define PORT_RE	PORTB
#define PIN_RE	PINB2
#define PORT_DE	PORTB
#define PIN_DE	PINB3

//------------------------------------------------------------------------
//Flag-uri de start si stop pentru timer
//---------------------------------------------------------------------------
uint16_t flag_start_timer_usart0;
uint16_t flag_stop_timer_usart0;

uint16_t flag_start_timer_usart1;
uint16_t flag_stop_timer_usart1;


//------------------------------------------------------------------------------------------------------
//Contor pentru timer RX
//-------------------------------------------------------------------------------------------------------
uint16_t contor_rx_timer_usart0;
uint16_t contor_rx_timer_usart1;

//------------------------------------------------------------------------
//contor pentru buffer rx
//---------------------------------------------------------------------------
uint8_t contor_buffer_usart0;
uint8_t contor_buffer_usart1;


//------------------------------------------------------------------------
//Structura pentru registrii PWM Modbus
//---------------------------------------------------------------------------
typedef struct _Modbus_Pwm {
	uint16_t h_reg_address;
	uint16_t Value_pwm ;
} Modbus_Pwm_t;


//------------------------------------------------------------------------------------------------------
//Structura pentru mesajele de protocol Modbus
//-------------------------------------------------------------------------------------------------------
typedef struct _ModbusProtocolMessageStruct {
	uint8_t address;
	uint8_t function_code;
	uint8_t dataBlock[MODBUS_MAX_DATABLOCK_LENGTH];
	uint8_t nDLEN;
} ModbusProtocolMessageStruct_t;


//------------------------------------------------------------------------------------------------------
//Uniune pentru mesajele de protocol Modbus, pentru acces fie ca structura, fie ca array
//-------------------------------------------------------------------------------------------------------
typedef union _ModbusProtocolMessage {
	ModbusProtocolMessageStruct_t asStruct;
	uint8_t asArray[MODBUS_MAX_DATABLOCK_LENGTH + MODBUS_NDLEN_LENGTH];
} ModbusProtocolMessage_t;


//------------------------------------------------------------------------------------------------------
//Structura pentru manipularea mesajelor de protocol Modbus
//-------------------------------------------------------------------------------------------------------
typedef struct _ModbusProtocolHandle{
	ModbusProtocolMessage_t txMessage;
	ModbusProtocolMessage_t rxMessage;
} ModbusProtocolHandle_t;

extern ModbusProtocolHandle_t modbus_message;
extern Modbus_Pwm_t Registers_pwm[APP_REGISTERS_NUMBER];

extern uint8_t send_leds_command[200]; //array incarcat cu valorile pentru pwm dupa ce se face reglarea automata
extern uint8_t byte_count;
extern uint8_t flag_slave_full;
extern uint8_t nr_bytes_send;
uint8_t flag_adresa_nu_exista;
uint8_t flag_valoare_gresita_pwm;

//------------------------------------------------------------------------
//Functii pentru trimiterea comenzilor de citire sau scriere pentru reglarea automata
//---------------------------------------------------------------------------
void TrimitComandaSenzor(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg);
void TrimitComandaLED(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg);

//------------------------------------------------------------------------------------------------------
//Functii pentru gestionarea exceptiilor, initializarea registrelor si procesarii comenzilor
//-------------------------------------------------------------------------------------------------------
void exceptions(ModbusProtocolHandle_t * Modbushandle, uint8_t exceptie);
void initializeRegisters(Modbus_Pwm_t *Registers, uint8_t startAddress, uint8_t numRegisters);
void ModbusGatewayProcessComm(ModbusProtocolHandle_t * Modbushandle, Modbus_Pwm_t * Reg);
uint16_t ModbusComputeCRCTOT(ModbusProtocolMessage_t *datagram);

//------------------------------------------------------------------------
//Functii pentru initializari
//---------------------------------------------------------------------------
void USART0_Init(unsigned int ubrr);
void USART1_Init(unsigned int ubrr);
void USART0_TX_SIR_SIZE(uint8_t *string, uint8_t size);
void USART0_TX_CHAR(uint8_t data);
void USART1_TX_SIR_SIZE(uint8_t *string, uint8_t size);
void USART1_TX_CHAR(uint8_t data);
void init_contor_timp(void);
void init_timer0(void);
void init_devices();
void init_ports();


void pinToggle(volatile uint8_t *port, uint8_t pin);

//------------------------------------------------------------------------------------------------------
//Definirea rutinelor de intrerupere pentru USART si Timer
//-------------------------------------------------------------------------------------------------------
ISR(USART1_RX_vect);
ISR(USART0_RX_vect);
ISR(TIMER0_COMP_vect);

#endif /* USART0_H_ */