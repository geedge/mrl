#include <assert.h>
#include<stdio.h>

#include "mrl_packet.h"
#include "mrl_redis.h"
#include "mrl_stat.h"

extern struct mrl_global_instance mrl_instance;
extern struct global_stat_t global_stat;
extern void mrl_detect_action(const char *ip_addr);

void wrapped_Maat_set_feather_opt(Maat_feather_t feather, enum MAAT_INIT_OPT type, const void* value, int size)
{    
    int ret = Maat_set_feather_opt(feather, type, value, size);
    if(ret < 0)
    {        
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"wrapped_Maat_set_feather_opt","Maat_set_feather_opt func error!");
        assert(0); 
    }
}

void ht_nominee_free_cb(void * data)
{
    struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)data;
    if(nominee_item != NULL)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"ht_nominee_free_cb","the nominee item %s is free.",nominee_item->ip_addr);
        free(nominee_item);
        nominee_item = NULL;
        global_stat.free_memory += sizeof(struct mrl_nominee_item);
    }
        
}

MESA_htable_handle mrl_htable_init(void * fn_data_free_cb)
{
	unsigned int opt_int;
    MESA_htable_handle htable = MESA_htable_born();
    assert(htable != NULL);
    
    opt_int = 1;
    MESA_htable_set_opt(htable, MHO_THREAD_SAFE, &opt_int, sizeof(int));

    opt_int = 1;
    MESA_htable_set_opt(htable, MHO_MUTEX_NUM, &opt_int, sizeof(int));
    
    opt_int = 0;
    MESA_htable_set_opt(htable, MHO_EXPIRE_TIME, &opt_int, sizeof(int));

    opt_int = 0;
    MESA_htable_set_opt(htable, MHO_AUTO_UPDATE_TIME, &opt_int, sizeof(int));

    opt_int = 0;
    MESA_htable_set_opt(htable, MHO_SCREEN_PRINT_CTRL, &opt_int, sizeof(int));

    opt_int = mrl_instance.mrl_cfg.ht_slot_size;
    MESA_htable_set_opt(htable, MHO_HASH_SLOT_SIZE, &opt_int, sizeof(int));
    
    opt_int = mrl_instance.mrl_cfg.ht_max_element_num;
    MESA_htable_set_opt(htable, MHO_HASH_MAX_ELEMENT_NUM, &opt_int, sizeof(int));

    opt_int = mrl_instance.mrl_cfg.ht_mutex_num;
    MESA_htable_set_opt(htable, MHO_MUTEX_NUM, &opt_int, sizeof(int));

    MESA_htable_set_opt(htable, MHO_CBFUN_DATA_FREE, fn_data_free_cb, sizeof(fn_data_free_cb));

    int ret = MESA_htable_mature(htable);
    if (ret < 0)
    {
       MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_init_htable","MESA_htable_mature func error!");
        assert(0);
    }
   return htable;
}

Maat_feather_t Maat_init(const char * instance_name, const char * redis_ip, const short redis_port, const char *stat_path)
{
    // init Maat       
    Maat_feather_t feather = Maat_feather(mrl_instance.mrl_cfg.Maat_max_threads, mrl_instance.mrl_cfg.Maat_table_path, mrl_instance.mrl_log_handle);
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_INSTANCE_NAME, instance_name, strlen(instance_name)+1);  
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_REDIS_IP, redis_ip, strlen(redis_ip)+1);   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_REDIS_PORT, &(redis_port), sizeof(redis_port));   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_STAT_FILE_PATH, stat_path, strlen(stat_path)+1);   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_STAT_ON, NULL, 0);  
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_PERF_ON, NULL, 0); 
    int ret = Maat_initiate_feather(feather);    
    if(ret< 0)    
    {        
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_init","Maat_initiate_feather func error!");
        assert(0); 
    }
    return feather;
}

int Maat_plugin_table(Maat_feather_t feather,const char* table_name,	Maat_start_callback_t *start,Maat_update_callback_t *update,Maat_finish_callback_t *finish,	void *u_para)
{   
    int table_id=0,ret=0;   
    table_id=Maat_table_register(feather,table_name);   
    if(table_id==-1)    
    {       
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_plugin_table","Maat_table_register func error,feather is %d and table name is %s!",feather,table_name);
        assert(0);
    }   
    else    
    {       
        ret=Maat_table_callback_register(feather, table_id, start, update,finish,u_para);        
        if(ret<0)      
        {         
            MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_plugin_table","Maat_table_callback_register func error,ret is %d!",ret);
            assert(0);
        }   
    }  
    return ret;
}
void nominee_update_cb(int table_id,const char *table_line,void *u_para)
{
   int ret = 0;
   uint32_t queue_ip = 0;
   struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)calloc(1, sizeof(struct mrl_nominee_item));
   global_stat.malloc_memory += sizeof(struct mrl_nominee_item);
   sscanf(table_line,"%d\t%d\t%d\t%s\t%d\t%s\t%s",
        &(nominee_item->config_id), &(nominee_item->addr_pool_id), &(nominee_item->addr_type),
        nominee_item->ip_addr, &(nominee_item->is_valid), nominee_item->effective_range, nominee_item->op_time);
   MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","get nominee item:"
                "config_id:%d,addr_pool_id:%d,addr_type:%d,ip_addr:%s,is_valid:%d,effective_range:%s,op_time:%s", 
                nominee_item->config_id, nominee_item->addr_pool_id,nominee_item->addr_type, 
                nominee_item->ip_addr, nominee_item->is_valid,nominee_item->effective_range,nominee_item->op_time);
   struct mrl_ht_nominee_key nominee_key;
   memset(&nominee_key,0,sizeof(struct mrl_ht_nominee_key));
   memcpy(nominee_key.sip,nominee_item->ip_addr,strlen(nominee_item->ip_addr));
   memcpy(nominee_key.dip,mrl_instance.mrl_cfg.dest_ip,strlen(mrl_instance.mrl_cfg.dest_ip));
   nominee_key.sport=mrl_instance.mrl_cfg.local_port;
   nominee_key.dport=mrl_instance.mrl_cfg.dest_port;
    switch(nominee_item->is_valid)
    {
        case 0:
           if(MESA_htable_search_cb(mrl_instance.ht_nominee, (const unsigned char *)&nominee_key, sizeof(nominee_key),NULL,NULL,NULL) != NULL)
           {
               ret = MESA_htable_del(mrl_instance.ht_nominee, (const unsigned char *)&nominee_key, sizeof(nominee_key), NULL);
               if(ret < 0)
               {
                    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"nominee_update_cb","MESA_htable_del func error! ret is %d",ret);
                    assert(0);  
               }
               MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","the nominee key[sip:%s, dip:%s, sport:%hu, dport:%hu] is deleted.",nominee_key.sip,nominee_key.dip,nominee_key.sport,nominee_key.dport);
           }
           else
           {
                MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","the nominee key[sip:%s, dip:%s, sport:%hu, dport:%hu] is not exsit in nominee htable.",nominee_key.sip,nominee_key.dip,nominee_key.sport,nominee_key.dport);
           }
           break;
        case 1:
           if(MESA_htable_search_cb(mrl_instance.ht_nominee, (const unsigned char *)&nominee_key, sizeof(nominee_key),NULL,NULL,NULL) == NULL)
           {
               ret = MESA_htable_add(mrl_instance.ht_nominee, (const unsigned char *)&nominee_key, sizeof(nominee_key), nominee_item);
               if(ret < 0)
               {
                    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"nominee_update_cb","insert the nominee key[sip:%s, dip:%s, sport:%hu, dport:%hu] fail! ret is %d",nominee_key.sip,nominee_key.dip,nominee_key.sport,nominee_key.dport,ret);
                    assert(0);
               }
               MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","insert the nominee key[sip:%s, dip:%s, sport:%hu, dport:%hu] into nominee htable.",nominee_key.sip,nominee_key.dip,nominee_key.sport,nominee_key.dport);
               inet_pton(AF_INET,nominee_key.sip,&queue_ip);
               ret = MESA_lqueue_join_tail(mrl_instance.mrl_queue,&(queue_ip),sizeof(queue_ip));
               MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","insert queue ip is %s",nominee_key.sip);
               if(ret != 0)
               {
                    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"nominee_update_cb","the ip insert into lqueue fail!ret is %d",nominee_key.sip,ret);
                    assert(0);
               }
           }
           else
           {
                MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","the nominee key[sip:%s, dip:%s, sport:%hu, dport:%hu] is duplicated in nominee htable.",nominee_key.sip,nominee_key.dip,nominee_key.sport,nominee_key.dport);
           }
           /*
           inet_pton(AF_INET,nominee_key.sip,&queue_ip);
           printf("insert ip %s into lqueue\n",nominee_key.sip);
           ret = MESA_lqueue_join_tail(mrl_instance.mrl_queue,&(queue_ip),sizeof(queue_ip));
           MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"nominee_update_cb","insert queue ip is %s",nominee_key.sip);
           if(ret != 0)
           {
                MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"nominee_update_cb","the ip insert into lqueue fail!ret is %d",nominee_key.sip,ret);
                assert(0);
           }*/
           break;
        default:
            assert(0);
            break;
    }
    
}


Maat_feather_t mrl_Maat_feather_init()
{
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_Maat_feather_init","start init Maat feather!");
    Maat_feather_t feather;
    const char *nominee_instance = "mrl_nominee";
    const char *candidate_instance = "mrl_candidate";
    // init Maat       
    feather = Maat_feather(mrl_instance.mrl_cfg.Maat_max_threads, mrl_instance.mrl_cfg.Maat_table_path, mrl_instance.mrl_log_handle);
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_INSTANCE_NAME, nominee_instance, strlen(nominee_instance)+1);
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_INSTANCE_NAME, candidate_instance, strlen(candidate_instance)+1);
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_REDIS_IP, mrl_instance.mrl_cfg.Maat_redis_ip, strlen(mrl_instance.mrl_cfg.Maat_redis_ip)+1);   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_REDIS_PORT, &(mrl_instance.mrl_cfg.Maat_redis_port), sizeof(mrl_instance.mrl_cfg.Maat_redis_port));   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_STAT_FILE_PATH, mrl_instance.mrl_cfg.Maat_stat_path, strlen(mrl_instance.mrl_cfg.Maat_stat_path)+1);   
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_STAT_ON, NULL, 0);  
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_PERF_ON, NULL, 0);
    printf("redis index is %d\n",mrl_instance.mrl_cfg.redis_index);
    wrapped_Maat_set_feather_opt(feather, MAAT_OPT_REDIS_INDEX,&(mrl_instance.mrl_cfg.redis_index), sizeof(mrl_instance.mrl_cfg.redis_index));
    int ret = Maat_initiate_feather(feather);    
    if(ret< 0)    
    {        
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_init","Maat_initiate_feather func error!");
        assert(0); 
    }
    Maat_plugin_table(feather,IR_NOMINEE_IP_TABLE_NAME,NULL,nominee_update_cb,NULL,NULL);
    return feather;
}

int mrl_get_candidate_location(const char *ip_addr)
{
    const char *mycountry="China";
    int location = 0;
    location = mrl_search_ip_country(mrl_instance.mrl_mmdb, ip_addr, mycountry);
    if(location < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_get_candidate_location","cur ip addr %s not exist in mmdb",ip_addr);
        return 0;
    }
    return location;
}
void mrl_create_candidate_item(struct mrl_candidate_item *candidate_item, struct mrl_nominee_item *nominee_item)
{
    candidate_item->config_id = nominee_item->config_id;
    candidate_item->addr_pool_id= nominee_item->addr_pool_id;
    candidate_item->addr_type = MRL_IPV4_TYPE ;
    memcpy(candidate_item->ip_addr,nominee_item->ip_addr,strlen(nominee_item->ip_addr));
    candidate_item->location = mrl_get_candidate_location(candidate_item->ip_addr);
    memcpy(candidate_item->mrl_ip, mrl_instance.mrl_cfg.mrl_ip,strlen(mrl_instance.mrl_cfg.mrl_ip));
    candidate_item->is_valid = 1;
    memcpy(candidate_item->effective_range, nominee_item->effective_range, strlen(nominee_item->effective_range));
    get_cur_time(candidate_item->op_time);
}



long ht_search_cb(void *data, const uchar *key, uint size, void *user_arg)
{
    struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)data;
    struct mrl_candidate_item *candidate_item = (struct mrl_candidate_item *)user_arg;
    
    if(nominee_item != NULL)//用户可能配置端口？
    {
        mrl_create_candidate_item(candidate_item,nominee_item);
    }
    return 1;
}

int mrl_get_link_id_index(int gdev_index, int link_id)
{
    int position = -1;
    unsigned int i = 0;
    for(;i < mrl_instance.mrl_cfg.vxlan_link_id_num;i++)
    {
        if(mrl_instance.mrl_cfg.vxlan_link_id[gdev_index][i] == link_id)
        {
            position = i;
            break;
        }
    }
    return position;
}

void mrl_get_vxlan_info(struct streaminfo *mystream, struct mrl_vxlan_info  *vxlan_info, unsigned int gdev_index)
{
    int link_id = 0;
    memcpy(vxlan_info->vxlan_outer_local_mac, mrl_instance.mrl_cfg.vxlan_outer_local_mac, strlen(mrl_instance.mrl_cfg.vxlan_outer_local_mac));
    memcpy(vxlan_info->vxlan_outer_local_ip, mrl_instance.mrl_cfg.vxlan_outer_local_ip, strlen(mrl_instance.mrl_cfg.vxlan_outer_local_ip));
    memcpy(vxlan_info->vxlan_outer_local_port, mrl_instance.mrl_cfg.xvlan_outer_local_port, strlen(mrl_instance.mrl_cfg.xvlan_outer_local_port));

    memcpy(vxlan_info->vxlan_outer_gdev_mac, mrl_instance.mrl_cfg.vxlan_outer_gdev_mac, strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_mac));
    memcpy(vxlan_info->vxlan_outer_gdev_ip,  mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[gdev_index], strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[gdev_index]));
    memcpy(vxlan_info->vxlan_outer_gdev_port, mrl_instance.mrl_cfg.vxlan_outer_gdev_port[gdev_index], strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_port[gdev_index]));
    
    get_rawpkt_opt_from_streaminfo(mystream, RAW_PKT_GET_VXLAN_ID, &link_id);
    vxlan_info->vxlan_link_id = ntohl(link_id);
    printf("cur link id is %d\n",vxlan_info->vxlan_link_id);
    int position = mrl_get_link_id_index(gdev_index, vxlan_info->vxlan_link_id);
    if(position <0)
    {
        char debug_sip[MRL_STR_IP_LEN];
        char debug_dip[MRL_STR_IP_LEN];
        uint16_t debug_sport;
        uint16_t debug_dport;
        mrl_inet_ntoa(mystream->addr.tuple4_v4->saddr,debug_sip);
        mrl_inet_ntoa(mystream->addr.tuple4_v4->daddr,debug_dip);
        debug_sport = ntohs(mystream->addr.tuple4_v4->source);
        debug_dport = ntohs(mystream->addr.tuple4_v4->dest);
        printf("cur stream[sip:%s,sport:%hu,dip:%s,dport:%hu] link id is %d\n",debug_sip,debug_sport,debug_dip,debug_dport,vxlan_info->vxlan_link_id);
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_get_vxlan_info","cur stream[sip:%u,sport:%hu,dip:%u,dport:%hu]"
                                "get link id error,link id is %d,gdev index is %d,position is %d",
                                debug_sip,debug_sport,debug_dip,debug_dport,
                                vxlan_info->vxlan_link_id,gdev_index,position);
        assert(0);
    }
    
    vxlan_info->vxlan_encap_type = 0;
    vxlan_info->vxlan_vpn_id= mrl_instance.mrl_cfg.vxlan_vpn_id;
    vxlan_info->vxlan_link_dir = mrl_instance.mrl_cfg.vxlan_link_dir;
    
    memcpy(vxlan_info->vxlan_inner_smac, mrl_instance.mrl_cfg.vxlan_inner_smac[gdev_index][position],strlen(mrl_instance.mrl_cfg.vxlan_inner_smac[gdev_index][position]));
    memcpy(vxlan_info->vxlan_inner_dmac, mrl_instance.mrl_cfg.vxlan_inner_dmac[gdev_index][position], strlen(mrl_instance.mrl_cfg.vxlan_inner_dmac[gdev_index][position]));
}


void Maat_set_cmd_line(Maat_feather_t feather, struct mrl_candidate_item *candidate_item)
{
    const struct Maat_line_t *p_line;
	struct Maat_line_t line_rule;
	char table_line[512];
	int ret=0;
	memset(&line_rule,0,sizeof(line_rule));
    memset(&table_line,0,sizeof(table_line));
    line_rule.label_id=0;
    line_rule.rule_id=candidate_item->config_id;
    line_rule.table_name=IR_CANDIDATE_IP_TABLE_NAME;
    snprintf(table_line,sizeof(table_line),"%d\t%d\t%d\t%s\t%d\t%s\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t%s",
        candidate_item->config_id, candidate_item->addr_pool_id,candidate_item->addr_type, 
        candidate_item->ip_addr,candidate_item->location, candidate_item->mrl_ip,
        candidate_item->vxlan_info.vxlan_link_id,candidate_item->vxlan_info.vxlan_encap_type,
        candidate_item->vxlan_info.vxlan_link_dir,candidate_item->vxlan_info.vxlan_vpn_id,
        candidate_item->vxlan_info.vxlan_outer_local_port,candidate_item->vxlan_info.vxlan_outer_gdev_port,
        candidate_item->vxlan_info.vxlan_outer_local_ip,candidate_item->vxlan_info.vxlan_outer_gdev_ip,
        candidate_item->vxlan_info.vxlan_outer_local_mac,candidate_item->vxlan_info.vxlan_outer_gdev_mac,
        candidate_item->vxlan_info.vxlan_inner_smac,candidate_item->vxlan_info.vxlan_inner_dmac,
        candidate_item->is_valid, candidate_item->effective_range, candidate_item->op_time);
    line_rule.table_line=table_line;
    line_rule.expire_after=0;
    p_line = &line_rule;
    ret=Maat_cmd_set_lines(feather, &p_line, 1, MAAT_OP_ADD);
    if(ret < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_set_cmd_line","Maat_cmd_set_lines func error! ret is %d",ret);
    }
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"Maat_set_cmd_line","maat set candidate item:"
        "config_id:%d,addr_pool_id:%d,addr_type:%d,ip_addr:%s,location:%d,mrl_ip:%s,link_id:%d,encap_type:%d," 
        "link_dir:%d,vpn_id:%d,local_port:%s,gdev_port:%s, local_ip:%s,gdev_ip:%s,local_mac:%s, gdev_mac:%s,"
        "inner_smac:%s, inner_dmac:%s,is_valid:%d,effective_range:%s,op_time:%s", 
        candidate_item->config_id, candidate_item->addr_pool_id,candidate_item->addr_type, candidate_item->ip_addr, 
        candidate_item->location, candidate_item->mrl_ip,candidate_item->vxlan_info.vxlan_link_id,
        candidate_item->vxlan_info.vxlan_encap_type, candidate_item->vxlan_info.vxlan_link_dir,
        candidate_item->vxlan_info.vxlan_vpn_id,
        candidate_item->vxlan_info.vxlan_outer_local_port,candidate_item->vxlan_info.vxlan_outer_gdev_port,
        candidate_item->vxlan_info.vxlan_outer_local_ip,candidate_item->vxlan_info.vxlan_outer_gdev_ip,
        candidate_item->vxlan_info.vxlan_outer_local_mac,candidate_item->vxlan_info.vxlan_outer_gdev_mac,
        candidate_item->vxlan_info.vxlan_inner_smac, candidate_item->vxlan_info.vxlan_inner_dmac, 
        candidate_item->is_valid,candidate_item->effective_range,candidate_item->op_time);
    return;
}

unsigned int get_gdev_ip_index(UINT32 gdev_ip)
{
    unsigned int index = 0;
    char temp_ip[MRL_STR_IP_LEN];
    memset(temp_ip,0,MRL_STR_IP_LEN);
    inet_ntop(AF_INET, &gdev_ip, temp_ip, MRL_STR_IP_LEN);
    for(; index < mrl_instance.mrl_cfg.vxlan_gdev_num; index++)
    {
        if(memcmp(temp_ip,mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[index],strlen(temp_ip)) == 0)
        {
            break;
        }
    }
    return index;
}


bool mrl_identify_nominee(struct streaminfo *mystream)
{
    char ip_addr[MRL_STR_IP_LEN];
    memset(ip_addr,0,MRL_STR_IP_LEN);
    uint32_t gdev_ip;
    long ret = 0;
    unsigned int index = 0;
    struct mrl_candidate_item candidate_item;
    memset(&candidate_item ,0,sizeof(struct mrl_candidate_item));
    inet_ntop(AF_INET, &(mystream->addr.tuple4_v4->saddr), ip_addr, MRL_STR_IP_LEN);
    struct mrl_ht_nominee_key nominee_key;
    memset(&nominee_key,0,sizeof(nominee_key));
    inet_ntop(AF_INET,(void*)&(mystream->addr.tuple4_v4->saddr),nominee_key.sip,MRL_STR_IP_LEN);
    inet_ntop(AF_INET,(void*)&(mystream->addr.tuple4_v4->daddr),nominee_key.dip,MRL_STR_IP_LEN);
    nominee_key.sport= ntohs(mystream->addr.tuple4_v4->source);
    nominee_key.dport= ntohs(mystream->addr.tuple4_v4->dest);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_identify_nominee","cur stream nominee key is [sip:%s, dip:%s, sport:%hu, dport:%hu]",nominee_key.sip, nominee_key.dip, nominee_key.sport, nominee_key.dport);
    if(MESA_htable_search_cb(mrl_instance.ht_nominee, (const unsigned char *)&nominee_key, sizeof(nominee_key),ht_search_cb,(void *)&candidate_item,&ret) != NULL)
    {
        global_stat.recv_detect_pkts ++;
        get_rawpkt_opt_from_streaminfo(mystream, RAW_PKT_GET_GDEV_IP, &(gdev_ip));
        index = get_gdev_ip_index(gdev_ip);
        assert(index < mrl_instance.mrl_cfg.vxlan_gdev_num);//如果等于就说明该数据包的GDEV不在主动探测的GDEV中，错误
        mrl_get_vxlan_info(mystream, &(candidate_item.vxlan_info),index);              
        Maat_set_cmd_line(mrl_instance.mrl_feather,&candidate_item);
        return true;
    }
    else
    {
        return false;
    }
}

