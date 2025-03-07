/************************************************************************
** File:
**   $Id: server_msgids.h  $
**
** Purpose:
**  Define SERVER Message IDs
**
*************************************************************************/
#ifndef _SERVER_MSGIDS_H_
#define _SERVER_MSGIDS_H_

/* 
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define SERVER_CMD_MID              0x18F1 /* TODO: Change this for your app */ 

/* 
** This MID is for commands telling the app to publish its telemetry message
*/
#define SERVER_REQ_HK_MID           0x18F2 /* TODO: Change this for your app */

/* 
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define SERVER_HK_TLM_MID           0x08F1 /* TODO: Change this for your app */
#define SERVER_DEVICE_TLM_MID       0x08F2 /* TODO: Change this for your app */

#define CLIENT_PING_SERVER_REQ_MID   0x18F5  // Message ID for PING from CLIENT to SERVER

#endif /* _SERVER_MSGIDS_H_ */
