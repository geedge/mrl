void mrl_socket_init()
{
    //init send fd
    int i;
    int thread_num = 0;
    int len = sizeof(thread_num);
    sapp_get_platform_opt(SPO_THREAD_COUNT,(void *) &thread_num, &len);
    mrl_instance.mrl_rcv_fd = (int *) malloc (sizeof(int)*thread_num);
    for(i = 0; i< thread_num;i++)
    {
        mrl_instance.mrl_rcv_fd = socket(AF_INET, SOCK_DGRAM, 0);   
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
    int vxlan_link_id = 1;

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
    option[option_num].int_value = vxlan_info.vxlan_link_id;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info.vxlan_encap_type;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_VXLAN_LINK_DIR;
    option[option_num].length = 1;
    option[option_num].char_value = vxlan_info.vxlan_link_dir;
    option_num++;///////

    
    option[option_num].type = SAPP_SEND_OPT_INNER_LINK_ENCAP_TYPE;
    option[option_num].length = 1;
    option[option_num].char_value = TUNNAT_TUNNEL_TYPE_ETHER;
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

    
    MESA_sendpacket_iplayer_options(thread_seq,  
                         payload,
                         payload_len,
                         vxlan_info.vxlan_link_dir,
                         option,
                         option_num);
}

void mrl_get_candidate_vxlan_info(struct streaminfo *mystream, struct mrl_vxlan_info  *vxlan)
{
    int gdev_ip = 0;
    get_rawpkt_opt_from_streaminfo(mystream, RAW_PKT_GET_GDEV_IP, &(gdev_ip));
    inet_ntop(AF_INET, &gdev_ip, vxlan->vxlan_outer_gdev_ip, MRL_STR_IP_LEN);
    int index = 0;
    for(; index < mrl_instance.mrl_cfg.gdev_num; index++)
    {
        if(memcmp(vxlan->vxlan_outer_gdev_ip,mrl_instance.mrl_cfg.gdev_ip[index],strlen(vxlan->vxlan_outer_gdev_ip)) == 0)
        {
            break;
        }
    }
    assert(index < mrl_instance.mrl_cfg.gdev_num);//如果等于就说明该数据包的GDEV不在主动探测的GDEV中，错误
    memcpy(vxlan->vxlan_inner_smac, mrl_instance.mrl_cfg.vxlan_inner_smacs[index],strlen(mrl_instance.mrl_cfg.vxlan_inner_smacs[index]));
    memcpy(vxlan->vxlan_inner_dmac, mrl_instance.mrl_cfg.vxlan_inner_dmacs[index], strlen(mrl_instance.mrl_cfg.vxlan_inner_dmacs[index]));
    vxlan->vxlan_encap_type = mrl_instance.mrl_cfg.vxlan_encap_types[index];
    vxlan->vxlan_link_id = mrl_instance.mrl_cfg.vxlan_link_ids[index];
    vxlan->vxlan_link_dir = mrl_instance.mrl_cfg.vxlan_link_dirs[index];
    vxlan->vxlan_outer_local_port = mrl_instance.mrl_cfg.xvlan_outer_locals_port[index];
    vxlan->vxlan_outer_gdev_port = mrl_instance.mrl_cfg.vxlan_outer_gdevs_port[index];
    memcpy(vxlan->vxlan_outer_local_ip, mrl_instance.mrl_cfg.vxlan_outer_locals_ip[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_locals_ip[index]));
    memcpy(vxlan->vxlan_outer_gdev_ip,  mrl_instance.mrl_cfg.vxlan_outer_gdevs_ip[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdevs_ip[index]));
    memcpy(vxlan->vxlan_outer_local_mac, mrl_instance.mrl_cfg.vxlan_outer_locals_mac[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_locals_mac[index]));
    memcpy(vxlan->vxlan_outer_gdev_mac, mrl_instance.mrl_cfg.vxlan_outer_gdevs_mac[index],strlen(mrl_instance.mrl_cfg.vxlan_outer_gdevs_mac[index]));
    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_DEBUG,"mrl_get_candidate_vxlan_info","get vxlan opt from rawpkt:
                inner_smac:%s, inner_dmac:%s,encap_type:%d,link_id:%d, link_dir:%d, 
                local_port:%u,gdev_port:%u, local_ip:%s,gdev_ip:%s,local_mac:%s, gdev_mac:%s", 
                vxlan->vxlan_inner_smac, vxlan->vxlan_inner_dmac, vxlan->vxlan_encap_type,
                vxlan->vxlan_link_id, vxlan->vxlan_link_dir,vxlan->vxlan_outer_local_port,
                vxlan->vxlan_outer_gdev_port,vxlan->vxlan_outer_local_ip,vxlan->vxlan_outer_gdev_ip,
                vxlan->vxlan_outer_local_mac,vxlan->vxlan_outer_gdev_mac);

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


void *mrl_recv_mgw_action(void *arg)
{
    
    char rcv_buff[MRL_BUFF_LEN];
    ssize_t recv_len = 0;
    int thread_seq = 0;
    int temp_len = sizeof(thread_seq);
    sapp_get_platform_opt(SPO_REQUEST_SEND_THREAD_ID, &thread_seq, &temp_len);
    recv_len = mrl_sock_recv(mrl_instance.mgw_rcv_fd,rcv_buff, MRL_BUFF_LEN);
    struct mrl_vxlan_info *vxlan_info = (struct mrl_vxlan_info *)rcv_buff;
    size_t vxlan_len = sizeof(struct mrl_vxlan_info);
    mrl_send_to_gdev(thread_seq,vxlan_info,rcv_buff + vxlan_len, recv_len - vxlan_len);
}

void mrl_send_to_mgw(void *raw_pkt, int thread_seq)
{
    const char *pkt = (const char *)raw_pkt;
    ssize_t pkt_len = strlen(pkt);
    ssize_t send_len = 0;
    send_len = mrl_sock_send(mrl_instance.mrl_snd_fd[thread_seq],pkt,pkt_len,*(mrl_instance.mgw_addr));
    assert(send_len == pkt_len);
}


