#include <arpa/inet.h>
#include <sys/types.h>
#include <assert.h>
#include<unistd.h>

#include "stream.h"
#include "stream_inc/stream_control.h"

#include "mrl_packet.h"
#include "mrl_main.h"
#include "mrl_base.h"

#define MRL_MAC_LEN 6
#define MRL_BUFF_LEN 3000

#define PORT_MODE ((1<<10) -1)
#define PORT_MASK (((1<<15) -1)^((1<<6) -1))

extern struct mrl_global_instance mrl_instance;

void mrl_socket_init()
{
    //init send fd
    int i;
    int thread_num = 0;
    int len = sizeof(thread_num);
    thread_num = 5;
    //sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
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
    thread_num = 5;
    //sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
    for(i = 0; i< thread_num;i++)
    {
      close(mrl_instance.mrl_snd_fd[i]);   
    }
    
    close(mrl_instance.mrl_rcv_fd);
}

ssize_t mrl_sock_send(int fd, const char *pkt, ssize_t pkt_len, struct sockaddr_in *dest)
{
    ssize_t send_len = 0;
    send_len = sendto(fd, pkt,pkt_len,0,(struct sockaddr *)dest,sizeof(struct sockaddr_in));
    return send_len;
}

ssize_t mrl_sock_recv(int fd, char *recv_buff, size_t buff_len)
{
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t recv_len = 0;
    recv_len = recvfrom(fd, recv_buff, buff_len, 0, (struct sockaddr *) &from, & from_len);
    return recv_len ;
}


void mrl_send_to_gdev(int thread_seq, struct mrl_vxlan_info * vxlan_info, const char *payload, size_t payload_len)
{ 
    SAPP_TLV_T option[32];
    int option_num = 0;

    char temp_mac[MRL_MAC_LEN];
    unsigned int temp_ip;
    short  temp_port;

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

    
    /*MESA_sendpacket_iplayer_options(thread_seq,  
                         payload,
                         payload_len,
                         vxlan_info->vxlan_link_dir,
                         option,
                         option_num);*/
}


//判断是否是IP复用包
bool mrl_pkt_signature_identify(struct streaminfo *mystream)
{
    UINT16  sport,dport;
    UINT32 sip,dip;
    sport = ntohs(mystream->addr.tuple4_v4->source);
    dport = ntohs(mystream->addr.tuple4_v4->dest);
    sip = ntohl(mystream->addr.tuple4_v4->saddr);
    dip = ntohl(mystream->addr.tuple4_v4->daddr);
    if(((sport ^ sip ^dip)%PORT_MODE) == (dport & PORT_MASK))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void mrl_send_to_mgw(void *raw_pkt, int thread_seq)
{
    const char *pkt = (const char *)raw_pkt;
    ssize_t pkt_len = strlen(pkt);
    ssize_t send_len = 0;
    send_len = mrl_sock_send(mrl_instance.mrl_snd_fd[thread_seq],pkt,pkt_len,&(mrl_instance.mgw_addr));
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
        recv_len = mrl_sock_recv(mrl_instance.mrl_rcv_fd,rcv_buff, MRL_BUFF_LEN);
        struct mrl_vxlan_info *vxlan_info = (struct mrl_vxlan_info *)rcv_buff;
        size_t vxlan_len = sizeof(struct mrl_vxlan_info);
        temp_len = sizeof(thread_seq);
        thread_seq = 1;
        //sapp_get_platform_opt(SPO_INDEPENDENT_THREAD_ID, &thread_seq, &temp_len);
        mrl_send_to_gdev(thread_seq,vxlan_info,rcv_buff + vxlan_len, recv_len - vxlan_len);
    }
    return NULL;
}


