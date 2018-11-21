#ifndef __MRL_UTILS_H_
#define __MRL_UTILS_H_
#include "maxminddb.h"

#define MRL_STR_MAC_LEN 18
#define MRL_STR_PORT_LEN 6
#define MRL_STR_IP_LEN 16
#define MRL_STR_ENCAP_TYPE_LEN 5
#define MRL_STR_LINK_ID_LEN 5
#define MRL_STR_LINK_DIR_LEN 2
#define MRL_STR_VPN_ID_LEN 5

#define MRL_DATE_LEN 50

#define MRL_MAX_PATH 50
#define MRL_IPV4_TYPE 4

#define MRL_UDP_HDR_LEN 8
#define MRL_IP_HDR_LEN 20

#define MRL_MAC_LEN 6
#define MRL_BUFF_LEN 3000
#define MRL_IP_PKT_LEN 1500

#define MRL_TCP_TYPE 6
#define MRL_UDP_TYPE 17

#define MRL_MAX_STRING 50

#define RLOG_LV_DEBUG       10
#define RLOG_LV_INFO        20
#define RLOG_LV_FATAL       30


unsigned int mrl_split_str(char *str, const char *delim, char **dest);
int mrl_mac_pton(const char *str, int delim, uint8_t *mac);
void get_cur_time(char *time);
unsigned short mrl_get_checksum(unsigned short * buffer, size_t len);

void mrl_mmdb_init(const char *path);
int mrl_search_ip_country(MMDB_s mmdb, const char *ip_address, const char *mycountry);


#endif
