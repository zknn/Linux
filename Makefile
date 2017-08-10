 ROOT_PATH=$(shell pwd)
CONF=$(ROOT_PATH)/conf
PLUGIN=$(ROOT_PATH)/plugin
WWWROOT=$(ROOT_PATH)/wwwroot
cc=gcc
bin=httpd
src=httpd.c main.c
obj=$(shell  echo $(src) | sed 's/\.c/\.o/g')
LDFLAGS=-lpthread
FLAGS=-D_STDOUT_  #-D_DEBUG_

.PHONY:all
all:$(bin) cgi

$(bin):$(obj) 
	$(cc) -o $@ $^ $(LDFLAGS)
%.o:%.c
	$(cc) -c $< $(FLAGS)  

cgi:
	cd $(WWWROOT)/cgi-bin;\
	make;\
	cp mathcgi $(ROOT_PATH)
.PHONY:clean
clean:
	rm -rf *.o $(bin) output;\
	cd $(WWWROOT)/cgi-bin;\
	make clean

.PHONY:debug
debug:
	@echo $(src)
	@echo $(obj)

.PHONY:output
output:
	mkdir -p output/wwwroot/cgi-bin
	mkdir -p output/conf
	cp httpd output
	cp mathcgi output/wwwroot/cgi-bin
	cp wwwroot/index.html output/wwwroot
