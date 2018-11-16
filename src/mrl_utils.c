#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<ctype.h>
#include<time.h>
#include<errno.h>

#include "mrl_utils.h"
#include "mrl_main.h"
extern struct mrl_global_instance mrl_instance;

/* ascii字符转16进制 */
char mrl_ascii_to_hex(char ascii)
{
	char c = 0;
	
	switch(ascii)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			c = ascii - 0x30;
		break;

		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			c = 10 + ascii - 0x61;
		break;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			c = 10 + ascii - 0x41;
		break;
	}

	return c;
}

/*  2012-04-11 LiJia add,将MAC字符串形式转换为16进制MAC地址.
参数:
	str:   MAC地址字符串
	delim: 字符串分隔符，常见为':', '-'等,如: xx:xx:xx:xx:xx:xx
			如果字符串无分隔符，delim设为-1.
	mac:   存储MAC地址的数组(指针),结果为网络序,
		   如网卡MAC地址为11:22:33:44:55:66,则mac[0]为0x11,mac[5]为0x66.
返回值:
	0: 正常
	-1:错误
*/
int mrl_mac_pton(const char *str, int delim, uint8_t *mac)
{
#define MAC_STR_LEN_DELIM		(17) /* length of "11:22:33:44:55:66" */
#define MAC_STR_LEN_NODELIM		(12) /* length of "112233445566" */
	const char *s = str;
	int i; 

	/* 检查输入合法性 */
	if(delim != -1)
	{
		if(strlen(str) != MAC_STR_LEN_DELIM) 
		{
			MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mac_pton","MAC string length error!");
            assert(0);
			return -1;
		}
	}
	else
	{
		if(strlen(str) != MAC_STR_LEN_NODELIM) 
		{
			MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mac_pton","MAC string length error!");
            assert(0);
            return -1;
		}
	}

	/* 检查输入合法性，同时转换成16进制值 */
	for(i = 0; i < 6; i++)
	{
		mac[i] = 0; /* 先清零，赋值语句都是或操作 */
		if(isxdigit(*s)==0)
		{
			MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mac_pton","MAC string type error!");
            assert(0);
            return -1;
		}
		mac[i] |= mrl_ascii_to_hex(*s) << 4;
		s++;
		
		if(isxdigit(*s)==0)
		{
			MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mac_pton","MAC string type error!");
            assert(0);
            return -1;
		}
		mac[i] |= mrl_ascii_to_hex(*s);
		s++;
		
		if((delim != -1) && i<5 && (*s++ != (char)delim))
		{
		    MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mac_pton","MAC string type error!");
            assert(0);
			return -1;
		}
	}

	return 0;
}

unsigned int mrl_split_str(char *str, const char *delim, char **dest)
{
    unsigned int i = 0;
    char *temp;
    size_t len = 0;
    temp = strtok(str, delim);
    while (temp != NULL)
    {
        len = strlen(temp);
        /* While there are tokens in "string"*/
        if(memcpy(dest[i], temp, len) <0)
        {
            assert(0);
        }
        dest[i][len]='\0';
        i++;
        /* Get next token: */
        temp = strtok(NULL, delim);
    }
    return i;
}

void get_cur_time(char *date)
{
    time_t t;
    struct tm *lt;
    time(&t);
    lt = localtime(&t);
    snprintf(date, MRL_DATE_LEN,"%d/%d/%d/%d:%d:%d",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
}

unsigned short mrl_get_checksum(unsigned short * buffer, size_t len)
{
    unsigned int sum = 0;
 
    while (len > 1) {
        sum += *buffer++;
        len -= sizeof(unsigned short);
    }
 
    if (len) {
        sum += *(unsigned char *)buffer;
    }
 
    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xffff);
    }
 
    return (unsigned short)(~sum);
}

void mrl_mmdb_init(const char *path)
{
    printf("mrl mmdb init\n");
    int status =MMDB_open(path,MMDB_MODE_MMAP,&(mrl_instance.mrl_mmdb));
    if(status != MMDB_SUCCESS)
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mmdb_init","Can't open %s - %s!",path,MMDB_strerror(status));
        if(status == MMDB_IO_ERROR)
        {
            MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mmdb_init","IO error: %s",strerror(errno));
        }
        assert(0);
    }
}

int mrl_search_ip_country(MMDB_s mmdb, const char *ip_address, const char *mycountry)
{
    int gai_error, mmdb_error;      
    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, ip_address, &gai_error, &mmdb_error);
    if (0 != gai_error) 
    {       
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mmdb_search","Error from getaddrinfo for %s - %s!",ip_address,gai_strerror(gai_error));
        assert(0); 
           
    }   
    if (MMDB_SUCCESS != mmdb_error) 
    {
        MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mmdb_search","Got an error from libmaxminddb: %s!",MMDB_strerror(mmdb_error));
        assert(0);        
    }  
    MMDB_entry_data_s entry_data;
    if (result.found_entry) 
    {      
        int status = MMDB_get_value(&result.entry, &entry_data, "COUNTRY", NULL);
        if (MMDB_SUCCESS != status)     
        {
            MESA_handle_runtime_log(mrl_instance.mrl_log_handle, RLOG_LV_FATAL,"mrl_mmdb_search","Got an error looking up the entry data - %s!",MMDB_strerror(status));
            assert(0);
        }      
        if (entry_data.has_data)        
        {           
            if(memcmp(mycountry, entry_data.utf8_string, entry_data.data_size) == 0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
    return -1;
}

