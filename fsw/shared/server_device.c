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

    #ifdef SERVER_CFG_DEBUG
        OS_printf("SERVER_ReadData: Waiting for %d bytes from UART...\n", data_length);
    #endif

    /* Wait until enough data is available or timeout occurs */
    while ((bytes_available < data_length) && (ms_timeout_counter < SERVER_CFG_MS_TIMEOUT))
    {
        OS_TaskDelay(1);
        ms_timeout_counter++;
        bytes_available = uart_bytes_available(device);
    }

    if (ms_timeout_counter >= SERVER_CFG_MS_TIMEOUT)
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("SERVER_ReadData: Timeout waiting for data (got %d of %d)\n", bytes_available, data_length);
        #endif
        return OS_ERROR;
    }

    /* Limit read size to requested length */
    if (bytes_available > data_length)
    {
        bytes_available = data_length;
    }

    /* Read bytes from UART */
    bytes = uart_read_port(device, read_data, bytes_available);

    #ifdef SERVER_CFG_DEBUG
        OS_printf("SERVER_ReadData: Requested = %d, Available = %d, Read = %d\n", data_length, bytes_available, bytes);
        OS_printf("SERVER_ReadData: Data = ");
        for (int i = 0; i < bytes; ++i)
        {
            OS_printf("%02x ", read_data[i]);
        }
        OS_printf("\n");
    #endif

    if (bytes != bytes_available)
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("SERVER_ReadData: Bytes read (%d) != bytes available (%d)\n", bytes, bytes_available);
        #endif
        status = OS_ERROR;
    }

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

    /* Prepare command packet */
    write_data[0] = SERVER_DEVICE_HDR_0;
    write_data[1] = SERVER_DEVICE_HDR_1;
    write_data[2] = cmd_code;
    write_data[3] = (payload >> 24) & 0xFF;
    write_data[4] = (payload >> 16) & 0xFF;
    write_data[5] = (payload >> 8)  & 0xFF;
    write_data[6] = payload & 0xFF;
    write_data[7] = SERVER_DEVICE_TRAILER_0;
    write_data[8] = SERVER_DEVICE_TRAILER_1;

    #ifdef SERVER_CFG_DEBUG
        OS_printf("SERVER_CommandDevice: Sending command (code 0x%02X, payload 0x%08X)...\n", cmd_code, payload);
        OS_printf("  TX Bytes = ");
        for (uint32_t i = 0; i < SERVER_DEVICE_CMD_SIZE; ++i)
            OS_printf("%02X ", write_data[i]);
        OS_printf("\n");
    #endif

    /* Flush stale data */
    status = uart_flush(device);
    if (status != UART_SUCCESS)
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("SERVER_CommandDevice: uart_flush failed, status = %d\n", status);
        #endif
        return status;
    }

    /* Send command */
    bytes = uart_write_port(device, write_data, SERVER_DEVICE_CMD_SIZE);
    if (bytes != SERVER_DEVICE_CMD_SIZE)
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("SERVER_CommandDevice: uart_write_port wrote %d bytes, expected %d\n", bytes, SERVER_DEVICE_CMD_SIZE);
        #endif
        return OS_ERROR;
    }

    /* Read and verify echo */
    status = SERVER_ReadData(device, read_data, SERVER_DEVICE_CMD_SIZE);
    if (status != OS_SUCCESS)
    {
        #ifdef SERVER_CFG_DEBUG
            OS_printf("SERVER_CommandDevice: SERVER_ReadData failed, status = %d\n", status);
        #endif
        return status;
    }

    #ifdef SERVER_CFG_DEBUG
        OS_printf("  RX Echo   = ");
        for (int i = 0; i < SERVER_DEVICE_CMD_SIZE; ++i)
            OS_printf("%02X ", read_data[i]);
        OS_printf("\n");
    #endif

    /* Confirm echoed response matches sent data */
    for (int i = 0; i < SERVER_DEVICE_CMD_SIZE; ++i)
    {
        if (read_data[i] != write_data[i])
        {
            #ifdef SERVER_CFG_DEBUG
                OS_printf("SERVER_CommandDevice: Mismatch at byte %d (TX: %02X, RX: %02X)\n",
                          i, write_data[i], read_data[i]);
            #endif
            return OS_ERROR;
        }
    }

    return OS_SUCCESS;
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

                data->ServerInt      = read_data[6] << 24;
                data->ServerInt     |= read_data[7] << 16;
                data->ServerInt     |= read_data[8] << 8;
                data->ServerInt     |= read_data[9];
            }
        }
    }
    return status;
}
