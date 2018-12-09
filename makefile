bin=HttpServer
cc=g++


.PHONY:$(bin) clean


$(bin):HttpServer.cc
	$(cc) -o $@ $^ -lpthread

clean:
	rm -f $(bin)
	



