bin=HttpServer
cc=g++


.PHONY:$(bin) clean


$(bin):HttpServer.cc
	$(cc) -o $@ $^ -lpthread -std=c++11

clean:
	rm -f Log.txt $(bin)
	
.PHONY:cgi
cgi:
	g++ -o Cal Cal.cc
	mv Cal ./WEB_ROOT



