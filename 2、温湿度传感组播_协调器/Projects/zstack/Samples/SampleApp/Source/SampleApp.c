#include "OSAL.h"
#include "OSAL_Nv.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "SampleApp.h"
#include "SampleAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "MT_UART.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define UartDefaultRxLen 64
#define UartDefaultTxLen 64
#define Key_S1 P0_0
#define Key_S2 P0_1
#define Key_Active 0
#define LED_RED P1_0
#define LED_YELLOW P1_1
#define LED_ORANGE P1_4

// ¿ÉÐÐ·¶Î§£º0x0401 ¡ª 0x0FFF
#define ZD_NV_IP_ID 0x0440
#define ZD_NV_PORT_ID 0x0430
#define ZD_NV_SSID_ID 0x0420
#define ZD_NV_PSWD_ID 0x0410

// ³¤¶È¶¨Òå£¬ÐèÎª4µÄÕûÊý±¶
// Êµ¼Ê³¤¶È <= LENGTH - 1
#define SSID_MAX_LENGTH 20
#define PSWD_MAX_LENGTH 20
#define PORT_MAX_LENGTH 8
#define IP_MAX_LENGTH   16

#define isPressed(x) (x == Key_Active)
#define print(x,...) _UARTSend(1,x,##__VA_ARGS__)
#define debug(x,...) _UARTSend(0,x,##__VA_ARGS__)
#define debug_and_print(x,...) do{\
	_UARTSend(0,x,##__VA_ARGS__);\
	_UARTSend(1,x,##__VA_ARGS__);\
}while(0)

void _UARTSend(uint8 port, uint8 *fmt, ...);
void _UARTRead(uint8 port, uint8 *buf, uint16 *len);
void _delay_us(uint16 n);
void _delay_ms(uint16 n);
uint8 wait_for(uint8 *str, uint8 *err, uint16 timeout);
void exit_send(void);
uint8 _zigbeeSend(uint8 *fmt, ...);
uint16 WiFiRecvApp(uint8 *buff);
uint8 WiFiSendApp(uint8 *fmt, ...);
void WiFiSendServer(uint8 *fmt, ...);
uint16 WiFiRecvServer(uint8 *buff);

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_PERIODIC_CLUSTERID,
  SAMPLEAPP_FLASH_CLUSTERID,
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

uint8 SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr;
afAddrType_t SampleApp_Flash_DstAddr; //×é²¥

aps_Group_t SampleApp_Group; //×é

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;

uint8 _zigbeeRecvFlag = 0;
uint8 _zigbeeSendCnt = 0;
uint8 _zigbeeSendBuffer[UartDefaultRxLen];
uint8 _zigbeeSendTimes = 0;
uint8 _jumpBackID = 0;
uint8 temp_humid[12];

void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );
void SampleApp_SendFlashMessage( uint16 flashTime );
void SampleApp_SendGroupMessage(void); //Íø·ä×é²¥Í¨Ñ¶·¢ËÍº¯Êý¶¨Òå.

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;
  
  MT_UartInit();//´®¿Ú³õÊ¼»¯
  MT_UartRegisterTaskID(task_id);//µÇ¼ÇÈÎÎñºÅ
  HalUARTWrite(0,"Hello World\n",12); //£¨´®¿Ú0£¬'×Ö·û'£¬×Ö·û¸öÊý¡££©
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;
  

  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1
  SampleApp_Group.ID = SAMPLEAPP_FLASH_GROUP;//0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7  );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );
  

#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  halUARTCfg_t uartConfig;
	uint8 _buffer[UartDefaultRxLen];
	uint8 InitNVStatus, readNVStatus, writeNVStatus;
	uint8 SSID[SSID_MAX_LENGTH], PSWD[PSWD_MAX_LENGTH];
	uint8 PORT[PORT_MAX_LENGTH], MYIP[IP_MAX_LENGTH];
	uint16 length, nv_id, nv_len, prefix_len, cnt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // Received when a key is pressed
        case KEY_CHANGE:
          SampleApp_HandleKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD:
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( (SampleApp_NwkState == DEV_ZB_COORD)||
               (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) ) {
            osal_set_event(SampleApp_TaskID, SAMPLEAPP_INITIALIZE_UART_EVT);
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT ) {
    _zigbeeSend(_zigbeeSendBuffer);
    debug(_zigbeeSendBuffer);
    _zigbeeSendCnt ++;
    if (_zigbeeSendCnt < _zigbeeSendTimes) {
        osal_start_timerEx(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
            (SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)));
    } else {
        osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_HEART_BEAT_EVT);
    }
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }
  
  if (events & SAMPLEAPP_INITIALIZE_UART_EVT) {

		// initialize uart_1
		uartConfig.configured           = TRUE;
		uartConfig.baudRate             = MT_UART_DEFAULT_BAUDRATE;
		uartConfig.flowControl          = MT_UART_DEFAULT_OVERFLOW;
		uartConfig.flowControlThreshold = MT_UART_DEFAULT_THRESHOLD;
		uartConfig.rx.maxBufSize        = MT_UART_DEFAULT_MAX_RX_BUFF;
		uartConfig.tx.maxBufSize        = MT_UART_DEFAULT_MAX_TX_BUFF;
		uartConfig.idleTimeout          = MT_UART_DEFAULT_IDLE_TIMEOUT;
		uartConfig.intEnable            = TRUE;
		uartConfig.callBackFunc         = NULL;
		HalUARTOpen(HAL_UART_PORT_1, &uartConfig);
		debug("UART_1 INITIALIZED!\r\n");

		if (isPressed(Key_S1)) {
			debug("Enter AP Mode\r\n");
			osal_set_event(SampleApp_TaskID, SAMPLEAPP_CONFIGURE_WIFI_EVT);
		} else {
			debug("Enter STA Mode\r\n");
			osal_set_event(SampleApp_TaskID, SAMPLEAPP_INITIALIZE_WIFI_EVT);
		}
		return (events ^ SAMPLEAPP_INITIALIZE_UART_EVT);
	}

	if (events & SAMPLEAPP_CONFIGURE_WIFI_EVT) {
		exit_send();
		_UARTRead(HAL_UART_PORT_1, _buffer, &length);
		do debug_and_print("AT+RST\r\n");
		while (wait_for("ready\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CWMODE=2\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CWSAP=\"ESP8266\",\"123456\",11,0\r\n"); // TODO: ADD MACRO
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CIPMODE=0\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CIPMUX=1\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0)); 
		do debug_and_print("AT+CIPSERVER=1,8266\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do {
			while (wait_for("0,CONNECT\r\n", "0,CONNECT FAIL\r\n", 0));
		} while (WiFiSendApp("CTS\r\n")); // ¸æËßAPPÁ¬½ÓÒÑ½¨Á¢£¬¶þ´ÎÎÕÊÖ
		while (1) {
			length = WiFiRecvApp(_buffer);
			if (length > 6) { // min: SSIDx\r\n ÔÊÐí19Î»³¤¶È
				if (osal_memcmp(_buffer, "IP", 2)) {
					nv_id = ZD_NV_IP_ID;
					nv_len = IP_MAX_LENGTH;
					prefix_len = 2;
					WiFiSendApp("GOT IP\r\n");
				} else 
				if (osal_memcmp(_buffer, "PORT", 4)) {
					nv_id = ZD_NV_PORT_ID;
					nv_len = PORT_MAX_LENGTH;
					prefix_len = 4;
					WiFiSendApp("GOT PORT\r\n");
				} else 
				if (osal_memcmp(_buffer, "SSID", 4)) {
					nv_id = ZD_NV_SSID_ID;
					nv_len = SSID_MAX_LENGTH;
					prefix_len = 4;
					WiFiSendApp("GOT SSID\r\n");
				} else 
				if (osal_memcmp(_buffer, "PSWD", 4)) {
					nv_id = ZD_NV_PSWD_ID;
					nv_len = PSWD_MAX_LENGTH;
					prefix_len = 4;
					WiFiSendApp("GOT PSWD\r\n");
				} else continue;
				length -= 2; // \r\n
				while (length < nv_len + prefix_len + 2) _buffer[length ++] = '\0';
				InitNVStatus = osal_nv_item_init(nv_id, nv_len, NULL);
				writeNVStatus = osal_nv_write(nv_id, 0, nv_len, _buffer + prefix_len);
				HalUARTWrite(1, _buffer + prefix_len, nv_len);
				(void) writeNVStatus;
			} else if (length == 4 && osal_memcmp(_buffer, (uint8 *)"OK\r\n", 4)) {
				WiFiSendApp("OVER\r\n");
				_delay_ms(10);
				do debug_and_print("AT+RST\r\n");
				while (wait_for("ready\r\n", "ERROR\r\n", 0));
				break;
			}
		}
		osal_set_event(SampleApp_TaskID, SAMPLEAPP_INITIALIZE_WIFI_EVT);
		return (events ^ SAMPLEAPP_CONFIGURE_WIFI_EVT);
	}

	if (events & SAMPLEAPP_INITIALIZE_WIFI_EVT) {
		// initialize esp8266
		exit_send();
		InitNVStatus = osal_nv_item_init(ZD_NV_SSID_ID, SSID_MAX_LENGTH, NULL);
		readNVStatus = osal_nv_read(ZD_NV_SSID_ID, 0, SSID_MAX_LENGTH, SSID);
		if (readNVStatus != SUCCESS || InitNVStatus != SUCCESS) {
			debug("Read Flash Failed\r\n");
			return (events ^ SAMPLEAPP_INITIALIZE_WIFI_EVT);
		}
		debug(SSID);
		InitNVStatus = osal_nv_item_init(ZD_NV_PSWD_ID, PSWD_MAX_LENGTH, NULL);
		readNVStatus = osal_nv_read(ZD_NV_PSWD_ID, 0, PSWD_MAX_LENGTH, PSWD);
		if (readNVStatus != SUCCESS || InitNVStatus != SUCCESS) {
			debug("Read Flash Failed\r\n");
			return (events ^ SAMPLEAPP_INITIALIZE_WIFI_EVT);
		}
		debug(PSWD);
		InitNVStatus = osal_nv_item_init(ZD_NV_IP_ID, IP_MAX_LENGTH, NULL);
		readNVStatus = osal_nv_read(ZD_NV_IP_ID, 0, IP_MAX_LENGTH, MYIP);
		if (readNVStatus != SUCCESS || InitNVStatus != SUCCESS) {
			debug("Read Flash Failed\r\n");
			return (events ^ SAMPLEAPP_INITIALIZE_WIFI_EVT);
		}
		debug(MYIP);
		InitNVStatus = osal_nv_item_init(ZD_NV_PORT_ID, PORT_MAX_LENGTH, NULL);
		readNVStatus = osal_nv_read(ZD_NV_PORT_ID, 0, PORT_MAX_LENGTH, PORT);
		if (readNVStatus != SUCCESS || InitNVStatus != SUCCESS) {
			debug("Read Flash Failed\r\n");
			return (events ^ SAMPLEAPP_INITIALIZE_WIFI_EVT);
		}
		debug(PORT);

		do debug_and_print("AT+CWMODE=1\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PSWD);
		while (wait_for("OK\r\n", "FAIL\r\n", 0));
		do debug_and_print("AT+CIPMUX=0\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CIPMODE=1\r\n");
		while (wait_for("OK\r\n", "ERROR\r\n", 0));
		do debug_and_print("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", MYIP, PORT);
		while (wait_for("OK\r\n", "CLOSED\r\n", 0));
		
		// drive initial events
		_delay_ms(50);
		osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_HEART_BEAT_EVT);
		return (events ^ SAMPLEAPP_INITIALIZE_WIFI_EVT);
	}

	if (events & SAMPLEAPP_SEND_HEART_BEAT_EVT) {
		_zigbeeSendCnt = 0;
        if (_jumpBackID == 0) {
            length = WiFiRecvServer(_buffer);
    		debug(_buffer);
    		if (osal_memcmp(_buffer, "heartbeat\r\n", 11)) { // 心跳包
    			WiFiSendServer("received");
    		} else
    		if (osal_memcmp(_buffer, "environment\r\n", 13)) { // 获取传感器所有数据
                _jumpBackID = 2;
                _zigbeeRecvFlag = 0;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "E1\r\n\0", 5);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else 
    		// if (osal_memcmp(_buffer, "temperature", 11)) { // 温度
    		// 	_zigbeeSend("temperature\r\n\0");
    		// 	// WiFiSendServer("OK");
    		// } else
    		// if (osal_memcmp(_buffer, "humidity", 8)) { // 湿度
    		// 	_zigbeeSend("humidity\r\n");
    		// 	// WiFiSendServer("OK");
    		// } else
    		if (osal_memcmp(_buffer, "light-on\r\n", 10)) { // 点灯
    			_jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "light-on\r\n\0", 11);
    			osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else
            if (osal_memcmp(_buffer, "light-off\r\n", 11)) { // 点灯
                _jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "light-off\r\n\0", 12);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
            } else
    		if (osal_memcmp(_buffer, "humidify\r\n", 10)) { // 加湿
    			_jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "humidify\r\n\0", 11);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else
    		if (osal_memcmp(_buffer, "stop-humidify\r\n", 15)) { // 停止加湿
    			_jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "stop-humidify\r\n\0", 16);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else
    		if (osal_memcmp(_buffer, "drain-water\r\n", 13)) { // 排水
                _jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "drain-water\r\n\0", 14);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else
    		if (osal_memcmp(_buffer, "change-water\r\n", 14)) { // 换水
    			_jumpBackID = 1;
                _zigbeeSendTimes = 2;
                osal_memcpy(_zigbeeSendBuffer, "change-water\r\n\0", 15);
                osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
    		} else {
                osal_start_timerEx(SampleApp_TaskID, SAMPLEAPP_SEND_HEART_BEAT_EVT, 100);
                return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
            }
        } else if (_jumpBackID == 1) {
            WiFiSendServer("OK");
            _jumpBackID = 0;
            osal_start_timerEx(SampleApp_TaskID, SAMPLEAPP_SEND_HEART_BEAT_EVT, 100);
        } else if (_jumpBackID == 2) {
            cnt = 50;
            while (!_zigbeeRecvFlag) {
                _delay_ms(1);
                if (cnt --) break;
            }
            _zigbeeRecvFlag = 0;
            _zigbeeSendTimes = 2;
            _jumpBackID = 3;
            osal_memcpy(_zigbeeSendBuffer, "E2\r\n\0", 5);
            osal_set_event(SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
        } else if (_jumpBackID == 3) {
            cnt = 50;
            while (!_zigbeeRecvFlag) {
                _delay_ms(1);
                if (cnt --) break;
            }
            _zigbeeRecvFlag = 0;
            _jumpBackID = 0;
            WiFiSendServer("temperature?0=%s,1=%s&humidity?0=%s,1=%s", 
                &temp_humid[0], &temp_humid[6], &temp_humid[3], &temp_humid[9]);
            osal_start_timerEx(SampleApp_TaskID, SAMPLEAPP_SEND_HEART_BEAT_EVT, 100);
        }
		return (events ^ SAMPLEAPP_SEND_HEART_BEAT_EVT);
	}

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      SampleApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_1 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
    SampleApp_SendFlashMessage( SAMPLEAPP_FLASH_DURATION );
  }

  if ( keys & HAL_KEY_SW_2 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1.
     */
    aps_Group_t *grp;
    grp = aps_FindGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    if ( grp )
    {
      // Remove from the group
      aps_RemoveGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    }
    else
    {
      // Add to the flash group
      aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt ) {
    switch (pkt->clusterId) {
    case SAMPLEAPP_FLASH_CLUSTERID:
        if (osal_memcmp(pkt->cmd.Data + 4, "E1", 2)) {
            temp_humid[0] = pkt->cmd.Data[0];
            temp_humid[1] = pkt->cmd.Data[1];
            temp_humid[2] = '\0';
            temp_humid[3] = pkt->cmd.Data[2];
            temp_humid[4] = pkt->cmd.Data[3];
            temp_humid[5] = '\0';
            _zigbeeRecvFlag = 1;
            debug("received from E1\r\n");
        } else
        if (osal_memcmp(pkt->cmd.Data + 4, "E2", 2)) {
            temp_humid[6] = pkt->cmd.Data[0];
            temp_humid[7] = pkt->cmd.Data[1];
            temp_humid[8] = '\0';
            temp_humid[9] = pkt->cmd.Data[2];
            temp_humid[10] = pkt->cmd.Data[3];
            temp_humid[11] = '\0';
            _zigbeeRecvFlag = 1;
            debug("received from E2\r\n");
        }
        break;
  }
}

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage( void )
{
  uint8 data[10]={'0','1','2','3','4','5','6','7','8','9'};//×Ô¶¨ÒåÊý¾Ý
  if ( AF_DataRequest( &SampleApp_Periodic_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_PERIODIC_CLUSTERID,
                       10,//×Ö½ÚÊý
                       data,//Ö¸ÕëÍ·
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      SampleApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void SampleApp_SendFlashMessage( uint16 flashTime )
{
  uint8 buffer[3];
  buffer[0] = (uint8)(SampleAppFlashCounter++);
  buffer[1] = LO_UINT16( flashTime );
  buffer[2] = HI_UINT16( flashTime );

  if ( AF_DataRequest( &SampleApp_Flash_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_FLASH_CLUSTERID,
                       3,
                       buffer,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
*********************************************************************/
void SampleApp_SendGroupMessage( void )
{
  uint8 data[20]="I am Coordinator\r\n\0";
  //uint8 data[20]="I am EndDevice1\r\n\0";
  //uint8 data[20]="I am EndDevice2\r\n\0";
  if ( AF_DataRequest( & SampleApp_Flash_DstAddr,
                       &SampleApp_epDesc,
                       SAMPLEAPP_FLASH_CLUSTERID,
                       20,
                       data,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}


uint8 _zigbeeSend(uint8 *fmt, ...) {
	va_list arg_ptr;
	uint8 _buffer[UartDefaultTxLen], cnt;
	uint16 length = 0;
	for(cnt = 0 ; cnt < UartDefaultTxLen ; cnt++)
		_buffer[cnt] = 0x00;
	va_start(arg_ptr, fmt);
	length = vsprintf((char *)_buffer, (const char *)fmt, arg_ptr);
	va_end(arg_ptr);
	if (AF_DataRequest(&SampleApp_Flash_DstAddr, &SampleApp_epDesc,
		SAMPLEAPP_FLASH_CLUSTERID, 20, _buffer,
		&SampleApp_TransID, AF_DISCV_ROUTE, AF_DEFAULT_RADIUS) == afStatus_SUCCESS) {
		debug("Zigbee Send Succeeded\r\n");
        return 0;
	} else {
        debug("Zigbee Send Failed\r\n");
        return 1;
    }
}

void _UARTSend(uint8 port, uint8 *fmt, ...) {
	va_list arg_ptr;
	uint8 _buffer[UartDefaultTxLen], cnt;
	uint16 length = 0;
	for(cnt = 0 ; cnt < UartDefaultTxLen ; cnt++)
		_buffer[cnt] = 0x00;
	va_start(arg_ptr, fmt);
	length = vsprintf((char *)_buffer, (const char *)fmt, arg_ptr);
	va_end(arg_ptr);
	HalUARTWrite(port, (uint8 *)_buffer, length);
}

void _UARTRead(uint8 port, uint8 *_buffer, uint16 *length) {
	for(*length = 0 ; *length < UartDefaultRxLen; (*length)++)
		_buffer[*length] = 0x00;
	*length = HalUARTRead(port, _buffer, UartDefaultRxLen);
}

void _delay_ms(uint16 timeout) {
	while (timeout --) {
	   _delay_us(1000);
	}
}

void _delay_us(uint16 timeout) {
	uint8 cnt;
	while (timeout --) {
		cnt = 32;
		while (cnt --) {
			asm("NOP");
		}
	}    
}

// 2 for timeout; 1 for error; 0 for clear; timeout = 0 stands for INF
uint8 wait_for(uint8 *str, uint8 *err, uint16 timeout) {
	uint16 wait_len, read_len, err_len;
	int16 i, index;
	uint8 buffer[UartDefaultRxLen], flag;
	wait_len = strlen((char *)str);
	err_len = strlen((char *)err);
	while (1) {
		_UARTRead(HAL_UART_PORT_1, buffer, &read_len);
		_delay_ms(1);
		if (read_len > 2) { // at least 0x0D 0x0A
			index = 0;
			for (i = read_len - 1; i >= 0; i --) {
				if (buffer[i] == 0x0A && i != read_len - 1) {
					index = i + 1;
					break;
				}
			}
			
			if (err_len == (read_len - index)) {
				flag = 1;
				for (i = 0; i < err_len; i ++) {
					if (buffer[index + i] != err[i]) {
						flag = 0;
						break;
					}
				}
				if (flag == 1) {
					debug(err);
					return 1;
				}
			}

			if (wait_len == (read_len - index)) {
				flag = 1;
				for (i = 0; i < wait_len; i ++) {
					if (buffer[index + i] != str[i]) {
						flag = 0;
						break;
					}
				}
				if (flag == 1) {
					debug(str);
					return 0;
				}
			}
		}
		timeout --;
		if (!timeout) return 2;
	}
}

void exit_send() {
	print("+++");
	_delay_ms(10);
	print("+++");
	_delay_ms(10);
	print("\r\n");
	_delay_ms(10);
}


uint8 WiFiSendApp(uint8 *fmt, ...) {
	va_list arg_ptr;
	uint8 buffer[UartDefaultTxLen], cnt;
	uint16 length = 0;
	for(cnt = 0 ; cnt < UartDefaultTxLen ; cnt++)
		buffer[cnt] = 0x00;
	va_start(arg_ptr, fmt);
	length = vsprintf((char *)buffer, (const char *)fmt, arg_ptr);
	va_end(arg_ptr);
	debug_and_print("AT+CIPSEND=0,%d\r\n", length);
	_delay_ms(10);
	// if (wait_for(">", "ERROR\r\n", 0)) return 1;
	HalUARTWrite(1, (uint8 *)buffer, length);
	return wait_for("SEND OK\r\n", "SEND FAIL\r\n", 0);
}

uint16 WiFiRecvApp(uint8 *buff) {
	uint16 read_len, l_index;
	uint8 buffer[UartDefaultRxLen];
	while (1) {
		_UARTRead(HAL_UART_PORT_1, buffer, &read_len);
		_delay_ms(1);
		// at least 11 chars "+IPD,0,X:\r\n"
		if (read_len > 10 
			// && buffer[read_len - 2] == '\r' 
			// && buffer[read_len - 1] == '\n'
		) {
			l_index = 0;
			while (l_index < read_len && buffer[l_index] != ':') {
				l_index ++;
			}
			if (l_index == read_len) continue;
			l_index ++;
			osal_memcpy(buff, buffer + l_index, read_len - l_index);
			return (read_len - l_index);
		}
	}
}

void WiFiSendServer(uint8 *fmt, ...) {
    va_list arg_ptr;
    uint8 buffer[UartDefaultTxLen], cnt;
    uint16 length = 0;
    for(cnt = 0 ; cnt < UartDefaultTxLen ; cnt++)
        buffer[cnt] = 0x00;
    va_start(arg_ptr, fmt);
    length = vsprintf((char *)buffer, (const char *)fmt, arg_ptr);
    va_end(arg_ptr);
    print("AT+CIPSEND\r\n");
    _delay_ms(5);
    HalUARTWrite(1, (uint8 *)buffer, length);
    _delay_ms(5);
    exit_send();
}

uint16 WiFiRecvServer(uint8 *buff) {
    uint8 buffer[UartDefaultRxLen];
    uint16 read_len = 0;
    while (1) {
        _UARTRead(HAL_UART_PORT_1, buffer, &read_len);
        _delay_ms(1);
        if (read_len > 1 && buffer[0] == '#' && buffer[read_len - 1] == '\n') {
            osal_memcpy(buff, buffer + 1, read_len - 1);
            buff[read_len - 1] = '\0';
            return (read_len - 1);
        }
    }
}
