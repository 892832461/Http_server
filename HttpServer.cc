#include"HttpServer.hpp"
#include<signal.h>
#include<unistd.h>

void Guide(std::string proc)
{
    std::cout<<"Usage "<<proc<<" port"<<std::endl;
}

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        Guide(argv[0]);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    HttpServer hp(atoi(argv[1]));
    hp.InitServer();
    hp.StartServer();

    //delete hp;
    std::cout<<"see you agine"<<std::endl;
    return 0;

    //test git usage;

}
