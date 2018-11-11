#define IR_NOMINEE_IP_TABLE_NAME "IR_NOMINEE_IP"
#define IR_CANDIDATE_IP_TABLE_NAME  "IR_CANDINATE_IP"
#define REDIS_CANDIATE_KEY_NAME  "IR_CANDIDATE_ID"
struct mrl_nominee_item{
    int region_id;
    int group_id;
    int addr_type;
    char src_ip[MRL_STR_IP_LEN];
    char mask_src_ip[MRL_STR_IP_LEN];
    char dport[MRL_STR_PORT_LEN];
    char mask_dport[MRL_STR_PORT_LEN];
    int protocol;
    int direction;
    int is_vaild;
    int action;
    int service;
    char date[DATE_LEN];
    int type;
    int strategy_group_id;
};

struct mrl_vxlan_info{
    char vxlan_outer_local_mac[MRL_STR_MAC_LEN];//接收到的数据包的本地设备MAC地址
    char vxlan_outer_gdev_mac[MRL_STR_MAC_LEN];//接收到的数据包的串行设备MAC地址
    char vxlan_outer_local_ip[MRL_STR_IP_LEN];
    char vxlan_outer_gdev_ip[MRL_STR_IP_LEN];//串行设备IP
    char vxlan_outer_gdev_port[MRL_STR_PORT_LEN];
    char vxlan_outer_local_port[MRL_STR_PORT_LEN];
    unsigned char vxlan_encap_type;
    int vxlan_link_id;
    unsigned char vxlan_link_dir;
    char vxlan_inner_smac[MRL_STR_MAC_LEN];
    char vxlan_inner_dmac[MRL_STR_MAC_LEN];
};

struct mrl_candidate_item{
    int candidate_id;
    int addr_type;
    char client_ip[MRL_STR_IP_LEN];//客户端IP
    int type;
    int location;
    int group_id;
    char mrl_ip[MRL_STR_IP_LEN];
    struct mrl_vxlan_info vxlan_info;
};

MESA_htable_handle mrl_htable_init()
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

    opt_int = mrl_instance.ht_slots;
    MESA_htable_set_opt(htable, MHO_HASH_SLOT_SIZE, &opt_int, sizeof(int));
    
    opt_int = mrl_instance.ht_max_sessions + 1;
    MESA_htable_set_opt(htable, MHO_HASH_MAX_ELEMENT_NUM, &opt_int, sizeof(int));

    opt_int = mrl_instance.ht_mutex_num;
    MESA_htable_set_opt(htable, MHO_MUTEX_NUM, &opt_int, sizeof(int));

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
    
    Maat_feather_t Maat_feather = Maat_feather(mrl_instance.mrl_cfg.Maat_max_threads, mrl_instance.mrl_cfg.Maat_table_path, mrl_instance.mrl_log_handle);    
    if(Maat_feather == NULL)   
    {   
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_init","Maat_feather func error!");
        assert(0);
    }   
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_INSTANCE_NAME, instance_name, strlen(instance_name)+1);  
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_REDIS_IP, redis_ip, strlen(redis_ip)+1);   
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_REDIS_PORT, &(redis_port), sizeof(redis_port));   
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_STAT_FILE_PATH, stat_path, strlen(stat_path)+1);   
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_STAT_ON, NULL, 0);  
    wrapped_Maat_set_feather_opt(Maat_feather, MAAT_OPT_PERF_ON, NULL, 0); 
    int ret = Maat_initiate_feather(Maat_feather);    
    if(unlikely(ret< 0))    
    {        
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_init","Maat_initiate_feather func error!");
        assert(0); 
    }
    return Maat_feather;
}

int Maat_plugin_table(Maat_feather_t feather,const char* table_name,	Maat_start_callback_t *start,Maat_update_callback_t *update,Maat_finish_callback_t *finish,	void *u_para)
{   
    int table_id=0,ret=0;   
    table_id=Maat_table_register(feather,table_name);   
    if(unlikely(table_id==-1))    
    {       
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"Maat_plugin_table","Maat_table_register func error,feather is %d and table name is %s!",feather,table_name);
        assert(0);
    }   
    else    
    {       
        ret=Maat_table_callback_register(feather, table_id, start, update,finish,u_para);        
        if(unlikely(ret<0))      
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


void nominee_update_cb(int table_id,const char* table_line,void* u_para)
{
    
   struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)calloc(1, sizeof(struct mrl_nominee_item));
   sscanf(table_line,"%d\t%d\t%d\t%s\t%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d",
        nominee_item->region_id, nominee_item->group_id, nominee_item->addr_type,
        nominee_item->src_ip, nominee_item->mask_src_ip, nominee_item->dport,
        nominee_item->mask_dport, nominee_item->protocol, nominee_item->direction,
        nominee_item->is_vaild, nominee_item->action, nominee_item->service,
        nominee_item->date, nominee_item->type, nominee_item->strategy_group_id);
    
    int ret =MESA_htable_add(mrl_instance.ht_nominee, (const unsigned char *)nominee_item->src_ip, sizeof(nominee_item->src_ip), nominee_item);
    assert(ret >= 0);
}


void candidate_update_cb(int table_id,const char* table_line,void* u_para)
{
    struct mrl_candidate_item *candidate_item = (struct mrl_candidate_item *)calloc(1, sizeof(struct mrl_candidate_item));
    sscanf(table_line,"%d\t%d\t%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%d\t%s\t%s",
        candidate_item->candidate_id, candidate_item->addr_type, candidate_item->client_ip,
        candidate_item->type, candidate_item->location, candidate_item->group_id,
        candidate_item->mrl_ip,candidate_item->vxlan_info.vxlan_outer_local_mac,
        candidate_item->vxlan_info.vxlan_outer_gdev_mac,candidate_item->vxlan_info.vxlan_outer_local_ip,
        candidate_item->vxlan_info.vxlan_outer_gdev_ip,candidate_item->vxlan_info.vxlan_outer_local_port,
        candidate_item->vxlan_info.vxlan_outer_gdev_port,candidate_item->vxlan_info.vxlan_encap_type,
        candidate_item->vxlan_info.vxlan_link_id,candidate_item->vxlan_info.vxlan_link_dir,
        candidate_item->vxlan_info.vxlan_inner_smac,candidate_item->vxlan_info.vxlan_inner_dmac);
    
    int ret = MESA_htable_add(mrl_instance.ht_candidate, (const unsigned char *)candidate_item->client_ip, sizeof(candidate_item->client_ip), candidate_item);
    assert(ret >= 0);
}

long ht_search_cb(void *data, const uchar *key, uint size, void *user_arg)
{
    struct mrl_nominee_item *nominee_item = (struct mrl_nominee_item *)data;
    struct streaminfo *mystrem = (struct streaminfo *)user_arg;
    if(nominee_item != NULL)//用户可能配置端口？
    {
        mrl_create_candidate_item(mystrem,nominee_item);
    }
    return 1;
}

void mrl_identify_nominee(struct streaminfo *mystrem)
{
    UINT32 dip;
    bool ret = false;
    dip = ntohl(mystrem->addr.tuple4_v4->daddr);
    MESA_htable_search_cb(mrl_instance.ht_nominee, (const unsigned char *)&(dip), sizeof(dip),ht_search_cb,(void *)mystrem,NULL);
}


void Maat_set_cmd_line(Maat_feather_t feather, struct mrl_candidate_item *candidate_item)
{
	struct Maat_line_t line_rule;
	char table_line[128];
	int ret=0,i=0;
	memset(&line_rule,0,sizeof(line_rule));
    line_rule.label_id=0;
    line_rule.rule_id=(int)Maat_cmd_incrby(feather,"TEST_PLUG_SEQ", 1);
    line_rule.table_name=IR_CANDIDATE_IP_TABLE_NAME;
    snprintf(table_line,sizeof(table_line[i]),"%d\t%d\t%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%d\t%s\t%s",
        candidate_item->candidate_id, candidate_item->addr_type, candidate_item->client_ip,
        candidate_item->type, candidate_item->location, candidate_item->group_id,
        candidate_item->mrl_ip,candidate_item->vxlan_info.vxlan_outer_local_mac,
        candidate_item->vxlan_info.vxlan_outer_gdev_mac,candidate_item->vxlan_info.vxlan_outer_local_ip,
        candidate_item->vxlan_info.vxlan_outer_gdev_ip,candidate_item->vxlan_info.vxlan_outer_local_port,
        candidate_item->vxlan_info.vxlan_outer_gdev_port,candidate_item->vxlan_info.vxlan_encap_type,
        candidate_item->vxlan_info.vxlan_link_id,candidate_item->vxlan_info.vxlan_link_dir,
        candidate_item->vxlan_info.vxlan_inner_smac,candidate_item->vxlan_info.vxlan_inner_dmac);
    line_rule.table_line=table_line;
    line_rule.expire_after=0;
    ret=Maat_cmd_set_lines(feather, line_rule, 1, MAAT_OP_ADD);
    assert(ret>0);
    return;
}

void mrl_create_candidate_item(struct streaminfo *mystream, struct mrl_nominee_item *nominee_item)
{
    int table_id,ret;
    struct mrl_candidate_item candidate_ip_item;
    memset(&candidate_ip_item ,0,sizeof(struct mrl_candidate_item));
    candidate_ip_item.candidate_id = (int)Maat_cmd_incrby(mrl_instance.candiate_feather,REDIS_CANDIATE_KEY_NAME, 1);;
    candidate_ip_item.addr_type = MRL_IPV4_TYPE ;
    memcpy(candidate_ip_item.client_ip,nominee_item->src_ip,strlen(nominee_item->src_ip));
    candidate_ip_item.type = nominee_item->type;
     //to do how to get ip location
    candidate_ip_item.location = 0;
   
    candidate_ip_item.group_id = nominee_item->group_id;
    memcpy(candidate_ip_item.mrl_ip, mrl_instance.mrl_cfg.local_ip,strlen(mrl_instance.mrl_cfg.local_ip));
    mrl_get_candidate_vxlan_info(mystream,&(candidate_ip_item.vxlan));
   
   MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_create_candidate_item","get candidate item:
                   candidate_id:%d,addr_type:%d,client_ip:%s,type:%d, location:%d, 
                   group_id:%d,mrl_ip:%s", 
                  candidate_ip_item.candidate_id, candidate_ip_item.addr_type, candidate_ip_item.client_ip, candidate_ip_item.type,
                  candidate_ip_item.location, candidate_ip_item.group_id, candidate_ip_item.mrl_ip);              
    Maat_set_cmd_line(mrl_instance.candiate_feather,&mrl_candidate_item);
    
}



