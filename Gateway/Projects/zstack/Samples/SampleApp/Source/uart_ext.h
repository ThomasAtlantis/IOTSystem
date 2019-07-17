#ifndef UART_EXT_H
#define UART_EXT_H

#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Memory.h"
#include "hal_board_cfg.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_board.h"
#include "_hal_uart_isr.c"
#include "Onboard.h"
#include "MT_RPC.h"
#include "MT.h"

#define HAL_UART_BR_9600_EXT   0x00
#define HAL_UART_BR_19200_EXT  0x01
#define HAL_UART_BR_38400_EXT  0x02
#define HAL_UART_BR_57600_EXT  0x03
#define HAL_UART_BR_115200_EXT 0x04

#define HAL_UART_ONE_STOP_BIT_EXT       0x00
#define HAL_UART_TWO_STOP_BITS_EXT      0x01

#define HAL_UART_NO_PARITY_EXT          0x00
#define HAL_UART_EVEN_PARITY_EXT        0x01
#define HAL_UART_ODD_PARITY_EXT         0x02

#define HAL_UART_8_BITS_PER_CHAR_EXT    0x00
#define HAL_UART_9_BITS_PER_CHAR_EXT    0x01

#define HAL_UART_FLOW_OFF_EXT   0x00
#define HAL_UART_FLOW_ON_EXT    0x01

#define HAL_UART_PORT_0_EXT   0x00
#define HAL_UART_PORT_1_EXT   0x01
#define HAL_UART_PORT_MAX_EXT 0x02

#define  HAL_UART_SUCCESS_EXT        0x00
#define  HAL_UART_UNCONFIGURED_EXT   0x01
#define  HAL_UART_NOT_SUPPORTED_EXT  0x02
#define  HAL_UART_MEM_FAIL_EXT       0x03
#define  HAL_UART_BAUDRATE_ERROR_EXT 0x04

#define HAL_UART_RX_FULL_EXT         0x01
#define HAL_UART_RX_ABOUT_FULL_EXT   0x02
#define HAL_UART_RX_TIMEOUT_EXT      0x04
#define HAL_UART_TX_FULL_EXT         0x08
#define HAL_UART_TX_EMPTY_EXT        0x10

#define SOP_STATE_EXT      0x00
#define CMD_STATE1_EXT     0x01
#define CMD_STATE2_EXT     0x02
#define LEN_STATE_EXT      0x03
#define DATA_STATE_EXT     0x04
#define FCS_STATE_EXT      0x05

typedef void (*halUARTCBackExt_t) (uint8 port, uint8 event);

typedef struct {
  volatile uint16 bufferHead;
  volatile uint16 bufferTail;
  uint16 maxBufSize;
  uint8 *pBuffer;
} halUARTBufControlExt_t;


typedef union {
  bool paramCTS;
  bool paramRTS;
  bool paramDSR;
  bool paramDTR;
  bool paramCD;
  bool paramRI;
  uint16 baudRate;
  bool flowControl;
  bool flushControl;
} halUARTIoctlExt_t;

#define MT_UART_SOF_EXT                  0xFE
#define MT_UART_DEFAULT_OVERFLOW_EXT     FALSE
#define MT_UART_DEFAULT_BAUDRATE_EXT     HAL_UART_BR_115200_EXT
#define MT_UART_DEFAULT_THRESHOLD_EXT    MT_UART_THRESHOLD
#define MT_UART_DEFAULT_MAX_RX_BUFF_EXT  MT_UART_RX_BUFF_MAX
#define MT_UART_DEFAULT_MAX_TX_BUFF_EXT  MT_UART_TX_BUFF_MAX
#define MT_UART_DEFAULT_IDLE_TIMEOUT_EXT MT_UART_IDLE_TIMEOUT

typedef struct {
  osal_event_hdr_t  hdr;
  uint8             *msg;
} mtOSALSerialDataExt_t;

void MT_UartInit_Ext ( void );
void MT_UartRegisterTaskID_Ext( byte taskID );
byte MT_UartCalcFCS_Ext( uint8 *msg_ptr, uint8 len );
void MT_UartProcessZToolData_Ext( uint8 port, uint8 event );
void HalUARTInit_Ext(void);
uint8 HalUARTOpen_Ext(halUARTCfg_t *config);
uint16 HalUARTRead_Ext(uint8 *buf, uint16 len);
uint16 HalUARTWrite_Ext(uint8 *buf, uint16 len);
void HalUARTSuspend_Ext( void );
void HalUARTResume_Ext( void );
void HalUARTPoll_Ext( void );
uint16 Hal_UART_RxBufLen_Ext( void );

#endif // UART_EXT_H