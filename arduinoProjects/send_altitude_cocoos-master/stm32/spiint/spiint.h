//  SPI Interface for STM32. Compatible with Arduino's Wire SPI interface.
//  The SPI Interface supports the Simulator Module that captures, replays and simulates SPI commands for SPI sensors.
//  We do this so that we can capture the SPI send/receive commands of Arduino sensor drivers and replay
//  them efficiently on STM32, with multitasking.  And also for testing with simulated sensors.
//  We support only Blue Pill SPI Port 1:
//  SS1   = Pin PA4
//  SCK1  = Pin PA5
//  MISO1 = Pin PA6
//  MOSI1 = Pin PA7
#ifndef SPIINT_H_
#define SPIINT_H_
#include <stdint.h>  //  For uint8_t
#include <stdlib.h>  //  For size_t
#include <cocoos.h>  //  For Evt_t

#define MAX_SPI_PORTS 2  //  Define 2 SPI ports: SPI1, SPI2.

#define USE_16BIT_SPI_TRANSFERS 0  //  Uncomment for 8-bit SPI transfer.
//  #define USE_16BIT_SPI_TRANSFERS 1  //  Uncomment for 16-bit SPI transfer.

//  From Arduino.h.  Used by BME280Spi.h.
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#if USE_16BIT_SPI_TRANSFERS
#define SPI_DATA_TYPE uint16_t
#else
#define SPI_DATA_TYPE uint8_t
#endif

#ifdef __cplusplus
extern "C" {  //  Allows functions below to be called by C and C++ code.
#endif

enum SPI_Fails {  //  Error codes.
	SPI_Ok = 0,
  SPI_Missing_Simulator,
	SPI_Invalid_Port,
	SPI_Invalid_Size,
	SPI_Mismatch,
	SPI_Timeout,
  SPI_End,  //  Insert new codes above.
};

#define kHz 1000  //  For defining the SPI port speed.
#define MHz 1000000
#define MAX_TRANS_STATUS 6  //  How many history entries to keep.

enum Trans_Status {  //  Status of the SPI transceive request.
	TRANS_NONE = 0,
  TRANS_TIMEOUT,      //  Timeout
  TRANS_TX_HALFDONE,  //  Transmit Half-done
  TRANS_TX_COMPLETE,  //  Transmit Complete
  TRANS_TX_ERROR,     //  Transmit Error
  TRANS_RX_HALFDONE,  //  Receive Half-done
  TRANS_RX_COMPLETE,  //  Receive Complete
  TRANS_RX_ERROR,     //  Receive Error
};

struct Simulator_Control;

struct SPI_Control {  //  Represents an STM32 SPI port, e.g. SPI1, SPI2.
  uint8_t id;         //  SPI port ID: 1=SPI1, 2=SPI2.
  uint8_t pin;        //  For Legacy Arduino support: Map this port to a pin number.
	SPI_Fails	failCode;            //  Last fail code.
  Simulator_Control *simulator;  //  Simulator for the port that will capture, replay and simulate SPI commands.

  //  SPI port configuration
  uint32_t speedMaximum; //  Max speed of comms, e.g. 500000 for 500 kHz.  Compatible with Arduino.
  uint8_t bitOrder;      //  MSBFIRST or LSBFIRST.  Compatible with Arduino.
  uint8_t dataMode;      //  SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3.  Compatible with Arduino.
  uint32_t timeout;      //  Timeout in milliseconds.

  //  SPI transceive
  Sem_t *tx_semaphore;  //  If not NULL, signal this semaphore when transmit has been completed.
  Sem_t *rx_semaphore;  //  If not NULL, signal this semaphore when receive has been completed.
  SPI_DATA_TYPE *tx_buf; int tx_len;  //  Last packet transmitted.
  SPI_DATA_TYPE *rx_buf; int rx_len;  //  Last packet received.
  volatile int rx_remainder;  //  Excess number bytes to be received after transmission.  Declared volatile because it's updated by the interrupt service routine.
  volatile Trans_Status transceive_status;  //  Status of SPI transmit/receive command.  Declared volatile because it's updated by the interrupt service routine.
  volatile Trans_Status transceive_history[MAX_TRANS_STATUS];  //  History of transceive status.  Declared volatile because it's updated by the interrupt service routine.

  //  STM32 SPI port definitions
	uint32_t SPIx;  					            //  SPI Port e.g. SPI1
	volatile uint32_t *ptr_SPI_DR;  	    //  SPI DR e.g. &SPI1_DR
	volatile uint32_t *ptr_SPI_I2SCFGR;   //  SPI I2S Config e.g. &SPI1_I2SCFGR
	uint32_t RCC_SPIx;			              //  SPI Clock e.g. RCC_SPI1

	//  STM32 GPIO definitions (port, pin, clock) for each SPI pin (SS, SCK, MISO, MOSI)
	uint32_t ss_port;   uint16_t ss_pin;   uint32_t ss_rcc;    //  SS pin e.g. GPIOA, GPIO4, RCC_GPIOA
	uint32_t sck_port;  uint16_t sck_pin;  uint32_t sck_rcc;   //  SCK pin e.g. GPIOA, GPIO5, RCC_GPIOA
	uint32_t miso_port; uint16_t miso_pin; uint32_t miso_rcc;  //  MISO pin e.g. GPIOA, GPIO6, RCC_GPIOA
	uint32_t mosi_port; uint16_t mosi_pin; uint32_t mosi_rcc;  //  MOSI pin e.g. GPIOA, GPIO7, RCC_GPIOA

	//  STM32 DMA definitions (port, channel, interrupt, clock) for SPI transmit and receive DMA channels.
	uint32_t tx_dma; uint8_t tx_channel; uint8_t tx_irq; uint32_t tx_rcc;  //  Transmit DMA e.g. DMA1, DMA_CHANNEL3, NVIC_DMA1_CHANNEL3_IRQ, RCC_DMA1
	uint32_t rx_dma; uint8_t rx_channel; uint8_t rx_irq; uint32_t rx_rcc;  //  Receive DMA e.g. DMA1, DMA_CHANNEL2, NVIC_DMA1_CHANNEL2_IRQ, RCC_DMA1
};

//  This is the new SPI Interface.  New code should use this.
SPI_Control *spi_setup(uint32_t spi_id);  	//  Enable SPI peripheral and GPIO clocks.  Should be called once only.
SPI_Fails spi_configure(  //  Configure the SPI port comms settings.
  SPI_Control *port,      //  SPI port.
  uint32_t speedMaximum,  //  Max speed of comms, e.g. 500000 for 500 kHz.  Compatible with Arduino.
  uint8_t bitOrder,       //  MSBFIRST or LSBFIRST.  Compatible with Arduino.
  uint8_t dataMode);      //  SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3.  Compatible with Arduino.
SPI_Fails spi_open(SPI_Control *port);  //  Enable DMA interrupt for SPI port.

//  Send an SPI command to transmit and receive SPI data via DMA and interrupts.  If the simulator is in...
//  Capture Mode: We capture the transmit/receive data into the simulator trail.
//  Replay mode: We replay the transmit/receive SPI command recorded in the simulator trail. Record the received data into the trail.
//  Simulate mode: We don't execute any SPI commands, just return the data received data from the trail.
//  Note: tx_buf and rx_buf MUST be buffers in static memory, not on the stack.
SPI_Fails spi_transceive(
	SPI_Control *port, 		  //  SPI port.
	SPI_DATA_TYPE *tx_buf,  //  Bytes to be transmitted to SPI port.	Must be in static memory, not stack.
	int tx_len, 			      //  Number of bytes to be transmitted.
	SPI_DATA_TYPE *rx_buf,  //  Buffer for receiving bytes from SPI port.
	int rx_len, 			      //  Number of bytes to be received. Must be in static memory, not stack.
	Sem_t *completed_semaphore  //  If not null, semaphore to be signalled upon completing the request.
	);

//  This function is the same as spi_transceive() except that it blocks until the result is received.  
//  Should only be used for legacy Arduino code.
//  New code should call spi_transceive() and pass a semaphore to be signalled.
//  Note: tx_buf and rx_buf MUST be buffers in static memory, not on the stack.
SPI_Fails spi_transceive_wait(
	SPI_Control *port, 		  //  SPI port.
	SPI_DATA_TYPE *tx_buf,  //  Bytes to be transmitted to SPI port.	Must be in static memory, not stack.
	int tx_len, 			      //  Number of bytes to be transmitted.
	SPI_DATA_TYPE *rx_buf,  //  Buffer for receiving bytes from SPI port.
	int rx_len); 			      //  Number of bytes to be received. Must be in static memory, not stack.

//  Replay the next transceive request that was captured earlier.  If completed_semaphore is non-NULL,
//  signal the semaphore when the request has been completed.
SPI_Fails spi_transceive_replay(SPI_Control *port, Sem_t *completed_semaphore);

bool spi_is_transceive_complete(SPI_Control *port);    //  Return true if last SPI command was completed successfully or with error.
bool spi_is_transceive_successful(SPI_Control *port);  //  Return true if last SPI command was successful.
SPI_Fails spi_wait(SPI_Control *port);   //  Wait until transceive is complete.  This call blocks, should only be used for legacy Arduino code.
SPI_Fails spi_close(SPI_Control *port);  //  Disable DMA interrupt for SPI port.
SPI_Fails spi_split_trail(SPI_Control *port);  //  Split the received merged packet into the simulator trail.
SPI_Fails spi_test(SPI_Control *port);         //  For testing only.
SPI_Fails spi_dump_trail(SPI_Control *port);   //  Dump the captured / replayed / simulated SPI packets to console.
SPI_Fails spi_dump_packet(SPI_Control *port);  //  Dump the last SPI packet to console.

#ifdef __cplusplus
}  //  End of extern C scope.
#endif

#ifdef __cplusplus  //  SPIInterface class for C++ only
//  This is the legacy SPI Interface for Arduino.  New code should NOT use this.
class SPIInterfaceSettings {
public:
  SPIInterfaceSettings(uint32_t speedMaximum, uint8_t bitOrder, uint8_t dataMode);  //  Used by BME280Spi.cpp
  // SPIInterfaceSettings();
  uint8_t port;           //  SPI port ID: 1=SPI1, 2=SPI2
  uint32_t speedMaximum;  //  Same as SPI_Control
  uint8_t bitOrder;       //  Same as SPI_Control
  uint8_t dataMode;       //  Same as SPI_Control
};

class SPIInterface {
public:
  // static void begin();
  // static void usingInterrupt(uint8_t interruptNumber);
  // static void notUsingInterrupt(uint8_t interruptNumber);
  static void beginTransaction(SPIInterfaceSettings settings);  //  Used by BME280Spi.cpp
  static uint8_t transfer(uint8_t data);  //  Used by BME280Spi.cpp
  // static uint16_t transfer16(uint16_t data);
  // static void transfer(void *buf, size_t count);
  static void endTransaction(void);  //  Used by BME280Spi.cpp
  // static void end();
  // static void setBitOrder(uint8_t bitOrder);
  // static void setDataMode(uint8_t dataMode);
  // static void setClockDivider(uint8_t clockDiv);
  // static void attachInterrupt();
  // static void detachInterrupt();

  //  From Arduino wiring_digital.h.  We intercept and identify the SPI port to be used.
  static void pinMode(uint8_t, uint8_t);  //  Used by BME280ASpi.h
  static void digitalWrite(uint8_t pin, uint8_t val);  //  Used by BME280ASpi.h
  // static int digitalRead(uint8_t);  //  Used by BME280SpiSw.cpp
};
#endif  //  __cplusplus
#endif  //  SPIINT_H_
