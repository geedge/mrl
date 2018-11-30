#ifndef MRL_STAT_H
#define MRL_STAT_H

#include <stdint.h>

#include "MESA/field_stat2.h"

enum FS_FIELD
{
	FIELD_RECV_MGW_PKTS = 0,
    FIELD_SEND_MGW_PKTS,
    FIELD_RECV_GDEV_PKTS,
    FIELD_SEND_GDEV_PKTS,

    FIELD_RECV_GDEV_TCP_PKTS,
    FIELD_RECV_GDEV_UDP_PKTS,
    
    FIELD_RECV_IR_PKTS,
    
    FIELD_RECV_DETECT_PKTS,
    FIELD_SEND_DETECT_PKTS,
    
    FIELD_MAX
};

enum FS_STATUS
{
    STATUS_MALLOC_MEMORY,
    STATUS_FREE_MEMORY,
    STATUS_MAX
};


struct global_stat_t
{
    uint64_t recv_from_mgw_pkts;
    uint64_t send_to_mgw_pkts;


    uint64_t recv_gdev_total_pkts;
    uint64_t send_gdev_total_pkts;

    uint64_t recv_gdev_tcp_pkts;
    uint64_t recv_gdev_udp_pkts;
    
    uint64_t recv_ir_pkts;

    uint64_t recv_detect_pkts;
    uint64_t send_detect_pkts;

    uint64_t malloc_memory;
	uint64_t free_memory;
};

struct fs_stat_t
{
    screen_stat_handle_t handle;
    int fs_field_id[FIELD_MAX];
    int fs_status_id[STATUS_MAX];
};

void mrl_stat_init();

void *mrl_stat_action(void *arg);


#endif // STAT_H_INCLUDED

