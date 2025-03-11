/*******************************************************************************
** File: server_device.c
**
** Purpose:
**   This file contains the source code for the SERVER device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "server_device.h"


/* 
** Generic read data from device
*/
int32_t SERVER_ReadData(uart_info_t* device, uint8_t* read_data, uint8_t data_length)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    int32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(device);
    while((bytes_available < data_length) && (ms_timeout_counter < SERVER_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(device);
    }

    if (ms_timeout_counter < SERVER_CFG_MS_TIMEOUT)
    {
        /* Limit bytes available */
        if (bytes_available > data_length)
        {
            bytes_available = data_length;
        }
        
        /* Read data */
        bytes = uart_read_port(device, read_data, bytes_available);
        if (bytes != bytes_available)
        {
            #ifdef SERVER_CFG_DEBUG
                OS_printf("  SERVER_ReadData: Bytes read != to requested! \n");
            #endif
            status = OS_ERROR;
        } /* uart_read */
    }
    else
    {
        status = OS_ERROR;
    } /* ms_timeout_counter */

    return status;
}


/* 
** Generic command to device
** Note that confirming the echoed response is specific to this implementation
*/
int32_t SERVER_CommandDevice(uart_info_t* device, uint8_t cmd_code, uint32_t payload)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    uint8_t write_data[SERVER_DEVICE_CMD_SIZE];
    uint8_t read_data[SERVER_DEVICE_DATA_SIZE];

    /* Prepare command */
    write_data[0] = SERVER_DEVICE_HDR_0;
    write_data[1] = SERVER_DEVICE_HDR_1;
    write_data[2] = cmd_code;
    write_data[3] = payload >> 24;
    write_data[4] = payload >> 16;
    write_data[5] = payload >> 8;
    write_data[6] = payload;
    write_data[7] = SERVER_DEVICE_TRAILER_0;
    write_data[8] = SERVER_DEVICE_TRAILER_1;

    /* Flush any prior data */
    status = uart_flush(device);
    if (status == UART_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(device, write_data, SERVER_DEVICE_CMD_SIZE);
        #ifdef SERVER_CFG_DEBUG
            OS_printf("  SERVER_CommandDevice[%d] = ", bytes);
            for (uint32_t i = 0; i < SERVER_DEVICE_CMD_SIZE; i++)
            {
                OS_printf("%02x", write_data[i]);
            }
            OS_printf("\n");
        #endif
        if (bytes == SERVER_DEVICE_CMD_SIZE)
        {
            status = SERVER_ReadData(device, read_data, SERVER_DEVICE_CMD_SIZE);
            if (status == OS_SUCCESS)
            {
                /* Confirm echoed response */
                bytes = 0;
                while ((bytes < (int32_t) SERVER_DEVICE_CMD_SIZE) && (status == OS_SUCCESS))
                {
                    if (read_data[bytes] != write_data[bytes])
                    {
                        status = OS_ERROR;
                    }
                    bytes++;
                }
            } /* SERVER_ReadData */
            else
            {
                #ifdef SERVER_CFG_DEBUG
                    OS_printf("SERVER_CommandDevice - SERVER_ReadData returned %d \n", status);
                #endif
            }
        } 
        else
        {
            #ifdef SERVER_CFG_DEBUG
                OS_printf("SERVER_CommandDevice - uart_write_port returned %d, expected %d \n", bytes, SERVER_DEVICE_CMD_SIZE);
            #endif
        } /* uart_write */
    } /* uart_flush*/
    return status;
}


/*
** Request housekeeping command
*/
int32_t SERVER_RequestHK(uart_info_t* device, SERVER_Device_HK_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[SERVER_DEVICE_HK_SIZE];

    /* Command device to send HK */
    status = SERVER_CommandDevice(device, SERVER_DEVICE_REQ_HK_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = SERVER_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            #ifdef SERVER_CFG_DEBUG
                OS_printf("  SERVER_RequestHK = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            /* Verify data header and trailer */
            if ((read_data[0]  == SERVER_DEVICE_HDR_0)     && 
                (read_data[1]  == SERVER_DEVICE_HDR_1)     && 
                (read_data[14] == SERVER_DEVICE_TRAILER_0) && 
                (read_data[15] == SERVER_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];

                data->DeviceConfig  = read_data[6] << 24;
                data->DeviceConfig |= read_data[7] << 16;
                data->DeviceConfig |= read_data[8] << 8;
                data->DeviceConfig |= read_data[9];

                data->DeviceStatus  = read_data[10] << 24;
                data->DeviceStatus |= read_data[11] << 16;
                data->DeviceStatus |= read_data[12] << 8;
                data->DeviceStatus |= read_data[13];

                #ifdef SERVER_CFG_DEBUG
                    OS_printf("  Header  = 0x%02x%02x  \n", read_data[0], read_data[1]);
                    OS_printf("  Counter = 0x%08x      \n", data->DeviceCounter);
                    OS_printf("  Config  = 0x%08x      \n", data->DeviceConfig);
                    OS_printf("  Status  = 0x%08x      \n", data->DeviceStatus);
                    OS_printf("  Trailer = 0x%02x%02x  \n", read_data[14], read_data[15]);
                #endif
            }
            else
            {
                #ifdef SERVER_CFG_DEBUG
                    OS_printf("  SERVER_RequestHK: SERVER_ReadData reported error %d \n", status);
                #endif 
                status = OS_ERROR;
            }
        } /* SERVER_ReadData */
    }
    else
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("  SERVER_RequestHK: SERVER_CommandDevice reported error %d \n", status);
        #endif 
    }
    return status;
}


/*
** Request data command
*/
int32_t SERVER_RequestData(uart_info_t* device, SERVER_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[SERVER_DEVICE_DATA_SIZE];

    /* Command device to send data */
    status = SERVER_CommandDevice(device, SERVER_DEVICE_REQ_DATA_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read data */
        status = SERVER_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            /* Verify data header and trailer */
            if ((read_data[0]  == SERVER_DEVICE_HDR_0)     && 
                (read_data[1]  == SERVER_DEVICE_HDR_1)     && 
                (read_data[10] == SERVER_DEVICE_TRAILER_0) && 
                (read_data[11] == SERVER_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];

                data->ServerInt  = read_data[6] << 24; // Takes the byte at index [6] and shifts it 24 bits left to put it in the highest 8 bits.
                data->ServerInt |= read_data[7] << 16; // Takes byte [7] and shifts it 16 bits left, placing it in the second-highest byte.
                data->ServerInt |= read_data[8] << 8; // Takes byte [8] and shifts it 8 bits left, placing it in the third-highest byte.
                data->ServerInt |= read_data[9]; // Takes byte [9] and leaves it as is (since it's already in the least significant position).
            }
        }
    }
    return status;
}
