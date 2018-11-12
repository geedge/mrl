CC = gcc

CXX = g++

CCFLAGS = -Wall -g -fPIC

CXXFLAGS = -Wall -g -fPIC

INCLUDES = -I ../include

INCLUDES += -I /opt/MESA/include 

INCLUDES += -I /opt/MESA/include/MESA

LIBS = -L /opt/MESA/lib

LIBS += -lMESA_prof_load -lMESA_handle_logger -lpthread -lmaatframe -lMESA_htable

TARGET = mrl

OBJS = mrl_main.o mrl_base.o mrl_packet.o mrl_redis.o

.PHONY: all clean install

all:$(TARGET) 

$(TARGET):$(OBJS)
	$(CXX)  $(LIBS)  $^ -o $@
	
$(OBJS):%.o:%.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o
	
install:
	cp $(TARGET) ../bin
