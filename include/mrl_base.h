#ifndef __MRL_BASE_H_
#define __MRL_BASE_H_

unsigned int mrl_split_str(char *str, const char *delim, char **dest);
int mrl_mac_pton(const char *str, int delim, char *mac);


#endif
