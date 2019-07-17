#include "uart_ext.h"

byte App_TaskID_Ext;
uint8  stateExt;
uint8  CMD_Token_Ext[2];
uint8  LEN_Token_Ext;
uint8  FSC_Token_Ext;
mtOSALSerialDataExt_t  *pMsgExt;
uint8  tempDataLenExt;

void MT_UartInit_Ext ( void ) {
  halUARTCfg_t uartConfig;
  App_TaskID_Ext = 0;
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = MT_UART_DEFAULT_BAUDRATE_EXT;
  uartConfig.flowControl          = MT_UART_DEFAULT_OVERFLOW_EXT;
  uartConfig.flowControlThreshold = MT_UART_DEFAULT_THRESHOLD_EXT;
  uartConfig.rx.maxBufSize        = MT_UART_DEFAULT_MAX_RX_BUFF_EXT;
  uartConfig.tx.maxBufSize        = MT_UART_DEFAULT_MAX_TX_BUFF_EXT;
  uartConfig.idleTimeout          = MT_UART_DEFAULT_IDLE_TIMEOUT_EXT;
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = NULL;
  HalUARTOpen_Ext (&uartConfig);
}

void MT_UartRegisterTaskID_Ext( byte taskID ) {
  App_TaskID_Ext = taskID;
}

byte MT_UartCalcFCS_Ext( uint8 *msg_ptr, uint8 len ) {
  byte x;
  byte xorResult;

  xorResult = 0;

  for ( x = 0; x < len; x++, msg_ptr++ )
    xorResult = xorResult ^ *msg_ptr;

  return ( xorResult );
}

void MT_UartProcessZToolData_Ext( uint8 port, uint8 event ) {
  uint8  ch;
  uint8  bytesInRxBuffer;
  
  (void)event;  // Intentionally unreferenced parameter

  while (Hal_UART_RxBufLen(port))
  {
    HalUARTRead (port, &ch, 1);

    switch (stateExt)
    {
      case SOP_STATE_EXT:
        if (ch == MT_UART_SOF_EXT)
          stateExt = LEN_STATE_EXT;
        break;

      case LEN_STATE_EXT:
        LEN_Token_Ext = ch;

        tempDataLenExt = 0;

        /* Allocate memory for the data */
        pMsgExt = (mtOSALSerialDataExt_t *)osal_msg_allocate( sizeof ( mtOSALSerialDataExt_t ) +
                                                        MT_RPC_FRAME_HDR_SZ + LEN_Token_Ext );

        if (pMsgExt)
        {
          /* Fill up what we can */
          pMsgExt->hdr.event = CMD_SERIAL_MSG;
          pMsgExt->msg = (uint8*)(pMsgExt+1);
          pMsgExt->msg[MT_RPC_POS_LEN] = LEN_Token_Ext;
          stateExt = CMD_STATE1_EXT;
        }
        else
        {
          stateExt = SOP_STATE_EXT;
          return;
        }
        break;

      case CMD_STATE1_EXT:
        pMsgExt->msg[MT_RPC_POS_CMD0] = ch;
        stateExt = CMD_STATE2_EXT;
        break;

      case CMD_STATE2_EXT:
        pMsgExt->msg[MT_RPC_POS_CMD1] = ch;
        /* If there is no data, skip to FCS stateExt */
        if (LEN_Token_Ext)
        {
          stateExt = DATA_STATE_EXT;
        }
        else
        {
          stateExt = FCS_STATE_EXT;
        }
        break;

      case DATA_STATE_EXT:

        /* Fill in the buffer the first byte of the data */
        pMsgExt->msg[MT_RPC_FRAME_HDR_SZ + tempDataLenExt++] = ch;

        /* Check number of bytes left in the Rx buffer */
        bytesInRxBuffer = Hal_UART_RxBufLen(port);

        /* If the remain of the data is there, read them all, otherwise, just read enough */
        if (bytesInRxBuffer <= LEN_Token_Ext - tempDataLenExt)
        {
          HalUARTRead (port, &pMsgExt->msg[MT_RPC_FRAME_HDR_SZ + tempDataLenExt], bytesInRxBuffer);
          tempDataLenExt += bytesInRxBuffer;
        }
        else
        {
          HalUARTRead (port, &pMsgExt->msg[MT_RPC_FRAME_HDR_SZ + tempDataLenExt], LEN_Token_Ext - tempDataLenExt);
          tempDataLenExt += (LEN_Token_Ext - tempDataLenExt);
        }

        /* If number of bytes read is equal to data length, time to move on to FCS */
        if ( tempDataLenExt == LEN_Token_Ext )
            stateExt = FCS_STATE_EXT;

        break;

      case FCS_STATE_EXT:

        FSC_Token_Ext = ch;

        /* Make sure it's correct */
        if ((MT_UartCalcFCS_Ext ((uint8*)&pMsgExt->msg[0], MT_RPC_FRAME_HDR_SZ + LEN_Token_Ext) == FSC_Token_Ext))
        {
          osal_msg_send( App_TaskID_Ext, (byte *)pMsgExt );
        }
        else
        {
          /* deallocate the msg */
          osal_msg_deallocate ( (uint8 *)pMsgExt );
        }

        /* Reset the stateExt, send or discard the buffers at this point */
        stateExt = SOP_STATE_EXT;

        break;

      default:
       break;
    }
  }
}

void HalUARTInit_Ext(void) {
  HalUARTInitISR();
}

uint8 HalUARTOpen_Ext(halUARTCfg_t *config) {
  HalUARTOpenISR(config);
  return HAL_UART_SUCCESS_EXT;
}

uint16 HalUARTRead_Ext(uint8 *buf, uint16 len) {
  return HalUARTReadISR(buf, len);
}

uint16 HalUARTWrite_Ext(uint8 *buf, uint16 len) {
  return HalUARTWriteISR(buf, len);
}

void HalUARTSuspend_Ext( void ) {
  HalUARTSuspendISR();
}

void HalUARTResume_Ext( void ) {
  HalUARTResumeISR();
}

void HalUARTPoll_Ext( void ) {
  HalUARTPollISR();
}

uint16 Hal_UART_RxBufLen_Ext( void ) {
  return HalUARTRxAvailISR();
}
