#ifndef __XMODEM_H__
#define __XMODEM_H__
int xmodemReceive(unsigned char *dest, int destsz);
int xmodemTransmit(unsigned char *src, int srcsz);
#endif