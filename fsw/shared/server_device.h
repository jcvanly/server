/*******************************************************************************
** File: server_device.h
**
** Purpose:
**   This is the header file for the SERVER device.
**
*******************************************************************************/
#ifndef _SERVER_DEVICE_H_
#define _SERVER_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"
#include "server_platform_cfg.h"


/*
** Type definitions
** TODO: Make specific to your application
*/
#define SERVER_DEVICE_HDR              0xDEAD
#define SERVER_DEVICE_HDR_0            0xDE
#define SERVER_DEVICE_HDR_1            0xAD

#define SERVER_DEVICE_NOOP_CMD         0x00
#define SERVER_DEVICE_REQ_HK_CMD       0x01
#define SERVER_DEVICE_REQ_DATA_CMD     0x02
#define SERVER_DEVICE_CFG_CMD          0x03
#define SERVER_DEVICE_SET_INT_CMD      0x04  // New command for setting ServerInt


#define SERVER_DEVICE_TRAILER          0xBEEF
#define SERVER_DEVICE_TRAILER_0        0xBE
#define SERVER_DEVICE_TRAILER_1        0xEF

#define SERVER_DEVICE_HDR_TRL_LEN      4
#define SERVER_DEVICE_CMD_SIZE         9

/*
** SERVER device housekeeping telemetry definition
*/
typedef struct
{
    uint32_t  DeviceCounter;
    uint32_t  DeviceConfig;
    uint32_t  DeviceStatus;

} __attribute__((packed)) SERVER_Device_HK_tlm_t;
#define SERVER_DEVICE_HK_LNGTH sizeof ( SERVER_Device_HK_tlm_t )
#define SERVER_DEVICE_HK_SIZE SERVER_DEVICE_HK_LNGTH + SERVER_DEVICE_HDR_TRL_LEN


/*
** SERVER device data telemetry definition
*/
typedef struct
{
    uint32_t  DeviceCounter;
    uint32_t  ServerInt;  // New field replacing X, Y, Z

} __attribute__((packed)) SERVER_Device_Data_tlm_t;

#define SERVER_DEVICE_DATA_LNGTH sizeof ( SERVER_Device_Data_tlm_t )
#define SERVER_DEVICE_DATA_SIZE SERVER_DEVICE_DATA_LNGTH + SERVER_DEVICE_HDR_TRL_LEN


/*
** Prototypes
*/
int32_t SERVER_ReadData(uart_info_t* device, uint8_t* read_data, uint8_t data_length);
int32_t SERVER_CommandDevice(uart_info_t* device, uint8_t cmd, uint32_t payload);
int32_t SERVER_RequestHK(uart_info_t* device, SERVER_Device_HK_tlm_t* data);
int32_t SERVER_RequestData(uart_info_t* device, SERVER_Device_Data_tlm_t* data);


#endif /* _SERVER_DEVICE_H_ */
