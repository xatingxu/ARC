CC=g++
bin=wjk
src=wjk.cc
INCLUDE=-IAip/
LIB=-ljsoncpp	-lcurl	-lcrypto	-lpthread
FLAGS=-D_LOG_
$(bin):$(src)
	$(CC)	-o	$@	$^	$(FLAGS)	$(INCLUDE)	$(LIB)
.PHONY:clean
clean:
	rm	-f	$(bin)
	
