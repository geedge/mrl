#include <arpa/inet.h>
#include <sys/types.h>
#include <assert.h>
#include<unistd.h>
#include <stdio.h>

#include "stream.h"
#include "stream_inc/stream_control.h"

#include "mrl_packet.h"
#include "mrl_main.h"
#include "mrl_utils.h"
#include "mrl_redis.h"


extern struct mrl_global_instance mrl_instance;
extern void mrl_get_vxlan_info(struct mrl_vxlan_info  *vxlan, unsigned int index);

void mrl_socket_init()
{
    printf("mrl socket init\n");
    int i;
    int thread_num = 0;
    int len = sizeof(thread_num);
    sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
    printf("sapp thread num is %d\n",thread_num);
    mrl_instance.mrl_snd_fd = (int *) malloc (sizeof(int)*thread_num);
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
    printf("sapp thread num is %d\n",thread_num);
    for(i = 0; i< thread_num;i++)
    {
      close(mrl_instance.mrl_snd_fd[i]);   
    }
    
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
    option[option_num].type = SAPP_SEND_OPT_GDEV_SMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;////////

    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_outer_gdev_mac, ':', temp_mac);
    option[option_num].type = SAPP_SEND_OPT_GDEV_DMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////

    inet_pton(AF_INET, vxlan_info->vxlan_outer_local_ip, &temp_ip); 
    option[option_num].type = SAPP_SEND_OPT_GDEV_SIP;
    option[option_num].length = 4;
    option[option_num].int_value = temp_ip;
    option_num++;///////


    inet_pton(AF_INET, vxlan_info->vxlan_outer_gdev_ip, &temp_ip); 
    option[option_num].type = SAPP_SEND_OPT_GDEV_DIP;
    option[option_num].length = 4;
    option[option_num].int_value = temp_ip;
    option_num++;///////


    temp_port = atoi(vxlan_info->vxlan_outer_local_port);
    assert(temp_port >0 && temp_port <= 65535);
    option[option_num].type = SAPP_SEND_OPT_GDEV_UDP_SPORT;
    option[option_num].length = 2;
    option[option_num].short_value = temp_port;
    option_num++;///////


    temp_port = atoi(vxlan_info->vxlan_outer_gdev_port);
    assert(temp_port >0 && temp_port <= 65535);
    option[option_num].type = SAPP_SEND_OPT_GDEV_UDP_DPORT;
    option[option_num].length = 2;
    option[option_num].short_value = temp_port;
    option_num++;///////
    
    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_VPN_ID;
    option[option_num].length = 4;
    option[option_num].int_value = 0;
    option_num++;///////

        
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_ID;
    option[option_num].length = 4;
    option[option_num].int_value = vxlan_info->vxlan_link_id;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info->vxlan_encap_type;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_DIR;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info->vxlan_link_dir;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_INNER_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info->vxlan_encap_type;
    option_num++;///////


    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_inner_smac, ':', temp_mac);
    option[option_num].type = SAPP_SEND_OPT_INNER_SMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////


    memset(temp_mac, 0, MRL_MAC_LEN);
    mrl_mac_pton(vxlan_info->vxlan_inner_dmac, ':', temp_mac);
    option[option_num].type = SAPP_SEND_OPT_INNER_DMAC;
    option[option_num].length = MRL_MAC_LEN;
    memcpy(option[option_num].array_value, temp_mac, MRL_MAC_LEN);
    option_num++;///////

    printf("send to gdev\n");
    MESA_sendpacket_iplayer_options(thread_seq,  
                         payload,
                         payload_len,
                         vxlan_info->vxlan_link_dir,
                         option,
                         option_num);
}


//判断是否是IP复用包
bool mrl_pkt_signature_identify(struct streaminfo *mystream)
{
    printf("start to signature identify\n");
    uint8_t type = 0;
    uint16_t  sport = 0,dport = 0;
    uint32_t sip = 0,dip = 0;
    sport = mystream->addr.tuple4_v4->source;
    dport = mystream->addr.tuple4_v4->dest;
    sip = mystream->addr.tuple4_v4->saddr;
    dip = mystream->addr.tuple4_v4->daddr;
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
    printf("cur stream sport is %u and dport is %u and sip is %d and dip is %d and protocol is %d\n",sport,dport,sip,dip,type);
    unsigned int hash_sport = sport ^ sip ^ dip ^type;
    hash_sport = hash_sport & 0xff;
    if(hash_sport == dport)
    {
        return true;
    }
    else
    {
        if(type == MRL_UDP_TYPE)
        {
            unsigned int hash_dport = dport ^ sip ^ dip ^type;
            hash_dport = hash_dport & 0xff;
            if(hash_dport == sport)
            {
                return true;
            }
        }
        return false;
    }
}


void mrl_send_to_mgw(void *raw_pkt, int thread_seq)
{
    const char *pkt = (const char *)raw_pkt;
    ssize_t pkt_len = strlen(pkt);
    ssize_t send_len = 0;
    send_len = mrl_sock_send(mrl_instance.mrl_snd_fd[thread_seq],pkt,pkt_len,&(mrl_instance.mgw_addr));
    printf("send socket len is %ld\n",send_len);
}


void *mrl_recv_mgw_action(void *arg)
{
    
    char rcv_buff[MRL_BUFF_LEN];
    ssize_t recv_len = 0;
    int thread_seq = 0;
    int temp_len = 0;
    while(1)
    {
        recv_len = mrl_sock_recv(mrl_instance.mrl_rcv_fd,rcv_buff, MRL_BUFF_LEN);
        printf("recv from mgw len is %ld\n",recv_len);
        struct mrl_vxlan_info *vxlan_info = (struct mrl_vxlan_info *)rcv_buff;
        size_t vxlan_len = sizeof(struct mrl_vxlan_info);
        temp_len = sizeof(thread_seq);
        sapp_get_platform_opt(SPO_INDEPENDENT_THREAD_ID, &thread_seq, &temp_len);
        printf("send to gdev thread seq is %d\n",thread_seq);
        mrl_send_to_gdev(thread_seq,vxlan_info,rcv_buff + vxlan_len, recv_len - vxlan_len);
    }
    return NULL;
}


size_t mrl_build_ip_hdr(char *ip_pkt, const char *src_ip, const char *dst_ip, size_t udp_len)
{
    size_t ip_len = udp_len + MRL_IP_HDR_LEN;
    struct iphdr *ip_hdr =(struct iphdr *)ip_pkt;
    ip_hdr->ihl = 0x5;
    ip_hdr->version=0x4;
    ip_hdr->tos = 0x00;
    ip_hdr->tot_len = ip_len;
    ip_hdr->id = 0x325f;
    ip_hdr->frag_off =0x0000;
    ip_hdr->ttl = 0x80;
    ip_hdr->protocol =0x11;
    ip_hdr->check = 0x0000;
    inet_pton(AF_INET,src_ip,&(ip_hdr->saddr));
    inet_pton(AF_INET,dst_ip,&(ip_hdr->daddr));
    ip_hdr->check = htons(mrl_get_checksum((uint16_t *)ip_pkt,MRL_IP_HDR_LEN));
    return ip_len;
}

size_t mrl_build_udp_hdr(char *udp_pkt,size_t payload_len)
{
    struct udphdr *udp_hdr = (struct udphdr *)udp_pkt;
    udp_hdr->source = htons(mrl_instance.mrl_cfg.local_port);
    udp_hdr->dest = htons(mrl_instance.mrl_cfg.detect_port);
    udp_hdr->len = payload_len + MRL_UDP_HDR_LEN;
    udp_hdr->check = 0;
    udp_hdr->check = htons(mrl_get_checksum((uint16_t *)udp_pkt,payload_len + MRL_UDP_HDR_LEN));
    return payload_len + MRL_UDP_HDR_LEN;
}

size_t mrl_build_udp_payload(char *payload)
{
    const char content[]="This is detect info";
    size_t payload_len = strlen(content);
    memcpy(payload,content,payload_len);
    return payload_len;
}
void mrl_detect_action(const char *ip_addr)
{
    printf("start detect ip %s\n",ip_addr);
    unsigned int i = 0;
    int thread_seq = 0;
    int temp_len = 0;
    size_t udp_payload_len = 0;
    size_t udp_len = 0;
    size_t ip_len = 0;
    char ip_pkt[MRL_IP_PKT_LEN];
    memset(ip_pkt,0,MRL_IP_PKT_LEN);
    struct mrl_vxlan_info vxlan_info;
    sapp_get_platform_opt(SPO_INDEPENDENT_THREAD_ID, &thread_seq, &temp_len);
    udp_payload_len = mrl_build_udp_payload(ip_pkt +MRL_UDP_HDR_LEN + MRL_IP_HDR_LEN);
    udp_len = mrl_build_udp_hdr(ip_pkt + MRL_IP_HDR_LEN, udp_payload_len);
    ip_len = mrl_build_ip_hdr(ip_pkt,ip_addr,mrl_instance.mrl_cfg.detect_ip,udp_len);
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_detect_action","detect pkt len is %d",ip_len);
    for(i = 0; i< mrl_instance.mrl_cfg.vxlan_gdev_num;i++)
    {
        memset(&vxlan_info,0,sizeof(vxlan_info));
        mrl_get_vxlan_info(&vxlan_info,i);
        mrl_send_to_gdev(thread_seq,&vxlan_info,ip_pkt,ip_len);
    }
}

void ht_nominee_iterate_cb(const uchar *key, uint size, void *data, void *user)
{
    if(data != NULL)
    {
        if(MESA_htable_search_cb(mrl_instance.ht_candidate, key, size, NULL,NULL, NULL) == NULL)
        {
            mrl_detect_action((const char *)key);
        }  
    }
    
}
void mrl_detect_init()
{
    printf("mrl detect init\n");
    MESA_htable_iterate(mrl_instance.ht_nominee, ht_nominee_iterate_cb, NULL);
    
}
