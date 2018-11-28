CC = gcc

CXX = g++

CCFLAGS = -Wall -g -fPIC

CXXFLAGS = -Wall -g -fPIC

INCLUDES = -I ../include

INCLUDES += -I /opt/mrzcpd/include

INCLUDES += -I /opt/MESA/include 

INCLUDES += -I /opt/MESA/include/MESA

LIBS = -L /opt/MESA/lib

#LIBS = -L ../support/lib

LIBS += -lMESA_prof_load -lMESA_handle_logger -lpthread -lmaatframe -lMESA_htable -lMESA_field_stat2 -lmaxminddb

TARGET = mrl.so

OBJS = mrl_main.o mrl_utils.o mrl_packet.o mrl_redis.o mrl_stat.o

.PHONY: all clean install

all:$(TARGET) 

$(TARGET):$(OBJS)
	$(CXX) -shared $(CXXFLAGS) $(OBJS) $(LIBS) -o $@  
	
$(OBJS):%.o:%.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o
	
install:
	cp $(TARGET) /home/lijie/sapp/plug/business/mrl
	cp ../bin/conf/mrl.inf /home/lijie/sapp/plug/business/mrl
	cp ../bin/conf/*.conf /home/lijie/sapp/mrl_conf/
	cp ../bin/conf/all_ip_only_coun_v4.mmdb /home/lijie/sapp/mrl_conf
