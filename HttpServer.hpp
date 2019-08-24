#ifndef __HTTP_SERVER_HPP__
#define __HTTP_SERVER_HPP__



#include"ProtocolUtil.hpp"
#include<pthread.h>
#include"ThreadPool.hpp"


class HttpServer
{
private:
    int listen_sock;
    int port;
    ThreadPool *tp;
    std::string IP;

public:
    HttpServer(int port_,std::string arg):listen_sock(-1),port(port_),IP(arg)
    { }


    void InitServer()
    {
        listen_sock = socket(AF_INET,SOCK_STREAM,0);
        if(listen_sock < 0)
        {
            LOG(ERROR,"create socket error");
            exit(2);
        }

        int opt_ = 1;
        setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt_,sizeof(opt_));

        struct sockaddr_in local_;
        local_.sin_family = AF_INET;
        local_.sin_port = htons(port);
        //local_.sin_addr.s_addr = INADDR_ANY;
        local_.sin_addr.s_addr = inet_addr(IP.c_str());
        
        if(bind(listen_sock, (struct sockaddr*)&local_,sizeof(local_)) < 0)
        {
            LOG(ERROR,"bind socket error");
            exit(3);
        }
        
        if(listen(listen_sock,5) < 0)
        {
            LOG(ERROR,"listen socket error");
            exit(4);
        }
        
        tp = new ThreadPool();
        tp->initThreadPool();
        LOG(INFO,"InitServer success!");
    }

        
        
    void StartServer()
    {
        LOG(INFO,"Server is up and running");
        while(1)
        {
            struct sockaddr_in peer_;
            socklen_t len_ = sizeof(peer_);
            LOG(INFO,"Waitting accept");
            int sock_ = accept(listen_sock,(struct sockaddr*)&peer_,&len_);
            if(sock_ < 0)
            {
                LOG(WARNING,"accept error");
                continue;
            }
            LOG(INFO,"Get new client");
        
            Task t;
            t.SetTask(sock_,Run::HandlerRequest);
            tp->PushTask(t);
            
        }
    }
            

    ~HttpServer()
    {
        if(listen_sock != -1)
        {
            close(listen_sock);
        }
        port = -1;
    }
};



#endif
