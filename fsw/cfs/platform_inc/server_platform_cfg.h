/************************************************************************
** File:
**   $Id: server_platform_cfg.h  $
**
** Purpose:
**  Define server Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _SERVER_PLATFORM_CFG_H_
#define _SERVER_PLATFORM_CFG_H_

/*
** Default SERVER Configuration
*/
#ifndef SERVER_CFG
    /* Notes: 
    **   NOS3 uart requires matching handle and bus number
    */
    #define SERVER_CFG_STRING           "usart_28"
    #define SERVER_CFG_HANDLE           28
    #define SERVER_CFG_BAUDRATE_HZ      115200
    #define SERVER_CFG_MS_TIMEOUT       50            /* Max 255 */
    /* Note: Debug flag disabled (commented out) by default */
    #define SERVER_CFG_DEBUG
#endif

#endif /* _SERVER_PLATFORM_CFG_H_ */
