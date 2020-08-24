#include"HttpServer.hpp"
#include<signal.h>
#include<unistd.h>


void Guide(std::string proc)
{
    std::cout<<"Usage "<<proc<<" port"<<std::endl;
}


int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        Guide(argv[0]);
        exit(1);
    }

    int pid = fork();
    if(pid < 0)
    {
        LOG(ERROR,"Daemon creation failed");
        return 0;
    }
    if(pid == 0)
    {
        LOG(INFO,"Daemon creation success");
        umask(0);
        int fd_out = open("Log.txt",O_WRONLY|O_RDONLY|O_CREAT,0644);
        dup2(fd_out,1);
        signal(SIGPIPE, SIG_IGN);
        HttpServer* hp = new HttpServer(atoi(argv[2]),argv[1]);
        hp->InitServer();
        hp->StartServer();

        delete hp;
        LOG(INFO,"Server down");
        exit(0);
    }
    else {

        return 0;
    }

    //test git usage;

}



