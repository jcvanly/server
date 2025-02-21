/*******************************************************************************
** File:
**   server_msg.h
**
** Purpose:
**  Define SERVER application commands and telemetry messages
**
*******************************************************************************/
#ifndef _SERVER_MSG_H_
#define _SERVER_MSG_H_

#include "cfe.h"
#include "server_device.h"


/*
** Ground Command Codes
** TODO: Add additional commands required by the specific component
*/
#define SERVER_NOOP_CC                 0
#define SERVER_RESET_COUNTERS_CC       1
#define SERVER_ENABLE_CC               2
#define SERVER_DISABLE_CC              3
#define SERVER_CONFIG_CC               4


/* 
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define SERVER_REQ_HK_TLM              0
#define SERVER_REQ_DATA_TLM            1


/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} SERVER_NoArgs_cmd_t;


/*
** SERVER write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32   DeviceCfg;

} SERVER_Config_cmd_t;


/*
** SERVER device telemetry definition
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    SERVER_Device_Data_tlm_t Server;

} __attribute__((packed)) SERVER_Device_tlm_t;
#define SERVER_DEVICE_TLM_LNGTH sizeof ( SERVER_Device_tlm_t )


/*
** SERVER housekeeping type definition
*/
typedef struct 
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8   CommandErrorCount;
    uint8   CommandCount;
    uint8   DeviceErrorCount;
    uint8   DeviceCount;
  
    /*
    ** TODO: Edit and add specific telemetry values to this struct
    */
    uint8   DeviceEnabled;
    SERVER_Device_HK_tlm_t DeviceHK;

} __attribute__((packed)) SERVER_Hk_tlm_t;
#define SERVER_HK_TLM_LNGTH sizeof ( SERVER_Hk_tlm_t )

#endif /* _SERVER_MSG_H_ */
