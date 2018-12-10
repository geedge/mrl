#include <arpa/inet.h>
#include <sys/types.h>
#include <assert.h>
#include<unistd.h>
#include <stdio.h>

#include "stream.h"
#include "stream_inc/stream_control.h"
#include "mrtunnat.h"

#include "mrl_packet.h"
#include "mrl_main.h"
#include "mrl_utils.h"
#include "mrl_redis.h"
#include "mrl_stat.h"

extern struct global_stat_t global_stat;

extern struct mrl_global_instance mrl_instance;

void mrl_socket_init()
{
    int i;
    int thread_num = 0;
    int len = sizeof(thread_num);
    sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
    mrl_instance.mrl_snd_fd = (int *) malloc (sizeof(int)*thread_num);
    global_stat.malloc_memory += sizeof(int)*thread_num;
    
    for(i = 0; i< thread_num;i++)
    {
        mrl_instance.mrl_snd_fd[i] = socket(AF_INET, SOCK_DGRAM, 0);   
    }
       
    //init destnation sock addr
    mrl_instance.mgw_addr.sin_family = AF_INET;
    inet_pton(AF_INET, mrl_instance.mrl_cfg.mgw_ip, &(mrl_instance.mgw_addr.sin_addr));
    mrl_instance.mgw_addr.sin_port = htons(mrl_instance.mrl_cfg.mgw_port);

    //init recv fd
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(mrl_instance.mrl_cfg.mrl_port);
    mrl_instance.mrl_rcv_fd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(mrl_instance.mrl_rcv_fd , (struct sockaddr *)(&addr), sizeof(addr));
   
    
}

void mrl_socket_close()
{
    int i;
    int thread_num = 0;
    int len = sizeof(thread_num);
    sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
    for(i = 0; i< thread_num;i++)
    {
      close(mrl_instance.mrl_snd_fd[i]);
    }

    free(mrl_instance.mrl_snd_fd);
    global_stat.free_memory += sizeof(int)*thread_num;
    
    close(mrl_instance.mrl_rcv_fd);
}

ssize_t mrl_sock_send(int fd, const char *pkt, ssize_t pkt_len, struct sockaddr_in *dest)
{
    ssize_t send_len = 0;
    while(1)
    {
        send_len = sendto(fd, pkt,pkt_len,0,(struct sockaddr *)dest,sizeof(struct sockaddr_in));
        if(send_len < 0)
        {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                assert(0);
            }
            
        }
        else
        {
            break;
        }
    }
    return send_len;
}

ssize_t mrl_sock_recv(int fd, char *recv_buff, size_t buff_len)
{
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t recv_len = 0;
    while(1)
    {
         recv_len = recvfrom(fd, recv_buff, buff_len, 0, (struct sockaddr *) &from, & from_len);
         if(recv_len < 0)
         {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                assert(0);
            }
            
         }
         else
         {
            break;
         }
    }
    return recv_len ;
}


void mrl_send_to_gdev(int thread_seq, struct mrl_vxlan_info * vxlan_info, const char *payload, size_t payload_len)
{ 
    SAPP_TLV_T option[32];
    int option_num = 0;

    uint8_t temp_mac[MRL_MAC_LEN];
    uint32_t temp_ip;
    uint16_t temp_port;
    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_outer_local_mac, ':', temp_mac);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_SMAC: %02x-%02x-%02x-%02x-%02x-%02x",
                            temp_mac[0],temp_mac[1],temp_mac[2],temp_mac[3],temp_mac[4],temp_mac[5]);
    option[option_num].type = SAPP_SEND_OPT_GDEV_SMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;////////

    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_outer_gdev_mac, ':', temp_mac);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_DMAC: %02x-%02x-%02x-%02x-%02x-%02x",
                            temp_mac[0],temp_mac[1],temp_mac[2],temp_mac[3],temp_mac[4],temp_mac[5]);
    option[option_num].type = SAPP_SEND_OPT_GDEV_DMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////

    inet_pton(AF_INET, vxlan_info->vxlan_outer_local_ip, &temp_ip);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_SIP: %s",vxlan_info->vxlan_outer_local_ip);
    option[option_num].type = SAPP_SEND_OPT_GDEV_SIP;
    option[option_num].length = 4;
    option[option_num].int_value = temp_ip;
    option_num++;///////


    inet_pton(AF_INET, vxlan_info->vxlan_outer_gdev_ip, &temp_ip); 
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_DIP: %s",vxlan_info->vxlan_outer_gdev_ip);
    option[option_num].type = SAPP_SEND_OPT_GDEV_DIP;
    option[option_num].length = 4;
    option[option_num].int_value = temp_ip;
    option_num++;///////


    temp_port = atoi(vxlan_info->vxlan_outer_local_port);
    assert(temp_port >0 && temp_port <= 65535);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_UDP_SPORT: %hu",temp_port);
    option[option_num].type = SAPP_SEND_OPT_GDEV_UDP_SPORT;
    option[option_num].length = 2;
    option[option_num].short_value = htons(temp_port);
    option_num++;///////


    temp_port = atoi(vxlan_info->vxlan_outer_gdev_port);
    assert(temp_port >0 && temp_port <= 65535);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_GDEV_UDP_DPORT: %hu",temp_port);
    option[option_num].type = SAPP_SEND_OPT_GDEV_UDP_DPORT;
    option[option_num].length = 2;
    option[option_num].short_value = htons(temp_port);
    option_num++;///////
    
    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_VPN_ID;
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_VXLAN_VPN_ID: %d",vxlan_info->vxlan_vpn_id);
    option[option_num].length = 4;
    option[option_num].int_value = vxlan_info->vxlan_vpn_id;
    option_num++;///////

    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_VXLAN_LINK_ID: %d",vxlan_info->vxlan_link_id);
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_ID;
    option[option_num].length = 4;
    option[option_num].int_value = vxlan_info->vxlan_link_id;
    option_num++;///////

    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_VXLAN_LINK_ENCAP_TYPE: %d",TUNNAT_TUNNEL_TYPE_ETHER);
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = TUNNAT_TUNNEL_TYPE_ETHER;
    option_num++;///////

    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_VXLAN_LINK_DIR: %d",vxlan_info->vxlan_link_dir);
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_DIR;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info->vxlan_link_dir;
    option_num++;///////

    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_INNER_LINK_ENCAP_TYPE: %d",TUNNAT_TUNNEL_TYPE_ETHER);
    option[option_num].type = SAPP_SEND_OPT_INNER_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = TUNNAT_TUNNEL_TYPE_ETHER;
    option_num++;///////


    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_inner_smac, ':', temp_mac);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_INNER_SMAC: %02x-%02x-%02x-%02x-%02x-%02x",
                            temp_mac[0],temp_mac[1],temp_mac[2],temp_mac[3],temp_mac[4],temp_mac[5]);
    option[option_num].type = SAPP_SEND_OPT_INNER_SMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////


    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_inner_dmac, ':', temp_mac);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_gdev","SAPP_SEND_OPT_INNER_DMAC: %02x-%02x-%02x-%02x-%02x-%02x",
                            temp_mac[0],temp_mac[1],temp_mac[2],temp_mac[3],temp_mac[4],temp_mac[5]);
    option[option_num].type = SAPP_SEND_OPT_INNER_DMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////
    
    int ret = 0;
    ret = MESA_sendpacket_iplayer_options(thread_seq,  
                         payload,
                         payload_len,
                         vxlan_info->vxlan_link_dir,
                         option,
                         option_num);
    assert(ret >=0);
}


//判断是否是IP复用包
bool mrl_pkt_signature_identify(struct streaminfo *mystream)
{
    uint8_t type = 0;
    char debug_sip[MRL_STR_IP_LEN];
    char debug_dip[MRL_STR_IP_LEN];
    uint16_t  sport = 0, dport = 0, temp_port = 0;
    uint32_t hash_sport = 0, hash_dport =0;
    uint32_t sip = 0, dip = 0;
    sport = ntohs(mystream->addr.tuple4_v4->source);
    dport = ntohs(mystream->addr.tuple4_v4->dest);
    sip = ntohl(mystream->addr.tuple4_v4->saddr);
    dip = ntohl(mystream->addr.tuple4_v4->daddr);
    switch(mystream->type)
    {
        case STREAM_TYPE_TCP:
            type = MRL_TCP_TYPE;
            break;
        case STREAM_TYPE_UDP:
            type = MRL_UDP_TYPE;
            break;
        default:
            assert(0);
            break;
    }
    hash_dport = sport ^ sip ^ dip ^type;
    hash_dport = hash_dport & 0xff;
    temp_port = dport & 0xff;
    
    mrl_inet_ntoa(sip,debug_sip);
    mrl_inet_ntoa(dip,debug_dip);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_pkt_signature_identify","cur stream[sip:%s,sport:%hu, dip:%s,dport:%hu]"
                            "hash info is:[hash_dport:%hu, ir_port:%u,protocol is %d",
                            debug_sip,sport,debug_dip,dport,hash_dport,temp_port,type);
    if(hash_dport == temp_port)
    {
        return true;
    }
    else
    {
        hash_sport = dport ^ sip ^ dip ^type;
        hash_sport = hash_sport & 0xff;
        temp_port = sport & 0xff;
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_pkt_signature_identify","cur stream[sip:%s,sport:%hu, dip:%s,dport:%hu]"
                            "hash info is:[hash_sport:%hu, ir_port:%u,protocol is %d",
                            debug_sip,sport,debug_dip,dport,hash_dport,temp_port,type);
        if(hash_sport == temp_port)
        {
            return true;
        }
        return false;
    }
}


void mrl_send_to_mgw(void *raw_pkt, int thread_seq)
{
    const char *pkt = (const char *)raw_pkt;
    struct iphdr *ip_hdr = (struct iphdr *)pkt;
    ssize_t pkt_len = ntohs(ip_hdr->tot_len);
    ssize_t send_len = 0;
    send_len = mrl_sock_send(mrl_instance.mrl_snd_fd[thread_seq],pkt,pkt_len,&(mrl_instance.mgw_addr));
    global_stat.send_to_mgw_pkts += 1;
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_send_to_mgw","pkt len is %d and send to mgw len is %d",pkt_len,send_len);
    assert(send_len == pkt_len);
    
}


void *mrl_recv_mgw_action(void *arg)
{
    
    char rcv_buff[MRL_BUFF_LEN];
    ssize_t recv_len = 0;
    int thread_seq = 0;
    int temp_len = 0;
    while(1)
    {
        if(mrl_instance.stop_flag == 1)
        {
            break;
        }
        memset(rcv_buff,0,MRL_BUFF_LEN);
        recv_len = mrl_sock_recv(mrl_instance.mrl_rcv_fd,rcv_buff, MRL_BUFF_LEN);
        global_stat.recv_from_mgw_pkts ++;
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_recv_mgw_action","recv from mgw len is %ld",recv_len);
        struct mrl_vxlan_info *vxlan_info = (struct mrl_vxlan_info *)rcv_buff;
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_recv_mgw_action","recv mgw vxlan info:"
                    "link_id:%d,encap_type:%d,link_dir:%d,vpn_id:%d,local_port:%s,gdev_port:%s, local_ip:%s,gdev_ip:%s,"
                    "local_mac:%s, gdev_mac:%s,inner_smac:%s, inner_dmac:%s", 
                    vxlan_info->vxlan_link_id,vxlan_info->vxlan_encap_type,vxlan_info->vxlan_link_dir,
                    vxlan_info->vxlan_vpn_id,vxlan_info->vxlan_outer_local_port,vxlan_info->vxlan_outer_gdev_port,
                    vxlan_info->vxlan_outer_local_ip,vxlan_info->vxlan_outer_gdev_ip,vxlan_info->vxlan_outer_local_mac,
                    vxlan_info->vxlan_outer_gdev_mac,vxlan_info->vxlan_inner_smac, vxlan_info->vxlan_inner_dmac);
        size_t vxlan_len = sizeof(struct mrl_vxlan_info);
        temp_len = sizeof(thread_seq);
        sapp_get_platform_opt(SPO_INDEPENDENT_THREAD_ID, &thread_seq, &temp_len);
        mrl_send_to_gdev(thread_seq,vxlan_info,rcv_buff + vxlan_len, recv_len - vxlan_len);
        global_stat.send_gdev_total_pkts ++;
    }
    return NULL;
}


size_t mrl_build_ip_hdr(char *ip_pkt, uint32_t src_ip, uint32_t dst_ip, size_t udp_len)
{
    size_t ip_len = udp_len + MRL_IP_HDR_LEN;
    struct iphdr *ip_hdr =(struct iphdr *)ip_pkt;
    ip_hdr->ihl = 0x5;
    ip_hdr->version=0x4;
    ip_hdr->tos = 0x00;
    ip_hdr->tot_len = htons(ip_len);
    ip_hdr->id = 0x325f;
    ip_hdr->frag_off =0x0000;
    ip_hdr->ttl = 0x80;
    ip_hdr->protocol =0x11;
    ip_hdr->check = 0x0000;
    ip_hdr->saddr = src_ip;
    ip_hdr->daddr = dst_ip;
    ip_hdr->check = mrl_get_checksum((uint16_t *)ip_pkt,MRL_IP_HDR_LEN);
    return ip_len;
}

size_t mrl_build_udp_hdr(char *udp_pkt,size_t payload_len)
{
    struct udphdr *udp_hdr = (struct udphdr *)udp_pkt;
    udp_hdr->source = htons(mrl_instance.mrl_cfg.local_port);
    udp_hdr->dest = htons(mrl_instance.mrl_cfg.dest_port);
    udp_hdr->len = htons(payload_len + MRL_UDP_HDR_LEN);
    udp_hdr->check = 0;
    return payload_len + MRL_UDP_HDR_LEN;
}

size_t mrl_build_udp_payload(char *payload)
{
    const char content[]="This is detect info";
    size_t payload_len = strlen(content);
    memcpy(payload,content,payload_len);
    return payload_len;
}

void mrl_construct_vxlan_info(struct mrl_vxlan_info *vxlan_info, int gdev_index, int link_id_index)
{
    memcpy(vxlan_info->vxlan_outer_local_mac, mrl_instance.mrl_cfg.vxlan_outer_local_mac,strlen(mrl_instance.mrl_cfg.vxlan_outer_local_mac));
    memcpy(vxlan_info->vxlan_outer_local_ip, mrl_instance.mrl_cfg.vxlan_outer_local_ip,strlen(mrl_instance.mrl_cfg.vxlan_outer_local_ip));
    memcpy(vxlan_info->vxlan_outer_local_port, mrl_instance.mrl_cfg.xvlan_outer_local_port,strlen(mrl_instance.mrl_cfg.xvlan_outer_local_port));

    memcpy(vxlan_info->vxlan_outer_gdev_mac, mrl_instance.mrl_cfg.vxlan_outer_gdev_mac,strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_mac));
    memcpy(vxlan_info->vxlan_outer_gdev_ip,  mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[gdev_index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_ip[gdev_index]));
    memcpy(vxlan_info->vxlan_outer_gdev_port , mrl_instance.mrl_cfg.vxlan_outer_gdev_port[gdev_index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdev_port[gdev_index]));
   
    vxlan_info->vxlan_encap_type = 0;
    vxlan_info->vxlan_vpn_id= mrl_instance.mrl_cfg.vxlan_vpn_id;
    vxlan_info->vxlan_link_dir = mrl_instance.mrl_cfg.vxlan_link_dir;
    
    vxlan_info->vxlan_link_id = mrl_instance.mrl_cfg.vxlan_link_id[gdev_index][link_id_index];
    memcpy(vxlan_info->vxlan_inner_smac, mrl_instance.mrl_cfg.vxlan_inner_smac[gdev_index][link_id_index],strlen(mrl_instance.mrl_cfg.vxlan_inner_smac[gdev_index][link_id_index]));
    memcpy(vxlan_info->vxlan_inner_dmac, mrl_instance.mrl_cfg.vxlan_inner_dmac[gdev_index][link_id_index], strlen(mrl_instance.mrl_cfg.vxlan_inner_dmac[gdev_index][link_id_index]));
}
void mrl_detect_action(uint32_t src_ip)
{
    char debug_ip[MRL_STR_IP_LEN];
    memset(debug_ip,0,MRL_STR_IP_LEN);
    inet_ntop(AF_INET, &src_ip,debug_ip, MRL_STR_IP_LEN);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_detect_action","detect ip is %s",debug_ip);
    unsigned int i = 0, j =0;
    int thread_seq = 0;
    int temp_len = 0;
    size_t udp_payload_len = 0;
    size_t udp_len = 0;
    size_t ip_len = 0;
    uint32_t dest_ip = 0;
    char ip_pkt[MRL_IP_PKT_LEN];
    memset(ip_pkt,0,MRL_IP_PKT_LEN);
    struct mrl_vxlan_info vxlan_info;
    sapp_get_platform_opt(SPO_INDEPENDENT_THREAD_ID, &thread_seq, &temp_len);
    udp_payload_len = mrl_build_udp_payload(ip_pkt +MRL_UDP_HDR_LEN + MRL_IP_HDR_LEN);
    udp_len = mrl_build_udp_hdr(ip_pkt + MRL_IP_HDR_LEN, udp_payload_len);
    inet_pton(AF_INET,mrl_instance.mrl_cfg.dest_ip,&dest_ip);
    ip_len = mrl_build_ip_hdr(ip_pkt,src_ip,dest_ip,udp_len);
    for(i = 0; i< mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        for(j = 0; j < mrl_instance.mrl_cfg.vxlan_link_id_num; j ++)
        {
            memset(&vxlan_info,0,sizeof(vxlan_info));
            mrl_construct_vxlan_info(&vxlan_info,i,j);
            mrl_send_to_gdev(thread_seq,&vxlan_info,ip_pkt,ip_len);
            global_stat.send_detect_pkts ++;
            global_stat.send_gdev_total_pkts ++;
        }
    }
}

void *mrl_detect_ip_action(void *arg)
{
    sleep(20);
    int ret = 0;
    uint32_t detect_ip = 0;
    long int ip_len = sizeof(detect_ip);
    while(1)
    {
        if(mrl_instance.stop_flag == 1)
        {
            break;
        }
        ret = MESA_lqueue_get_head(mrl_instance.mrl_queue,&detect_ip,&ip_len);
        if(ret != 0)
        {
            MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_detect_ip_action","MESA_lqueue_get_head func error! ret is %d",ret);
            assert(0);
        }
        mrl_detect_action(detect_ip);
    }
    return NULL;
}

