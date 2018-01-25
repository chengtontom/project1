CC=gcc  
  
TARGET = ct_statd
SRCS = main.c db_lib.c achieve_module.c

$(TARGET):$(SRCS)
	$(CC) -g -o $(TARGET) $^  \
	-L/usr/lib/ -lmysqlclient -I/usr/include/mysql \
	-lpthread

clean:  
	rm $(TARGET) 
  
