/*******************************************************************************
** File: server_app.h
**
** Purpose:
**   This is the main header file for the SERVER application.
**
*******************************************************************************/
#ifndef _SERVER_APP_H_
#define _SERVER_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "server_device.h"
#include "server_events.h"
#include "server_platform_cfg.h"
#include "server_perfids.h"
#include "server_msg.h"
#include "server_msgids.h"
#include "server_version.h"
#include "hwlib.h"


/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define SERVER_PIPE_DEPTH            32


/*
** Enabled and Disabled Definitions
*/
#define SERVER_DEVICE_DISABLED       0
#define SERVER_DEVICE_ENABLED        1


/*
** SERVER global data structure
** The cFE convention is to put all global app data in a single struct. 
** This struct is defined in the `server_app.h` file with one global instance 
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    SERVER_Hk_tlm_t   HkTelemetryPkt;   /* SERVER Housekeeping Telemetry Packet */
    
    /*
    ** Operational data  - not reported in housekeeping
    */
    CFE_MSG_Message_t * MsgPtr;             /* Pointer to msg received on software bus */
    CFE_SB_PipeId_t CmdPipe;            /* Pipe Id for HK command pipe */
    uint32 RunStatus;                   /* App run status for controlling the application state */

    /*
	** Device data 
    ** TODO: Make specific to your application
	*/
    SERVER_Device_tlm_t DevicePkt;      /* Device specific data packet */
    uint32_t ServerInt;
    uint8_t toggle;
    /* 
    ** Device protocol
    ** TODO: Make specific to your application
    */ 
    uart_info_t ServerUart;             /* Hardware protocol definition */

} SERVER_AppData_t;


/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern SERVER_AppData_t SERVER_AppData; /* SERVER App Data */


/*
**
** Local function prototypes.
**
** Note: Except for the entry point (SERVER_AppMain), these
**       functions are not called from any other source module.
*/
void  SERVER_AppMain(void);
int32 SERVER_AppInit(void);
void  SERVER_ProcessCommandPacket(void);
void  SERVER_ProcessGroundCommand(void);
void  SERVER_ProcessTelemetryRequest(void);
void  SERVER_ReportHousekeeping(void);
void  SERVER_ReportDeviceTelemetry(void);
void  SERVER_ResetCounters(void);
void  SERVER_Enable(void);
void  SERVER_Disable(void);
int32 SERVER_VerifyCmdLength(CFE_MSG_Message_t * msg, uint16 expected_length);
void SERVER_SendToUnintendedHost(void *data, size_t len);
void SERVER_SendHelloWorld(void);
void SERVER_HandlePing(void);
void SERVER_SendPingResponse(void);
void SERVER_ForwardToListener(const void *data, size_t length);
void SERVER_HandleToggleExfil(void);
#endif /* _SERVER_APP_H_ */
