#ifndef __MRL_PACKET_H_
#define __MRL_PACKET_H_

#include "stream.h"
#include "mrl_redis.h"

void mrl_socket_init();
void mrl_send_to_gdev(int thread_seq, struct mrl_vxlan_info * vxlan_info, const char *payload, size_t payload_len);
bool mrl_pkt_signature_identify(struct streaminfo *mystream);
void *mrl_recv_mgw_action(void *arg);
void mrl_send_to_mgw(void *raw_pkt, int thread_seq);


#endif


