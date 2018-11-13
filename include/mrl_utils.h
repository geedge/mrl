#ifndef __MRL_UTILS_H_
#define __MRL_UTILS_H_

unsigned int mrl_split_str(char *str, const char *delim, char **dest);
int mrl_mac_pton(const char *str, int delim, char *mac);
void get_cur_time(char *time);


#endif
