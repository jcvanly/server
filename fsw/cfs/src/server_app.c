/*******************************************************************************
** File: server_app.c
**
** Purpose:
**   This file contains the source code for the SERVER application.
**
*******************************************************************************/

/*
** Include Files
*/
#include <arpa/inet.h>
#include "server_app.h"
//#define UNINTENDED_HOST "192.168.0.130"  // Replace with actual IP
#define UNINTENDED_HOST "10.88.207.238"
#define UNINTENDED_PORT 9999  // Port number

/*
** Global Data
*/
SERVER_AppData_t SERVER_AppData;

/*
** Application entry point and main process loop
*/
void SERVER_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SERVER_PERF_ID);

    /* 
    ** Perform application initialization
    */
    status = SERVER_AppInit();
    if (status != CFE_SUCCESS)
    {
        SERVER_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&SERVER_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(SERVER_PERF_ID);

        /* 
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&SERVER_AppData.MsgPtr,  SERVER_AppData.CmdPipe,  CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(SERVER_PERF_ID);

        /*
        ** If the CFE_SB_ReceiveBuffer was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            SERVER_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(SERVER_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: SB Pipe Read Error = %d", (int) status);
            SERVER_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    SERVER_Disable();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(SERVER_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(SERVER_AppData.RunStatus);
} 


/* 
** Initialize application
*/
int32 SERVER_AppInit(void)
{
    int32 status = OS_SUCCESS;
    
    SERVER_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */ 
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);    /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SERVER: Error registering for event services: 0x%08X\n", (unsigned int) status);
       return status;
    }

    /*
    ** Create the Software Bus command pipe 
    */
    status = CFE_SB_CreatePipe(&SERVER_AppData.CmdPipe, SERVER_PIPE_DEPTH, "SERVER_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SERVER_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Creating SB Pipe,RC=0x%08X",(unsigned int) status);
       return status;
    }
    
    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SERVER_CMD_MID), SERVER_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SERVER_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X",
            SERVER_CMD_MID, (unsigned int) status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SERVER_REQ_HK_MID), SERVER_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SERVER_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            SERVER_REQ_HK_MID, (unsigned int) status);
        return status;
    }

    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CLIENT_PING_SERVER_REQ_MID), SERVER_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SERVER_SUB_PING_ERR_EID, CFE_EVS_EventType_ERROR,
            "SERVER: Error subscribing to Client Ping Request, MID=0x%04X, RC=0x%08X",
            CLIENT_PING_SERVER_REQ_MID, (unsigned int)status);
        return status;
    }


    /*
    ** TODO: Subscribe to any other messages here
    */


    /* 
    ** Initialize the published HK message - this HK message will contain the 
    ** telemetry that has been defined in the SERVER_HkTelemetryPkt for this app.
    */
    CFE_MSG_Init(CFE_MSG_PTR(SERVER_AppData.HkTelemetryPkt.TlmHeader),
                   CFE_SB_ValueToMsgId(SERVER_HK_TLM_MID),
                   SERVER_HK_TLM_LNGTH);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    CFE_MSG_Init(CFE_MSG_PTR(SERVER_AppData.DevicePkt.TlmHeader),
                   CFE_SB_ValueToMsgId(SERVER_DEVICE_TLM_MID),
                   SERVER_DEVICE_TLM_LNGTH);

    /*
    ** TODO: Initialize any other messages that this app will publish
    */


    /* 
    ** Always reset all counters during application initialization 
    */
    SERVER_ResetCounters();

    /*
    ** Initialize application data
    ** Note that counters are excluded as they were reset in the previous code block
    */
    SERVER_AppData.HkTelemetryPkt.DeviceEnabled = SERVER_DEVICE_DISABLED;
    SERVER_AppData.HkTelemetryPkt.DeviceHK.DeviceCounter = 0;
    SERVER_AppData.HkTelemetryPkt.DeviceHK.DeviceConfig = 0;
    SERVER_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus = 0;

    /* 
     ** Send an information event that the app has initialized. 
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(SERVER_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
               "SERVER App Initialized. Version %d.%d.%d.%d",
                SERVER_MAJOR_VERSION,
                SERVER_MINOR_VERSION, 
                SERVER_REVISION, 
                SERVER_MISSION_REV);	
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SERVER: Error sending initialization event: 0x%08X\n", (unsigned int) status);
    }
    return status;
} 


/* 
** Process packets received on the SERVER command pipe
*/
void SERVER_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(SERVER_AppData.MsgPtr, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the SERVER_CMD_MID (Message ID)
        */
        case SERVER_CMD_MID:
            SERVER_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        */
        case SERVER_REQ_HK_MID:
            SERVER_ProcessTelemetryRequest();
            break;

        case CLIENT_PING_SERVER_REQ_MID:
            SERVER_HandlePing();
            break;
        

        /*
        ** All other invalid messages that this app doesn't recognize, 
        ** increment the command error counter and log as an error event.  
        */
        default:
            SERVER_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SERVER_PROCESS_CMD_ERR_EID,CFE_EVS_EventType_ERROR, "SERVER: Invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }
    return;
} 


/*
** Process ground commands
** TODO: Add additional commands required by the specific component
*/
void SERVER_ProcessGroundCommand(void)
{
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_MSG_GetMsgId(SERVER_AppData.MsgPtr, &MsgId);

    /*
    ** Ground Commands, by definition, have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    CFE_MSG_GetFcnCode(SERVER_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SERVER_NOOP_CC:
            /*
            ** First, verify the command length immediately after CC identification 
            ** Note that VerifyCmdLength handles the command and command error counters
            */
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                /* Second, send EVS event on successful receipt ground commands*/
                CFE_EVS_SendEvent(SERVER_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: NOOP command received");
                /* Third, do the desired command action if applicable, in the case of NOOP it is no operation */
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SERVER_RESET_COUNTERS_CC:
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SERVER_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: RESET counters command received");
                SERVER_ResetCounters();
            }
            break;

        /*
        ** Enable Command
        */
        case SERVER_ENABLE_CC:
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SERVER_CMD_ENABLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: Enable command received");
                SERVER_Enable();
            }
            break;

        /*
        ** Disable Command
        */
        case SERVER_DISABLE_CC:
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SERVER_CMD_DISABLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: Disable command received");
                SERVER_Disable();
            }
            break;

        case SERVER_HELLO_WORLD_CC:
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SERVER_CMD_HELLO_WORLD_EID, CFE_EVS_EventType_INFORMATION, 
                                "SERVER: Hello World Command Received");

                /* Send Hello World to the UDP listener */
                SERVER_SendHelloWorld();
            }
            break;

        

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        ** Set Configuration Command
        ** Note that this is an example of a command that has additional arguments
        */
        case SERVER_CONFIG_CC:
            if (SERVER_VerifyCmdLength(SERVER_AppData.MsgPtr, sizeof(SERVER_Config_cmd_t)) == OS_SUCCESS)
            {
                uint32_t config = ntohl(((SERVER_Config_cmd_t*) SERVER_AppData.MsgPtr)->DeviceCfg); // command is defined as big-endian... need to convert to host representation
                CFE_EVS_SendEvent(SERVER_CMD_CONFIG_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: Configuration command received: %u", config);
                /* Command device to send HK */
                status = SERVER_CommandDevice(&SERVER_AppData.ServerUart, SERVER_DEVICE_CFG_CMD, config);
                if (status == OS_SUCCESS)
                {
                    SERVER_AppData.HkTelemetryPkt.DeviceCount++;
                }
                else
                {
                    SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
                }
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SERVER_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SERVER_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                "SERVER: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
} 


/*
** Process Telemetry Request - Triggered in response to a telemetery request
** TODO: Add additional telemetry required by the specific component
*/
void SERVER_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_MSG_GetMsgId(SERVER_AppData.MsgPtr, &MsgId);

    /* Pull this command code from the message and then process */
    CFE_MSG_GetFcnCode(SERVER_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        case SERVER_REQ_HK_TLM:
            SERVER_ReportHousekeeping();
            break;

        case SERVER_REQ_DATA_TLM:
            SERVER_ReportDeviceTelemetry();
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SERVER_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SERVER_DEVICE_TLM_ERR_EID, CFE_EVS_EventType_ERROR, 
                "SERVER: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}


/* 
** Report Application Housekeeping
*/
void SERVER_ReportHousekeeping(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (SERVER_AppData.HkTelemetryPkt.DeviceEnabled == SERVER_DEVICE_ENABLED)
    {
        status = SERVER_RequestHK(&SERVER_AppData.ServerUart, (SERVER_Device_HK_tlm_t*) &SERVER_AppData.HkTelemetryPkt.DeviceHK);
        if (status == OS_SUCCESS)
        {
            SERVER_AppData.HkTelemetryPkt.DeviceCount++;
        }
        else
        {
            SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SERVER_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR, 
                    "SERVER: Request device HK reported error %d", status);
        }
    }
    /* Intentionally do not report errors if disabled */

    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *) &SERVER_AppData.HkTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *) &SERVER_AppData.HkTelemetryPkt, true);
    return;
}


/*
** Collect and Report Device Telemetry
*/
void SERVER_ReportDeviceTelemetry(void)
{
    int32 status = OS_SUCCESS;

    if (SERVER_AppData.HkTelemetryPkt.DeviceEnabled == SERVER_DEVICE_ENABLED)
    {
        status = SERVER_RequestData(&SERVER_AppData.ServerUart, &SERVER_AppData.DevicePkt.Server);
        if (status == OS_SUCCESS)
        {
            SERVER_AppData.HkTelemetryPkt.DeviceCount++;
            CFE_SB_TimeStampMsg((CFE_MSG_Message_t *) &SERVER_AppData.DevicePkt);
            CFE_SB_TransmitMsg((CFE_MSG_Message_t *) &SERVER_AppData.DevicePkt, true);
        }
        else
        {
            SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SERVER_REQ_DATA_ERR_EID, CFE_EVS_EventType_ERROR, 
                    "SERVER: Request device data reported error %d", status);
        }
    }
}



/*
** Reset all global counter variables
*/
void SERVER_ResetCounters(void)
{
    SERVER_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    SERVER_AppData.HkTelemetryPkt.CommandCount = 0;
    SERVER_AppData.HkTelemetryPkt.DeviceErrorCount = 0;
    SERVER_AppData.HkTelemetryPkt.DeviceCount = 0;
    return;
} 


/*
** Enable Component
** TODO: Edit for your specific component implementation
*/
void SERVER_Enable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is disabled */
    if (SERVER_AppData.HkTelemetryPkt.DeviceEnabled == SERVER_DEVICE_DISABLED)
    {
        /*
        ** Initialize hardware interface data
        ** TODO: Make specific to your application depending on protocol in use
        ** Note that other components provide examples for the different protocols available
        */ 
        SERVER_AppData.ServerUart.deviceString = SERVER_CFG_STRING;
        SERVER_AppData.ServerUart.handle = SERVER_CFG_HANDLE;
        SERVER_AppData.ServerUart.isOpen = PORT_CLOSED;
        SERVER_AppData.ServerUart.baud = SERVER_CFG_BAUDRATE_HZ;
        SERVER_AppData.ServerUart.access_option = uart_access_flag_RDWR;

        /* Open device specific protocols */
        status = uart_init_port(&SERVER_AppData.ServerUart);
        if (status == OS_SUCCESS)
        {
            SERVER_AppData.HkTelemetryPkt.DeviceCount++;
            SERVER_AppData.HkTelemetryPkt.DeviceEnabled = SERVER_DEVICE_ENABLED;
            CFE_EVS_SendEvent(SERVER_ENABLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: Device enabled");
        }
        else
        {
            SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SERVER_UART_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: UART port initialization error %d", status);
        }
    }
    else
    {
        SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(SERVER_ENABLE_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: Device enable failed, already enabled");
    }
    return;
}


/*
** Disable Component
** TODO: Edit for your specific component implementation
*/
void SERVER_Disable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (SERVER_AppData.HkTelemetryPkt.DeviceEnabled == SERVER_DEVICE_ENABLED)
    {
        /* Open device specific protocols */
        status = uart_close_port(&SERVER_AppData.ServerUart);
        if (status == OS_SUCCESS)
        {
            SERVER_AppData.HkTelemetryPkt.DeviceCount++;
            SERVER_AppData.HkTelemetryPkt.DeviceEnabled = SERVER_DEVICE_DISABLED;
            CFE_EVS_SendEvent(SERVER_DISABLE_INF_EID, CFE_EVS_EventType_INFORMATION, "SERVER: Device disabled");
        }
        else
        {
            SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SERVER_UART_CLOSE_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: UART port close error %d", status);
        }
    }
    else
    {
        SERVER_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(SERVER_DISABLE_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: Device disable failed, already disabled");
    }
    return;
}


/*
** Verify command packet length matches expected
*/
int32 SERVER_VerifyCmdLength(CFE_MSG_Message_t * msg, uint16 expected_length)
{     
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cmd_code = 0;
    size_t actual_length = 0;

    CFE_MSG_GetSize(msg, &actual_length);
    if (expected_length == actual_length)
    {
        /* Increment the command counter upon receipt of an invalid command */
        SERVER_AppData.HkTelemetryPkt.CommandCount++;
    }
    else
    {
        CFE_MSG_GetMsgId(msg, &msg_id);
        CFE_MSG_GetFcnCode(msg, &cmd_code);

        CFE_EVS_SendEvent(SERVER_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %ld, Expected = %d",
              CFE_SB_MsgIdToValue(msg_id), cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command */
        SERVER_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
} 

void SERVER_SendHelloWorld(void) { 
    int sockfd; // Socket file descriptor
    struct sockaddr_in serverAddr; // Server address
    const char *hello_msg = "Hello World"; // Message to send

    /* Create a UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // SOCK_DGRAM for UDP
    if (sockfd < 0) { // Check if socket creation failed
        CFE_EVS_SendEvent(SERVER_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SERVER: Failed to create UDP socket"); // Send error event
        return; // Exit function
    }

    /* Configure server address */
    memset(&serverAddr, 0, sizeof(serverAddr)); // Clear server address
    serverAddr.sin_family = AF_INET; // Set address family to IPv4
    serverAddr.sin_port = htons(UNINTENDED_PORT); // Set port number
    inet_pton(AF_INET, UNINTENDED_HOST, &serverAddr.sin_addr); // Set IP address

    /* Send "Hello World" message */
    sendto(sockfd, hello_msg, strlen(hello_msg), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // Send message
    
    /* Close the socket */
    close(sockfd);
}

void SERVER_HandlePing(void)
{
    CFE_EVS_SendEvent(CLIENT_PING_SERVER_EID, CFE_EVS_EventType_INFORMATION,
                      "SERVER: Received ping from client, sending response.");

    SERVER_SendPingResponse();
}


void SERVER_SendPingResponse(void)
{
    SERVER_PingResponse_t PingResponseMsg;

    /* Initialize the message */
    CFE_MSG_Init(CFE_MSG_PTR(PingResponseMsg.TlmHeader),
                 CFE_SB_ValueToMsgId(SERVER_PING_RESP_MID),
                 sizeof(SERVER_PingResponse_t));

    /* Send the message */
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&PingResponseMsg, true);
}
