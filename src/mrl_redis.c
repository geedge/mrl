#include <assert.h>
#include<stdio.h>

#include "mrl_redis.h"
#include "mrl_utils.h"
#include "mrl_packet.h"

#define MRL_IPV4_TYPE 4

extern struct mrl_global_instance mrl_instance;

void wrapped_Maat_set_feather_opt(Maat_feather_t feather, enum MAAT_INIT_OPT type, const void* value, int size)
{    
    int ret = Maat_set_feather_opt(feather, type, value, size);
    if((ret < 0))
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
        free(nominee_item);
        nominee_item = NULL;
    }
}

void ht_candidate_free_cb(void * data)
{
    struct mrl_candidate_item *candidate_item = (struct mrl_candidate_item *)data;
    if(candidate_item != NULL)
    {
        free(candidate_item);
        candidate_item = NULL;
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

    opt_int = mrl_instance.mrl_cfg.ht_slots;
    MESA_htable_set_opt(htable, MHO_HASH_SLOT_SIZE, &opt_int, sizeof(int));
    
    opt_int = mrl_instance.mrl_cfg.ht_max_element_num + 1;
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


Maat_feather_t mrl_Maat_feather_init(const char * instance_name,const char *table_name, Maat_start_callback_t *start,Maat_update_callback_t *update,Maat_finish_callback_t *finish,void *u_para)
{
    Maat_feather_t feather;
    feather = Maat_init(instance_name,mrl_instance.mrl_cfg.Maat_redis_ip,mrl_instance.mrl_cfg.Maat_redis_port,mrl_instance.mrl_cfg.Maat_stat_path);
    Maat_plugin_table(feather,table_name,start,update,finish,u_para);
    return feather;
}


void mrl_create_candidate_item(struct mrl_candidate_item *candidate_item, struct mrl_nominee_item *nominee_item)
{
    candidate_item->config_id = nominee_item->config_id;
    candidate_item->group_id = nominee_item->group_id;
    candidate_item->addr_type = MRL_IPV4_TYPE ;
    memcpy(candidate_item->ip_addr,nominee_item->ip_addr,strlen(nominee_item->ip_addr));
     //to do how to get ip location
    candidate_item->location = 0;
    memcpy(candidate_item->mrl_ip, mrl_instance.mrl_cfg.vxlan_outer_local_ip,strlen(mrl_instance.mrl_cfg.vxlan_outer_local_ip));
    candidate_item->is_vaild = 1;
    get_cur_time(candidate_item->op_time);
}

void nominee_update_cb(int table_id,const char* table_line,void* u_para)
{
   int ret = 0;
   struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)calloc(1, sizeof(struct mrl_nominee_item));
   sscanf(table_line,"%d\t%d\t%d\t%s\t%d\t%s",
        &(nominee_item->config_id), &(nominee_item->group_id), &(nominee_item->addr_type),
        nominee_item->ip_addr, &(nominee_item->is_vaild), nominee_item->op_time);
    switch(nominee_item->is_vaild)
    {
        case 0:
           ret = MESA_htable_del(mrl_instance.ht_nominee, (const unsigned char *)nominee_item->ip_addr, sizeof(nominee_item->ip_addr), NULL);
           assert(ret >= 0);
           if(MESA_htable_search_cb(mrl_instance.ht_candidate, (const unsigned char *)&(nominee_item->ip_addr), sizeof(nominee_item->ip_addr),NULL,NULL,NULL) != NULL)
           {
                ret = MESA_htable_del(mrl_instance.ht_candidate, (const unsigned char *)nominee_item->ip_addr, sizeof(nominee_item->ip_addr), NULL);
                assert(ret >= 0);
           }
           break;
        case 1:
            ret = MESA_htable_add(mrl_instance.ht_nominee, (const unsigned char *)nominee_item->ip_addr, sizeof(nominee_item->ip_addr), nominee_item);
            assert(ret >= 0);
            //todo active detect
            detect_ip_addr(nominee_item->ip_addr);
            break;
        default:
            assert(0);
            break;
    }
    
}


void candidate_update_cb(int table_id,const char* table_line,void* u_para)
{
    int ret = 0;
    struct mrl_candidate_item *candidate_item = (struct mrl_candidate_item *)calloc(1, sizeof(struct mrl_candidate_item));
    sscanf(table_line,"%d\t%d\t%d\t%s\t%d\t%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%s",
        &(candidate_item->config_id), &(candidate_item->group_id),&(candidate_item->addr_type), 
        candidate_item->ip_addr,&(candidate_item->location),candidate_item->mrl_ip,
        &(candidate_item->vxlan_info.vxlan_link_id),  &(candidate_item->vxlan_info.vxlan_encap_type),
        &(candidate_item->vxlan_info.vxlan_link_dir),candidate_item->vxlan_info.vxlan_outer_local_port,
        candidate_item->vxlan_info.vxlan_outer_gdev_port,candidate_item->vxlan_info.vxlan_outer_local_ip,
        candidate_item->vxlan_info.vxlan_outer_gdev_ip,candidate_item->vxlan_info.vxlan_outer_local_mac,
        candidate_item->vxlan_info.vxlan_outer_gdev_mac,candidate_item->vxlan_info.vxlan_inner_smac,
        candidate_item->vxlan_info.vxlan_inner_dmac,&(candidate_item->is_vaild), candidate_item->op_time);
    ret = MESA_htable_add(mrl_instance.ht_candidate, (const unsigned char *)(candidate_item->ip_addr), sizeof(candidate_item->ip_addr), candidate_item);
    assert(ret >= 0);
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


void mrl_get_candidate_vxlan_info(struct streaminfo *mystream, struct mrl_vxlan_info  *vxlan)
{
    int gdev_ip = 0;
    //get_rawpkt_opt_from_streaminfo(mystream, RAW_PKT_GET_GDEV_IP, &(gdev_ip));
    inet_ntop(AF_INET, &gdev_ip, vxlan->vxlan_outer_gdev_ip, MRL_STR_IP_LEN);
    unsigned int index = 0;
    for(; index < mrl_instance.mrl_cfg.vxlan_gdev_num; index++)
    {
        if(memcmp(vxlan->vxlan_outer_gdev_ip,mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[index],strlen(vxlan->vxlan_outer_gdev_ip)) == 0)
        {
            break;
        }
    }
    assert(index < mrl_instance.mrl_cfg.vxlan_gdev_num);//如果等于就说明该数据包的GDEV不在主动探测的GDEV中，错误
    memcpy(vxlan->vxlan_inner_smac, mrl_instance.mrl_cfg.vxlan_inner_smac[index],strlen(mrl_instance.mrl_cfg.vxlan_inner_smac[index]));
    memcpy(vxlan->vxlan_inner_dmac, mrl_instance.mrl_cfg.vxlan_inner_dmac[index], strlen(mrl_instance.mrl_cfg.vxlan_inner_dmac[index]));
    vxlan->vxlan_encap_type = atoi(mrl_instance.mrl_cfg.vxlan_encap_type[index]);
    vxlan->vxlan_link_id = atoi(mrl_instance.mrl_cfg.vxlan_link_id[index]);
    vxlan->vxlan_link_dir = atoi(mrl_instance.mrl_cfg.vxlan_link_dir[index]);
    memcpy(vxlan->vxlan_outer_local_port, mrl_instance.mrl_cfg.xvlan_outer_local_port,strlen(mrl_instance.mrl_cfg.xvlan_outer_local_port));
    memcpy(vxlan->vxlan_outer_gdev_port , mrl_instance.mrl_cfg.vxlan_outer_gdev_port[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_port[index]));
    memcpy(vxlan->vxlan_outer_local_ip, mrl_instance.mrl_cfg.vxlan_outer_local_ip,strlen(mrl_instance.mrl_cfg.vxlan_outer_local_ip));
    memcpy(vxlan->vxlan_outer_gdev_ip,  mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[index]));
    memcpy(vxlan->vxlan_outer_local_mac, mrl_instance.mrl_cfg.vxlan_outer_local_mac,strlen(mrl_instance.mrl_cfg.vxlan_outer_local_mac));
    memcpy(vxlan->vxlan_outer_gdev_mac, mrl_instance.mrl_cfg.vxlan_outer_gdev_mac[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_mac[index]));
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_get_candidate_vxlan_info","get vxlan opt from rawpkt:"
                "inner_smac:%s, inner_dmac:%s,encap_type:%d,link_id:%d, link_dir:%d,local_port:%u,"
                "gdev_port:%u, local_ip:%s,gdev_ip:%s,local_mac:%s, gdev_mac:%s", 
                vxlan->vxlan_inner_smac, vxlan->vxlan_inner_dmac, vxlan->vxlan_encap_type,
                vxlan->vxlan_link_id, vxlan->vxlan_link_dir,vxlan->vxlan_outer_local_port,
                vxlan->vxlan_outer_gdev_port,vxlan->vxlan_outer_local_ip,vxlan->vxlan_outer_gdev_ip,
                vxlan->vxlan_outer_local_mac,vxlan->vxlan_outer_gdev_mac);

}


void Maat_set_cmd_line(Maat_feather_t feather, struct mrl_candidate_item *candidate_item)
{
    const struct Maat_line_t *p_line;
	struct Maat_line_t line_rule;
	char table_line[128];
	int ret=0;
	memset(&line_rule,0,sizeof(line_rule));
    line_rule.label_id=0;
    line_rule.rule_id=candidate_item->config_id;
    line_rule.table_name=IR_CANDIDATE_IP_TABLE_NAME;
    snprintf(table_line,sizeof(table_line),"%d\t%d\t%d\t%s\t%d\t%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%s",
        candidate_item->config_id, candidate_item->group_id,candidate_item->addr_type, 
        candidate_item->ip_addr,candidate_item->location, candidate_item->mrl_ip,
        candidate_item->vxlan_info.vxlan_link_id,candidate_item->vxlan_info.vxlan_encap_type,
        candidate_item->vxlan_info.vxlan_link_dir,candidate_item->vxlan_info.vxlan_outer_local_port,
        candidate_item->vxlan_info.vxlan_outer_gdev_port,candidate_item->vxlan_info.vxlan_outer_local_ip,
        candidate_item->vxlan_info.vxlan_outer_gdev_ip,candidate_item->vxlan_info.vxlan_outer_local_mac,
        candidate_item->vxlan_info.vxlan_outer_gdev_mac,candidate_item->vxlan_info.vxlan_inner_smac,
        candidate_item->vxlan_info.vxlan_inner_dmac,candidate_item->is_vaild, candidate_item->op_time);
    line_rule.table_line=table_line;
    line_rule.expire_after=0;
    p_line = &line_rule;
    ret=Maat_cmd_set_lines(feather, &p_line, 1, MAAT_OP_ADD);
    if(ret < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_set_cmd_line","Maat_cmd_set_lines func error! ret is %d",ret);
        assert(ret>0);
    }
    
    return;
}


void mrl_identify_nominee(struct streaminfo *mystream)
{
    UINT32 dip;
    struct mrl_candidate_item candidate_item;
    memset(&candidate_item ,0,sizeof(struct mrl_candidate_item));
    dip = ntohl(mystream->addr.tuple4_v4->daddr);
    if(MESA_htable_search_cb(mrl_instance.ht_nominee, (const unsigned char *)&(dip), sizeof(dip),ht_search_cb,(void *)&candidate_item,NULL) != NULL)
    {
        mrl_get_candidate_vxlan_info(mystream,&(candidate_item.vxlan_info));
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_identify_nominee","get candidate item:"
        "config_id:%d,group_id:%d,addr_type:%d,ip_addr:%s,location:%d,mrl_ip:%s,is_vaild:%d,op_time:%s", 
                      candidate_item.config_id, candidate_item.group_id,candidate_item.addr_type, candidate_item.ip_addr, 
                      candidate_item.location, candidate_item.mrl_ip,candidate_item.is_vaild,candidate_item.op_time);              
        Maat_set_cmd_line(mrl_instance.candidate_feather,&candidate_item);
    }
}




void Maat_set_nominee_cmd_line(Maat_feather_t feather, struct mrl_nominee_item *nominee_item)
{
    const struct Maat_line_t *p_line;
	struct Maat_line_t line_rule;
	char table_line[128];
	int ret=0,i=0;
	memset(&line_rule,0,sizeof(line_rule));
    line_rule.label_id=0;
    line_rule.rule_id=nominee_item->config_id;
    line_rule.table_name=IR_NOMINEE_IP_TABLE_NAME;
    snprintf(table_line,sizeof(table_line[i]),"%d\t%d\t%d\t%s\t%d\t%s",
        nominee_item->config_id, nominee_item->group_id,nominee_item->addr_type, 
        nominee_item->ip_addr,nominee_item->is_vaild, nominee_item->op_time);
    line_rule.table_line=table_line;
    line_rule.expire_after=0;
    p_line = &line_rule;
    ret=Maat_cmd_set_lines(feather, &p_line, 1, MAAT_OP_ADD);
    assert(ret>0);
    return;
}

