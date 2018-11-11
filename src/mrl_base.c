#include<string.h>
#include<stdio.h>
#include<assert.h>
#include <ctype.h>
#include "mrl_base.h"
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
int mrl_mac_pton(const char *str, int delim, char *mac)
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
			printf("MAC string length error!\n");
            assert(0);
			return -1;
		}
	}
	else
	{
		if(strlen(str) != MAC_STR_LEN_NODELIM) 
		{
			printf("MAC string length error!\n");
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
			printf("MAC string type error!\n");
            assert(0);
            return -1;
		}
		mac[i] |= mrl_ascii_to_hex(*s) << 4;
		s++;
		
		if(isxdigit(*s)==0)
		{
			printf("MAC string type error!\n");
            assert(0);
            return -1;
		}
		mac[i] |= mrl_ascii_to_hex(*s);
		s++;
		
		if((delim != -1) && i<5 && (*s++ != (char)delim))
		{
			printf("MAC string type error!\n");
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

