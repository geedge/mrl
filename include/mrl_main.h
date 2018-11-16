#ifndef __MRL_MAIN_H_
#define __MRL_MAIN_H_
#include <arpa/inet.h>
#include <sys/types.h>


#include "stream_inc/stream_rawpkt.h"
#include "MESA_prof_load.h"
#include "MESA_handle_logger.h"
#include "MESA_htable.h"
#include "Maat_rule.h"
#include "Maat_command.h"

#include "mrl_utils.h"

struct mrl_global_cfg
{
    //log info
    char mrl_log_path[MRL_MAX_PATH];
    int mrl_log_level;

    //detect info
    char detect_ip[MRL_STR_IP_LEN];
    uint16_t detect_port;
    uint16_t local_port;
    
    //socket info
    char mgw_ip[MRL_STR_IP_LEN];
    char mrl_ip[MRL_STR_IP_LEN];
    uint16_t mgw_port;//mgw listen udp port
    uint16_t mrl_port;//mrl listen udp port

    //maat info
    char Maat_table_path[MRL_MAX_PATH];
    char Maat_redis_ip[MRL_STR_IP_LEN];    
    uint16_t Maat_redis_port;    
    char Maat_stat_path[MRL_MAX_PATH];
    unsigned int Maat_max_threads;

    //htable info
    unsigned int ht_slots;
    unsigned int ht_max_element_num;
    unsigned int ht_mutex_num;
    
    /*vxlan info*/
    unsigned int vxlan_gdev_num;
    char **vxlan_outer_gdev_mac;
    char vxlan_outer_local_mac[MRL_STR_MAC_LEN];
    char **vxlan_outer_gdev_ip;
    char vxlan_outer_local_ip[MRL_STR_IP_LEN];
    char **vxlan_outer_gdev_port;
    char xvlan_outer_local_port[MRL_STR_PORT_LEN];
    char **vxlan_encap_type;
    char **vxlan_link_id;
    char **vxlan_link_dir;
    char **vxlan_inner_smac;
    char **vxlan_inner_dmac;  
    //char vxlan_inner_dip[MRL_STR_IP_LEN];
    //char vxlan_inner_dport[MRL_STR_PORT_LEN];
    //char vxlan_inner_sport[MRL_STR_PORT_LEN];
};

struct mrl_global_instance
{
    void *mrl_log_handle;
    struct mrl_global_cfg mrl_cfg;

    Maat_feather_t nominee_feather;
    Maat_feather_t candidate_feather;

    MESA_htable_handle ht_nominee;
    MESA_htable_handle ht_candidate;

    MMDB_s mrl_mmdb;
    
    int *mrl_snd_fd;
    int mrl_rcv_fd;
    struct sockaddr_in mgw_addr;
};
#endif
