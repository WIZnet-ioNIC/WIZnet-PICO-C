#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket.h"
#include "wizchip_conf.h"
#include "netbios.h"
#include "pico/time.h"

#define NETBIOS_BOARD_NAME     "W55RP20"             /*Define the NetBIOS name*/

#define NETBIOS_PORT           137                  /*The default port for the NetBIOS name service*/

#define NETBIOS_NAME_LEN       16                   /*NetBIOS name maximum length*/

#define NETBIOS_MSG_MAX_LEN    512                  /*The maximum length of a NetBIOS packet*/

#define NETBIOS_NAME_TTL       10                   /*NetBIOS response time*/



/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000U
#define NETB_HFLAG_OPCODE             0x7800U
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000U
#define NETB_HFLAG_AUTHORATIVE        0x0400U
#define NETB_HFLAG_TRUNCATED          0x0200U
#define NETB_HFLAG_RECURS_DESIRED     0x0100U
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080U
#define NETB_HFLAG_BROADCAST          0x0010U
#define NETB_HFLAG_REPLYCODE          0x0008U
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000U

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000U
#define NETB_NFLAG_NODETYPE           0x6000U
#define NETB_NFLAG_NODETYPE_HNODE     0x6000U
#define NETB_NFLAG_NODETYPE_MNODE     0x4000U
#define NETB_NFLAG_NODETYPE_PNODE     0x2000U
#define NETB_NFLAG_NODETYPE_BNODE     0x0000U

/** NetBIOS message header */
#pragma pack(1)
typedef struct _NETBIOS_HDR {
  uint16_t trans_id;
  uint16_t flags;
  uint16_t questions;
  uint16_t answerRRs;
  uint16_t authorityRRs;
  uint16_t additionalRRs;
}NETBIOS_HDR;


/** NetBIOS message name part */
typedef struct _NETBIOS_NAME_HDR {
  uint8_t  nametype;
  uint8_t  encname[(NETBIOS_NAME_LEN*2)+1];
  uint16_t type;
  uint16_t cls;
  uint32_t ttl;
  uint16_t datalen;
  uint16_t flags;
  //ip_addr_p_t addr;
  uint8_t addr[4];
}NETBIOS_NAME_HDR;


/** NetBIOS structure message */
typedef struct _NETBIOS_RESP
{
  NETBIOS_HDR      resp_hdr;
  NETBIOS_NAME_HDR resp_name;
}NETBIOS_RESP;
#pragma pack()

 /**
*@brief NetBIOS decoding
*@param  *name_enc      Decode the name of the former NetBIOS
*@param  *name_dec      The name of the NetBIOS after decoding
*@param  name_dec_len
*@return no
*/
static int netbios_name_decoding( char *name_enc, char *name_dec, int name_dec_len)
{
  char *pname;
  char  cname;
  char  cnbname;
  int   index = 0;
  /* Decode the name of the former NetBIOS */
  pname  = name_enc;
  for (;;) {
    /* Every two characters of the first level-encoded name
     * turn into one character in the decoded name. */
    cname = *pname;
    if (cname == '\0')
      break;    /* no more characters */
    if (cname == '.')
      break;    /* scope ID follows */
    if (cname < 'A' || cname > 'Z') {
      /* Not legal. */
      return -1;
    }
    cname -= 'A';
    cnbname = cname << 4;
    pname++;

    cname = *pname;
    if (cname == '\0' || cname == '.') {
      /* No more characters in the name - but we're in
       * the middle of a pair.  Not legal. */
      return -1;
    }
    if (cname < 'A' || cname > 'Z') {
      /* Not legal. */
      return -1;
    }
    cname -= 'A';
    cnbname |= cname;
    pname++;

    /* Do we have room to store the character? */
    if (index < NETBIOS_NAME_LEN) {
      /* Yes - store the character. */
      name_dec[index++] = (cnbname!=' '?cnbname:'\0');
    }
  }
  return 0;
}
char netbios_rx_buf[NETBIOS_MSG_MAX_LEN];
char netbios_tx_buf[NETBIOS_MSG_MAX_LEN];

uint16_t swaps(uint16_t i)
{
  uint16_t ret=0;
  ret = (i & 0xFF) << 8;
  ret |= ((i >> 8)& 0xFF);
  return ret;  
}
/**
*@brief  Decode the name of the former NetBIOS
*@param  no
*@return no
*/
uint32_t swapl(uint32_t l)
{
  uint32_t ret=0;
  ret = (l & 0xFF) << 24;
  ret |= ((l >> 8) & 0xFF) << 16;
  ret |= ((l >> 16) & 0xFF) << 8;
  ret |= ((l >> 24) & 0xFF);
  return ret;
}
 
uint16_t htons( uint16_t hostshort)
{
  /**< A 16-bit number in host byte order.  */
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
  return swaps(hostshort);
#else
  return hostshort;
#endif    
}

/**
*@brief  Converts an unsigned long data in host mode to a TCP/IP network byte in big-endian mode.
*@param  The data to be transformed
*@return Big-endian schema data
*/ 
unsigned long htonl(unsigned long hostlong)
{
  /**< hostshort  - A 32-bit number in host byte order.  */
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
  return swapl(hostlong);
#else
  return hostlong;
#endif  
}


unsigned long ntohs(unsigned short netshort)
{
  /**< netshort - network odering 16bit value */
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )  
  return htons(netshort);
#else
  return netshort;
#endif    
}


unsigned long ntohl(unsigned long netlong)
{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
  return htonl(netlong);
#else
  return netlong;
#endif    
}


unsigned short checksum(unsigned char * src,unsigned int len)
{
  unsigned int sum, tsum, i, j;
  unsigned long lsum;

  j = len >> 1;

  lsum = 0;

  for (i = 0; i < j; i++) 
  {
    tsum = src[i * 2];
    tsum = tsum << 8;
    tsum += src[i * 2 + 1];
    lsum += tsum;
  }

  if (len % 2) 
  {
    tsum = src[i * 2];
    lsum += (tsum << 8);
  }

  sum = lsum;
  sum = ~(sum + (lsum >> 16));
  return (unsigned short) sum;  
}


 /**
*@brief  Execute the NetBIOS name resolver
*@param  socket number
*@return no
*/
void do_netbios(uint8_t sn)
{
    unsigned char state;
    unsigned int len;
    int ret = 0;

    state = getSn_SR(sn);
    switch(state)
    {
        case SOCK_UDP:
        if((len=getSn_RX_RSR(sn))>0)
        {
            unsigned char rem_ip_addr[4];
            uint16_t rem_udp_port;
            char netbios_name[NETBIOS_NAME_LEN+1];
            NETBIOS_HDR* netbios_hdr;
            NETBIOS_NAME_HDR* netbios_name_hdr;

            len=recvfrom(sn,(unsigned char*)&netbios_rx_buf,len,rem_ip_addr,&rem_udp_port);
            printf("rem_ip_addr=%d.%d.%d.%d:%d\r\n",rem_ip_addr[0],rem_ip_addr[1],rem_ip_addr[2],rem_ip_addr[3],rem_udp_port);
            netbios_hdr = (NETBIOS_HDR*)netbios_rx_buf;
            netbios_name_hdr = (NETBIOS_NAME_HDR*)(netbios_hdr+1);
            /*If the packet is a NetBIOS query packet*/
            if (((netbios_hdr->flags & ntohs(NETB_HFLAG_OPCODE)) == ntohs(NETB_HFLAG_OPCODE_NAME_QUERY)) &&
                ((netbios_hdr->flags & ntohs(NETB_HFLAG_RESPONSE)) == 0) &&
                (netbios_hdr->questions == ntohs(1))) 
            {
            printf("netbios name query question\r\n");
            /* Decode the NetBIOS package */
            netbios_name_decoding( (char*)(netbios_name_hdr->encname), netbios_name, sizeof(netbios_name));
            printf("name is %s\r\n",netbios_name);
            /* If the query is made against the native Netbios */
            if (strcmp(netbios_name, NETBIOS_BOARD_NAME) == 0) 
            {
                printf("\n\n!! name is correct !!\n\n");
                uint8_t ip_addr[4];
                NETBIOS_RESP *resp = (NETBIOS_RESP*)netbios_tx_buf;
                /*Handle the header of the NetBIOS response packet*/
                resp->resp_hdr.trans_id      = netbios_hdr->trans_id;
                resp->resp_hdr.flags         = htons(NETB_HFLAG_RESPONSE |
                                                    NETB_HFLAG_OPCODE_NAME_QUERY |
                                                    NETB_HFLAG_AUTHORATIVE |
                                                    NETB_HFLAG_RECURS_DESIRED);
                resp->resp_hdr.questions     = 0;
                resp->resp_hdr.answerRRs     = htons(1);
                resp->resp_hdr.authorityRRs  = 0;
                resp->resp_hdr.additionalRRs = 0;
                /* Process the header data of the NetBIOS response packet*/
                memcpy( resp->resp_name.encname, netbios_name_hdr->encname, sizeof(netbios_name_hdr->encname));
                resp->resp_name.nametype     = netbios_name_hdr->nametype;
                resp->resp_name.type         = netbios_name_hdr->type;
                resp->resp_name.cls          = netbios_name_hdr->cls;
                resp->resp_name.ttl          = htonl(NETBIOS_NAME_TTL);
                resp->resp_name.datalen      = htons(sizeof(resp->resp_name.flags)+sizeof(resp->resp_name.addr));
                resp->resp_name.flags        = htons(NETB_NFLAG_NODETYPE_BNODE);
                getSIPR(ip_addr);
                memcpy(resp->resp_name.addr,ip_addr,4);    
                /* Send a response packet */
                sendto(sn, (unsigned char*)resp, sizeof(NETBIOS_RESP), rem_ip_addr, rem_udp_port);
                printf("send response\r\n");
            }
            }
        }
        break;
        
        case SOCK_CLOSED:
            close(sn);
            if((ret = socket(sn, Sn_MR_UDP, NETBIOS_PORT, 0x00)) != sn) {
                printf("Socket open error...\n");
            }
            printf("%d:Opened, UDP loopback, port [%d]\r\n", sn, NETBIOS_PORT);
        break;
        
        default:
        break;
    }
}
