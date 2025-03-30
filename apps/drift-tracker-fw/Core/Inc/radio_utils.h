//// helper functions for initializeing the radio
//// author: Kaden Du
//
#ifndef RADIO_UTILS_H
#define RADIO_UTILS_H
//
#ifdef __cplusplus
extern "C"
{
#endif
//
#include "radio.h"
//#include "sys_app.h"
//
//
#define RX_TIMEOUT_VALUE              3000
#define TX_TIMEOUT_VALUE              3000
/* PING string*/
#define PING "PING"
/* PONG string*/
#define PONG "PONG"
/*Size of the payload to be sent*/
/* Size must be greater of equal the PING and PONG*/
#define MAX_APP_BUFFER_SIZE          255
#if (PAYLOAD_LEN > MAX_APP_BUFFER_SIZE)
#error PAYLOAD_LEN must be less or equal than MAX_APP_BUFFER_SIZE
#endif /* (PAYLOAD_LEN > MAX_APP_BUFFER_SIZE) */
/* wait for remote to be in Rx, before sending a Tx frame*/
#define RX_TIME_MARGIN                200
/* Afc bandwidth in Hz */
#define FSK_AFC_BANDWIDTH             83333
/* LED blink Period*/
#define LED_PERIOD_MS                 200

#define RF_FREQUENCY 				  903000000

#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#ifndef TX_OUTPUT_POWER   /* please, to change this value, redefine it in USER CODE SECTION */
#define TX_OUTPUT_POWER                             20        /* dBm */ // was 14
#endif /* TX_OUTPUT_POWER */

#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       12         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

//#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))
//
//#define FSK_FDEV                                    25000     /* Hz */
//#define FSK_DATARATE                                50000     /* bps */
//#define FSK_BANDWIDTH                               50000     /* Hz */
//#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
//#define FSK_FIX_LENGTH_PAYLOAD_ON                   false
//
//#else
//#error "Please define a modem in the compiler subghz_phy_app.h."
//#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define PAYLOAD_LEN                                 64
//
//
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/*Ping Pong FSM states */
//static States_t State = RX;
/* App Rx Buffer*/
static uint8_t BufferRx[MAX_APP_BUFFER_SIZE];
/* App Tx Buffer*/
static uint8_t BufferTx[MAX_APP_BUFFER_SIZE];
/* Last  Received Buffer Size*/
uint16_t RxBufferSize = 0;
/* Last  Received packer Rssi*/
int8_t RssiValue = 0;
/* Last  Received packer SNR (in Lora modulation)*/
int8_t SnrValue = 0;
/* Led Timers objects*/
/* device state. Master: true, Slave: false*/
bool isMaster = true;
/* random delay to make sure 2 devices will sync*/
/* the closest the random delays are, the longer it will
   take for the devices to sync when started simultaneously*/
static int32_t random_delay;
static RadioEvents_t RadioEvents;

int TRANSMIT_FLAG = 0;
char received_message[70];
int RSSI_last;
//
//extern void uart_print(char *msg);
//
typedef enum
{
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;




static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
//  APP_LOG(TS_ON, VLEVEL_L, "OnTxDone\n\r");
	uart_print("TX Done\n");
	TRANSMIT_FLAG = 0;
  /* Update the State of the FSM*/
//  State = TX;

  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
//  APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
//	uart_print("Rx Done\n");

	// store last in a buffer to be called when asked
	memcpy(received_message, payload, sizeof(received_message));
	RSSI_last = rssi;
//	uart_print("Received Packet: ");
//	uart_print((char*)payload);
//	uart_print(" with ");
//	sprintf(buffer, "RSSI: %d dBm\n", rssi);
//	uart_print(buffer);

//#if ((USE_MODEM_LORA == 1) && (USE_MODEM_FSK == 0))
////  APP_LOG(TS_ON, VLEVEL_L, "RssiValue=%d dBm, SnrValue=%ddB\n\r", rssi, LoraSnr_FskCfo);
//
//  /* Record payload Signal to noise ratio in Lora*/
//  SnrValue = LoraSnr_FskCfo;
//#endif /* USE_MODEM_LORA | USE_MODEM_FSK */
//  /* Update the State of the FSM*/
////  State = RX;
//  /* Clear BufferRx*/
//  memset(BufferRx, 0, MAX_APP_BUFFER_SIZE);
//  /* Record payload size*/
//  RxBufferSize = size;
//  if (RxBufferSize <= MAX_APP_BUFFER_SIZE)
//  {
//    memcpy(BufferRx, payload, RxBufferSize);
//  }
  /* Record Received Signal Strength*/
//  RssiValue = rssi;
  /* Record payload content*/
//  APP_LOG(TS_ON, VLEVEL_H, "payload. size=%d \n\r", size);
//  for (int32_t i = 0; i < PAYLOAD_LEN; i++)
//  {
////    APP_LOG(TS_OFF, VLEVEL_H, "%02X", BufferRx[i]);
//    if (i % 16 == 15)
//    {
////      APP_LOG(TS_OFF, VLEVEL_H, "\n\r");
//    }
//  }
////  APP_LOG(TS_OFF, VLEVEL_H, "\n\r");

  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
//  APP_LOG(TS_ON, VLEVEL_L, "OnTxTimeout\n\r");
	uart_print("OnTxTimeout\n");
  /* Update the State of the FSM*/
//  State = TX_TIMEOUT;

  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
//  APP_LOG(TS_ON, VLEVEL_L, "OnRxTimeout\n\r");
	uart_print("OnRxTimeout\n");
  /* Update the State of the FSM*/
//  State = RX_TIMEOUT;

  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
//  APP_LOG(TS_ON, VLEVEL_L, "OnRxError\n\r");
	uart_print("OnRxError\n");
  /* Update the State of the FSM*/
//  State = RX_ERROR;

  /* USER CODE END OnRxError */
}

void Radio_Init() {
    // Initialize Radio Events
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    // Initialize the Radio
    Radio.Init(&RadioEvents);

    // Set frequency
    Radio.SetChannel(RF_FREQUENCY);

    // Configure TX and RX
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);

    // Clear TX buffer
    memset(BufferTx, 0x0, MAX_APP_BUFFER_SIZE);

    uart_print("Radio Initialized\n");
}

void Radio_Transmit(char *msg, int length) {
	TRANSMIT_FLAG = 1;
    memset(BufferTx, 0, MAX_APP_BUFFER_SIZE);  // Ensure clean buffer
    memcpy(BufferTx, msg, length);        // Copy only actual data
    Radio.Send(BufferTx, length);         // Send correct length
    uart_print("Transmitting...\n");
}

void Receive(int Timeout) {
	Radio.Rx(Timeout);
}

void print_last() {
	uint8_t buffer[10];
//	uart_print("Received Packet: ");
	uart_print((char*)received_message);
//	uart_print(" with ");
	sprintf(buffer, ",%d\n", RSSI_last);
	uart_print(buffer);
}

#ifdef __cplusplus
}
#endif
//
#endif // __RADIO_H__
