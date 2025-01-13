#ifndef __UPNP_H
#define __UPNP_H

#include <stdint.h>
#include "hyperterminal.h"
#include "socket.h"
signed char SSDPProcess(SOCKET sockfd);			/**< This function processes the SSDP message. */
signed char GetDescriptionProcess(SOCKET sockfd);	/**< This function gets the description message from IGD(Internet Gateway Device). */
signed char SetEventing(SOCKET sockfd);			/**< This function subscribes to the eventing message from IGD(Internet Gateway Device). */
void eventing_listener(SOCKET s);			/**< This function listenes the eventing message from IGD(Internet Gateway Device). */
/**< This function processes the delete port to IGD(Internet Gateway Device). */
signed short DeletePortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port);
/**< This function processes the add port to IGD(Internet Gateway Device). */
signed short AddPortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port, const char* internal_ip, const unsigned int internal_port, const char* description);

signed char parseHTTP(const char* xml);			/**< This function parses the HTTP header. */
signed char parseSSDP(const char* xml);		/**< This function parses the received SSDP message from IGD(Internet Gateway Device). */
signed char parseDescription(const char* xml);		/**< This function parses the received description message from IGD(Internet Gateway Device). */
void parseEventing(const char* xml);			/**< This function parses and prints the received eventing message from IGD(Internet Gateway Device). */
signed short parseError(const char* xml);		/**< This function parses the received UPnP error message from IGD(Internet Gateway Device). */
signed short parseDeletePort(const char* xml);		/**< This function parses the received delete port message from IGD(Internet Gateway Device). */
signed short parseAddPort(const char* xml);		/**< This function parses the received add port message from IGD(Internet Gateway Device). */


unsigned short ATOI(char* str,unsigned short base	);
unsigned long ATOI32(char* str,unsigned short base	);
void itoa2(unsigned short n,unsigned char* str, unsigned char len);
int ValidATOI(char* str, int base,int* ret);
void replacetochar(char * str,	char oldchar,char newchar	);
char C2D(unsigned char c	);
unsigned short swaps(unsigned short i);
unsigned long swapl(unsigned long l);
void mid(char* src, char* s1, char* s2, char* sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);
char* inet_ntoa(unsigned long addr);
unsigned long inet_addr(unsigned char* addr);
char VerifyIPAddress_orig(char* src	);
char VerifyIPAddress(char* src, unsigned char * ip);
unsigned short htons(unsigned short hostshort);
unsigned long htonl(unsigned long hostlong);
unsigned long ntohs(unsigned short netshort);
unsigned long ntohl(unsigned long netlong);
unsigned short checksum(unsigned char * src, unsigned int len);
unsigned char CheckDestInLocal(unsigned long destip);

void data_process_count_handle(void);
#endif



