/************************************************************************
** File:
**    server_events.h
**
** Purpose:
**  Define SERVER application event IDs
**
*************************************************************************/

#ifndef _SERVER_EVENTS_H_
#define _SERVER_EVENTS_H_

/* Standard app event IDs */
#define SERVER_RESERVED_EID              0
#define SERVER_STARTUP_INF_EID           1
#define SERVER_LEN_ERR_EID               2
#define SERVER_PIPE_ERR_EID              3
#define SERVER_SUB_CMD_ERR_EID           4
#define SERVER_SUB_REQ_HK_ERR_EID        5
#define SERVER_PROCESS_CMD_ERR_EID       6

/* Standard command event IDs */
#define SERVER_CMD_ERR_EID               10
#define SERVER_CMD_NOOP_INF_EID          11
#define SERVER_CMD_RESET_INF_EID         12
#define SERVER_CMD_ENABLE_INF_EID        13
#define SERVER_ENABLE_INF_EID            14
#define SERVER_ENABLE_ERR_EID            15
#define SERVER_CMD_DISABLE_INF_EID       16
#define SERVER_DISABLE_INF_EID           17
#define SERVER_DISABLE_ERR_EID           18
#define SERVER_CMD_HELLO_WORLD_EID       19


/* Device specific command event IDs */
#define SERVER_CMD_CONFIG_INF_EID        20
#define CLIENT_PING_SERVER_EID           21
#define SERVER_SUB_PING_ERR_EID          22
#define CLIENT_SUB_PING_RESP_ERR_EID     23
#define CLIENT_PING_RESP_EID             24
#define SERVER_SET_SERVER_INT_EID        25  




/* Standard telemetry event IDs */
#define SERVER_DEVICE_TLM_ERR_EID        30
#define SERVER_REQ_HK_ERR_EID            31

/* Device specific telemetry event IDs */
#define SERVER_REQ_DATA_ERR_EID          32

/* Hardware protocol event IDs */
#define SERVER_UART_INIT_ERR_EID         40
#define SERVER_UART_CLOSE_ERR_EID        41

#endif /* _SERVER_EVENTS_H_ */
