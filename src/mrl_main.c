#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "mrl_main.h"
#include "mrl_redis.h"
#include "mrl_packet.h"
#include "mrl_stat.h"

struct mrl_global_instance mrl_instance;
extern struct global_stat_t global_stat;
void mrl_cfg_init(const char *profile)
{
    char *temp_mac_str;
    char *temp_port_str;
    char *temp_ip_str;
    char *temp_encap_type_str;
    char *temp_vpn_id_str;
    char *temp_link_id_str;
    char *temp_link_dir_str;

    unsigned int split_num = 0;
    int temp_port = 0;
    unsigned int i=0;
    
    /**********  load log info  **********/
    MESA_load_profile_int_def(profile, "LOG_INFO", "log_level", &(mrl_instance.mrl_cfg.mrl_log_level),RLOG_LV_DEBUG);
    MESA_load_profile_string_def(profile, "LOG_INFO", "log_path", mrl_instance.mrl_cfg.mrl_log_path, MRL_MAX_PATH, "./log/mrl.log");

    /**********  load detect info  **********/
    MESA_load_profile_string_def(profile, "DETECT_INFO", "dest_ip", mrl_instance.mrl_cfg.dest_ip, MRL_STR_IP_LEN, "0.0.0.0");
    
    MESA_load_profile_int_def(profile, "DETECT_INFO", "dest_port",&(temp_port),8888);
    assert(temp_port > 0 && temp_port <= 65535);
    mrl_instance.mrl_cfg.dest_port = temp_port;

    MESA_load_profile_int_def(profile, "DETECT_INFO", "local_port",&(temp_port),8888);
    assert(temp_port > 0 && temp_port <= 65535);
    mrl_instance.mrl_cfg.local_port = temp_port;
    
    /**********  load sock info  **********/
    MESA_load_profile_int_def(profile, "SOCK_INFO", "mrl_port",&(temp_port),8888);
    assert(temp_port > 0 && temp_port <= 65535);
    mrl_instance.mrl_cfg.mrl_port = temp_port;

    
    MESA_load_profile_int_def(profile, "SOCK_INFO", "mgw_port",&(temp_port),8888);
    assert(temp_port > 0 && temp_port <= 65535);
    mrl_instance.mrl_cfg.mgw_port = temp_port;

    MESA_load_profile_string_def(profile, "SOCK_INFO", "mgw_ip", mrl_instance.mrl_cfg.mgw_ip, MRL_STR_IP_LEN, "0.0.0.0");
    MESA_load_profile_string_def(profile, "SOCK_INFO", "mrl_ip",mrl_instance.mrl_cfg.mrl_ip, MRL_STR_IP_LEN, "0.0.0.0");

    
    /**********  load maat info  **********/
    MESA_load_profile_string_def(profile, "MAAT_INFO", "Maat_table_path", mrl_instance.mrl_cfg.Maat_table_path, MRL_MAX_PATH, "./conf/table_info.conf");    
    MESA_load_profile_uint_def(profile, "MAAT_INFO", "Maat_max_threads", &(mrl_instance.mrl_cfg.Maat_max_threads), 1);   
    MESA_load_profile_string_def(profile, "MAAT_INFO", "Maat_redis_ip", mrl_instance.mrl_cfg.Maat_redis_ip, MRL_STR_IP_LEN, "127.0.0.1");
    MESA_load_profile_int_def(profile, "MAAT_INFO", "Maat_redis_port", &(temp_port), 6379);
    assert(temp_port > 0 && temp_port <= 65535);
    mrl_instance.mrl_cfg.Maat_redis_port = temp_port;
    MESA_load_profile_string_def(profile, "MAAT_INFO", "Maat_stat_path", mrl_instance.mrl_cfg.Maat_stat_path,MRL_MAX_PATH, "./log/Maat_stat.log");   


    /**********  load htable info *********/
    MESA_load_profile_uint_def(profile, "HTABLE_INFO", "ht_slots", &(mrl_instance.mrl_cfg.ht_slots), 1048576);
    MESA_load_profile_uint_def(profile, "HTABLE_INFO", "ht_max_element_num", &(mrl_instance.mrl_cfg.ht_max_element_num), 0);
    MESA_load_profile_uint_def(profile, "HTABLE_INFO", "ht_mutex_num", &(mrl_instance.mrl_cfg.ht_mutex_num), 1);

    
    /**********  load gdev info  **********/
    MESA_load_profile_uint_def(profile, "VXLAN_INFO", "vxlan_gdev_num", &(mrl_instance.mrl_cfg.vxlan_gdev_num), 1);

    temp_mac_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_MAC_LEN,sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_MAC_LEN;

    temp_ip_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_IP_LEN,sizeof(char));
     global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_IP_LEN;
     
    temp_port_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_PORT_LEN, sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_PORT_LEN;
     
    temp_encap_type_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_ENCAP_TYPE_LEN,sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_ENCAP_TYPE_LEN;
     
    temp_vpn_id_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_VPN_ID_LEN,sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_VPN_ID_LEN;

    temp_link_id_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_ID_LEN,sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_ID_LEN;

    temp_link_dir_str = (char *)calloc(mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_DIR_LEN, sizeof(char));
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_DIR_LEN;

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_gdev_mac",temp_mac_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_MAC_LEN, "00:00:00:00:00:00"); 
    mrl_instance.mrl_cfg.vxlan_outer_gdev_mac = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_outer_gdev_mac[i] = (char *)malloc(sizeof(char)*MRL_STR_MAC_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_MAC_LEN;
    
    }
    split_num = mrl_split_str(temp_mac_str,",",mrl_instance.mrl_cfg.vxlan_outer_gdev_mac);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num);
    
    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_gdev_ip",temp_ip_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_IP_LEN, "0.0.0.0"); 
    mrl_instance.mrl_cfg.vxlan_outer_gdev_ip = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[i] = (char *)malloc(sizeof(char)*MRL_STR_IP_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_IP_LEN;
    }
    split_num = mrl_split_str(temp_ip_str,",",mrl_instance.mrl_cfg.vxlan_outer_gdev_ip);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_gdev_port",temp_port_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_PORT_LEN, "8888"); 
    mrl_instance.mrl_cfg.vxlan_outer_gdev_port = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
        
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_outer_gdev_port[i] = (char *)malloc(sizeof(char)*MRL_STR_PORT_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_PORT_LEN;
    }
    split_num = mrl_split_str(temp_port_str,",",mrl_instance.mrl_cfg.vxlan_outer_gdev_port);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 
   
    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_encap_type",temp_encap_type_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_ENCAP_TYPE_LEN, "0000"); 
    mrl_instance.mrl_cfg.vxlan_encap_type = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_encap_type[i] = (char *)malloc(sizeof(char)*MRL_STR_ENCAP_TYPE_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_ENCAP_TYPE_LEN;
    }
    split_num = mrl_split_str(temp_encap_type_str,",",mrl_instance.mrl_cfg.vxlan_encap_type);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 


    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_vpn_id",temp_vpn_id_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_VPN_ID_LEN, "0000"); 
    mrl_instance.mrl_cfg.vxlan_vpn_id = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_vpn_id[i] = (char *)malloc(sizeof(char)*MRL_STR_VPN_ID_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_VPN_ID_LEN;
    } 
    split_num = mrl_split_str(temp_vpn_id_str,",",mrl_instance.mrl_cfg.vxlan_vpn_id);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 


    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_link_id",temp_link_id_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_LINK_ID_LEN, "0"); 
    mrl_instance.mrl_cfg.vxlan_link_id= (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_link_id[i] = (char *)malloc(sizeof(char)*MRL_STR_LINK_ID_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_LINK_ID_LEN;
    }
    split_num = mrl_split_str(temp_link_id_str,",",mrl_instance.mrl_cfg.vxlan_link_id);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_link_dir",temp_link_dir_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_LINK_DIR_LEN, "0"); 
    mrl_instance.mrl_cfg.vxlan_link_dir= (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_link_dir[i] = (char *)malloc(sizeof(char)*MRL_STR_LINK_DIR_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_LINK_DIR_LEN;
    }
    split_num = mrl_split_str(temp_link_dir_str,",",mrl_instance.mrl_cfg.vxlan_link_dir);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_inner_smac",temp_mac_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_MAC_LEN, "00:00:00:00:00:00"); 
    mrl_instance.mrl_cfg.vxlan_inner_smac = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_inner_smac[i] = (char *)malloc(sizeof(char)*MRL_STR_MAC_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_MAC_LEN;
    }
    split_num = mrl_split_str(temp_mac_str,",",mrl_instance.mrl_cfg.vxlan_inner_smac);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_inner_dmac",temp_mac_str, mrl_instance.mrl_cfg.vxlan_gdev_num*MRL_STR_MAC_LEN, "00:00:00:00:00:00"); 
    mrl_instance.mrl_cfg.vxlan_inner_dmac = (char **)malloc(sizeof(char *)*mrl_instance.mrl_cfg.vxlan_gdev_num);
    global_stat.malloc_memory +=mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
    
    for(i=0;i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        mrl_instance.mrl_cfg.vxlan_inner_dmac[i] = (char *)malloc(sizeof(char)*MRL_STR_MAC_LEN);
        global_stat.malloc_memory += sizeof(char)*MRL_STR_MAC_LEN;
    }
    split_num = mrl_split_str(temp_mac_str,",",mrl_instance.mrl_cfg.vxlan_inner_dmac);
    assert(split_num == mrl_instance.mrl_cfg.vxlan_gdev_num); 

    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_local_mac", mrl_instance.mrl_cfg.vxlan_outer_local_mac, MRL_STR_MAC_LEN, "00:00:00:00:00:00");
    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_local_ip", mrl_instance.mrl_cfg.vxlan_outer_local_ip, MRL_STR_IP_LEN, "0.0.0.0");
    MESA_load_profile_string_def(profile, "VXLAN_INFO", "vxlan_local_port", mrl_instance.mrl_cfg.xvlan_outer_local_port,MRL_STR_PORT_LEN, "8888");

    free(temp_mac_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_MAC_LEN;

    free(temp_ip_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_IP_LEN;

    free(temp_port_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_PORT_LEN;

    free(temp_encap_type_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_ENCAP_TYPE_LEN;

    free(temp_link_id_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_ID_LEN;

    free(temp_link_dir_str);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * MRL_STR_LINK_DIR_LEN;
}

extern "C" int mrl_init(void)
{
    int ret = 0;
    mrl_instance.stop_flag = 0;

    mrl_stat_init();
    
    mrl_cfg_init("./mrl_conf/mrl.conf");
    
    mrl_instance.mrl_log_handle = MESA_create_runtime_log_handle(mrl_instance.mrl_cfg.mrl_log_path, mrl_instance.mrl_cfg.mrl_log_level);
    if(mrl_instance.mrl_log_handle == NULL)
    {
        assert(0);
    }

    int thread_safe = 1;
    long max_item_num = 0;
    mrl_instance.mrl_queue = MESA_lqueue_create(thread_safe,max_item_num);
    if(mrl_instance.mrl_queue== NULL)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_init","MESA_lqueue_create func error!");
        assert(0); 
    }

    mrl_instance.ht_nominee = mrl_htable_init((void *)ht_nominee_free_cb);

    mrl_instance.mrl_feather = mrl_Maat_feather_init();
    
    mrl_socket_init();
    //TODO:read from nominee table and candidate table ifno and construct the packet
    //send to the speficied destination

    mrl_mmdb_init("./mrl_conf/all_ip_only_coun_v4.mmdb");
    
    //mrl_nominee_test_init();
    //创建单独线程接收并处理mwg发送的数据包
    pthread_t recv_pid, detect_pid, stat_pid;
    ret = pthread_create(&recv_pid,NULL,mrl_recv_mgw_action,NULL);
    if( ret < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_init","recv pthread create error,error is %d",ret);
        assert(0); 
    }
    pthread_detach(recv_pid);

    ret = pthread_create(&detect_pid,NULL,mrl_detect_ip_action,NULL);
    if( ret < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_init","detect pthread create error,error is %d",ret);
        assert(0); 
    }
    pthread_detach(detect_pid);

    ret = pthread_create(&stat_pid,NULL,mrl_stat_action,NULL);
    if( ret < 0)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_init","stat pthread create error,error is %d",ret);
        assert(0); 
    }
    pthread_detach(stat_pid);
    return 1;
}

void mrl_destroy_cfg()
{
    unsigned int i=0;
    for(i = 0; i<mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        free(mrl_instance.mrl_cfg.vxlan_outer_gdev_mac[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_MAC_LEN;
        
        free(mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_IP_LEN;
        
        free(mrl_instance.mrl_cfg.vxlan_outer_gdev_port[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_PORT_LEN;

        free(mrl_instance.mrl_cfg.vxlan_encap_type[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_ENCAP_TYPE_LEN;
        
        free(mrl_instance.mrl_cfg.vxlan_link_id[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_LINK_ID_LEN;

        free(mrl_instance.mrl_cfg.vxlan_link_dir[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_LINK_DIR_LEN;
        
        free(mrl_instance.mrl_cfg.vxlan_inner_smac[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_MAC_LEN;

        free(mrl_instance.mrl_cfg.vxlan_inner_dmac[i]);
        global_stat.free_memory += sizeof(char)*MRL_STR_MAC_LEN;
    }
    
    free(mrl_instance.mrl_cfg.vxlan_outer_gdev_mac);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
     
    free(mrl_instance.mrl_cfg.vxlan_outer_gdev_ip);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_outer_gdev_port);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_encap_type);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_link_id);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_link_dir);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_inner_smac);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);

    free(mrl_instance.mrl_cfg.vxlan_inner_dmac);
    global_stat.free_memory += mrl_instance.mrl_cfg.vxlan_gdev_num * sizeof(char *);
}


extern "C" void mrl_destroy(void)
{
    MESA_htable_destroy(mrl_instance.ht_nominee,ht_nominee_free_cb);
    MESA_lqueue_destroy(mrl_instance.mrl_queue,NULL,NULL);
    Maat_burn_feather(mrl_instance.mrl_feather);
    mrl_destroy_cfg();
    mrl_socket_close();
    mrl_instance.stop_flag = 1;
    MESA_destroy_runtime_log_handle(mrl_instance.mrl_log_handle);
}

extern "C" char mrl_tcpall_entry(struct streaminfo *a_tcp,void **pme, int thread_seq, void *raw_packet)
{
    global_stat.recv_gdev_total_pkts ++;
    global_stat.recv_gdev_tcp_pkts ++;
	char ret = APP_STATE_GIVEME;
	switch(a_tcp->pktstate){
		case OP_STATE_PENDING:
            if(mrl_pkt_signature_identify(a_tcp))
            {
                global_stat.recv_ir_pkts ++;
                mrl_send_to_mgw(raw_packet, thread_seq);
                ret = APP_STATE_GIVEME;//不指定faw和drop会采取哪个动作？
            }
            else
            { 
                ret = APP_STATE_DROPME|APP_STATE_FAWPKT;//如果当前IP是推荐表中的IP，那么后续数据包不用给我，并且回流转发
            }
		    break;

		case OP_STATE_DATA:
            global_stat.recv_ir_pkts ++;
			mrl_send_to_mgw(raw_packet, thread_seq);
			ret = APP_STATE_GIVEME; 
		    break;

		case OP_STATE_CLOSE:
            if(raw_packet != NULL)
            {
                global_stat.recv_ir_pkts ++;
                mrl_send_to_mgw(raw_packet, thread_seq);
            }
			ret = APP_STATE_DROPME; 
		    break;

		default:
			ret = APP_STATE_DROPME | APP_STATE_FAWPKT;
    		break;
	}
	return ret;
}


extern "C"  char mrl_udp_entry(struct streaminfo *a_udp, void **pme, int thread_seq, void *raw_packet)
{
    global_stat.recv_gdev_total_pkts ++;
    global_stat.recv_gdev_udp_pkts ++;

    char ret = APP_STATE_GIVEME;
	switch(a_udp->pktstate){
		case OP_STATE_PENDING:	
            if(mrl_pkt_signature_identify(a_udp))
            {
                global_stat.recv_ir_pkts ++;
                mrl_send_to_mgw(raw_packet, thread_seq);
                ret = APP_STATE_GIVEME;
            }
            else
            {
                if(mrl_identify_nominee(a_udp))
                {
                     ret =APP_STATE_DROPME;
                }
                else
                {
                    ret =APP_STATE_DROPME|APP_STATE_FAWPKT;
                }
            }
		    break;
		case OP_STATE_DATA:
            global_stat.recv_ir_pkts ++;
			mrl_send_to_mgw(raw_packet, thread_seq);
			ret = APP_STATE_GIVEME;
		    break;

		case OP_STATE_CLOSE:
            if(raw_packet != NULL)
            {
                global_stat.recv_ir_pkts ++;
                mrl_send_to_mgw(raw_packet, thread_seq);
            }
			ret = APP_STATE_DROPME;
		    break;

		default:
			ret = APP_STATE_DROPME | APP_STATE_FAWPKT; 
		    break;
	}
	return ret;
}

