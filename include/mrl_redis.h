#ifndef __MRL_REDIS_H_
#define __MRL_REDIS_H_

#include "mrl_main.h"

#define IR_NOMINEE_IP_TABLE_NAME "IR_NOMINEE_IP"
#define IR_CANDIDATE_IP_TABLE_NAME  "IR_CANDIDATE_IP"
//#define REDIS_CANDIATE_KEY_NAME  "IR_CANDIDATE_ID"
struct mrl_ht_nominee_key{
uint32_t sip;
uint32_t dip;
uint16_t sport;
uint16_t dport;
};

struct mrl_nominee_item{
    int config_id;
    int group_id;
    int addr_type;
    char ip_addr[MRL_STR_IP_LEN];
    int is_vaild;
    char op_time[MRL_DATE_LEN];
};

struct mrl_vxlan_info{ 
    int vxlan_link_id;
    int vxlan_encap_type;
    int vxlan_link_dir;
    int vxlan_vpn_id;
    char vxlan_outer_local_port[MRL_STR_PORT_LEN];
    char vxlan_outer_gdev_port[MRL_STR_PORT_LEN];
    char vxlan_outer_local_ip[MRL_STR_IP_LEN];
    char vxlan_outer_gdev_ip[MRL_STR_IP_LEN];//串行设备IP
    char vxlan_outer_local_mac[MRL_STR_MAC_LEN];//接收到的数据包的本地设备MAC地址
    char vxlan_outer_gdev_mac[MRL_STR_MAC_LEN];//接收到的数据包的串行设备MAC地址
    char vxlan_inner_smac[MRL_STR_MAC_LEN];
    char vxlan_inner_dmac[MRL_STR_MAC_LEN];
};
struct mrl_candidate_item{
    int config_id;
    int group_id;
    int addr_type;
    char ip_addr[MRL_STR_IP_LEN];//客户端IP
    int location;
    char mrl_ip[MRL_STR_IP_LEN];
    struct mrl_vxlan_info vxlan_info;
    int is_vaild;
    char op_time[MRL_DATE_LEN];
};

MESA_htable_handle mrl_htable_init(void * fn_data_free_cb);

//void ht_candidate_free_cb(void * data);
void ht_nominee_free_cb(void * data);
void Maat_set_cmd_line(Maat_feather_t feather, struct mrl_candidate_item *candidate_item);
void Maat_set_nominee_cmd_line(Maat_feather_t feather, struct mrl_nominee_item *nominee_item);

Maat_feather_t mrl_Maat_feather_init();
void nominee_update_cb(int table_id,const char* table_line,void* u_para);
bool mrl_identify_nominee(struct streaminfo *mystream);

#endif


