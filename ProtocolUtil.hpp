#ifndef __PROTOCOL_UTIL_HPP__
#define __PROTOCOL_UTIL_HPP__

#include<stdlib.h>
#include<iostream>
#include<string>
#include<sstream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include"Log.hpp"



class Run
{
public:
    static void* SayHi(void * arg_)
    {
        int id = *(int *)arg_;
        for(int i = 0; i< 5 ;++i)
        {
            std::cout<<"hi :"<<id<<std::endl;
            sleep(1);
        }
    }

};






#endif
