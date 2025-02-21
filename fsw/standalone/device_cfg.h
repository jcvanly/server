#ifndef _SERVER_CHECKOUT_DEVICE_CFG_H_
#define _SERVER_CHECKOUT_DEVICE_CFG_H_

/*
** SERVER Checkout Configuration
*/
#define SERVER_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define SERVER_CFG_STRING           "/dev/usart_16"
#define SERVER_CFG_HANDLE           16 
#define SERVER_CFG_BAUDRATE_HZ      115200
#define SERVER_CFG_MS_TIMEOUT       250
#define SERVER_CFG_DEBUG

#endif /* _SERVER_CHECKOUT_DEVICE_CFG_H_ */
