#include"mrl_stat.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

struct global_stat_t global_stat;
struct fs_stat_t global_fs_stat;
void mrl_stat_init()
{
    int value = 0;
    char stat_file[] = "log/mrl_stat.log";
    memset(&global_stat,0,sizeof(struct global_stat_t));

    global_fs_stat.handle = FS_create_handle();

    FS_set_para(global_fs_stat.handle, OUTPUT_DEVICE, stat_file, strlen(stat_file)+1);

    value = 1;
    FS_set_para(global_fs_stat.handle, PRINT_MODE, &value, sizeof(value));

    value = 0;
    FS_set_para(global_fs_stat.handle, CREATE_THREAD, &value, sizeof(value));

    /* field */
    global_fs_stat.fs_field_id[FIELD_RECV_MGW_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"RECV_MGW_PKTS");
    global_fs_stat.fs_field_id[FIELD_SEND_MGW_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"SEND_MGW_PKTS");

    global_fs_stat.fs_field_id[FIELD_RECV_GDEV_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"RECV_GDEV_PKTS");
    global_fs_stat.fs_field_id[FIELD_SEND_GDEV_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"SEND_GDEV_PKTS");

    global_fs_stat.fs_field_id[FIELD_RECV_IR_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"rec_ir_pkts");

    global_fs_stat.fs_field_id[FIELD_RECV_GDEV_TCP_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"recv_gdev_tcp_pkts");
    global_fs_stat.fs_field_id[FIELD_RECV_GDEV_UDP_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"recv_gdev_udp_pkts");

    global_fs_stat.fs_field_id[FIELD_SEND_DETECT_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"send_detect_pkts");
    global_fs_stat.fs_field_id[FIELD_RECV_DETECT_PKTS] = FS_register(global_fs_stat.handle, FS_STYLE_FIELD, FS_CALC_SPEED,"recv_detect_pkts");

    /*status*/
    global_fs_stat.fs_status_id[STATUS_MALLOC_MEMORY] = FS_register(global_fs_stat.handle, FS_STYLE_STATUS, FS_CALC_CURRENT, "malloc_memory(Byte)");
    global_fs_stat.fs_status_id[STATUS_FREE_MEMORY] = FS_register(global_fs_stat.handle, FS_STYLE_STATUS, FS_CALC_CURRENT, "free_memory(Byte)");

    FS_start(global_fs_stat.handle);
}

void mrl_stat_output()
{
    /*field*/
    
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_SEND_MGW_PKTS], 0, FS_OP_SET, global_stat.send_to_mgw_pkts);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_MGW_PKTS], 0, FS_OP_SET, global_stat.recv_from_mgw_pkts);
    
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_SEND_GDEV_PKTS], 0, FS_OP_SET, global_stat.send_gdev_total_pkts);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_GDEV_PKTS], 0, FS_OP_SET, global_stat.recv_gdev_total_pkts);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_GDEV_TCP_PKTS], 0, FS_OP_SET, global_stat.recv_gdev_tcp_pkts);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_GDEV_UDP_PKTS], 0, FS_OP_SET, global_stat.recv_gdev_udp_pkts);

    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_IR_PKTS], 0, FS_OP_SET, global_stat.recv_ir_pkts);
    
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_SEND_DETECT_PKTS], 0, FS_OP_SET, global_stat.send_detect_pkts);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_field_id[FIELD_RECV_DETECT_PKTS], 0, FS_OP_SET, global_stat.recv_detect_pkts);
   
    /*status*/
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_status_id[STATUS_MALLOC_MEMORY], 0, FS_OP_SET, global_stat.malloc_memory);
    FS_operate(global_fs_stat.handle, global_fs_stat.fs_status_id[STATUS_FREE_MEMORY], 0, FS_OP_SET, global_stat.free_memory);
    FS_passive_output(global_fs_stat.handle);
}


void *mrl_stat_action(void *arg)
{
    printf("start stat thread\n");
	for(;;)
	{
		mrl_stat_output();
		sleep(1);
	}
    return NULL;
}


