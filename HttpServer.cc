#include"HttpServer.hpp"
#include<signal.h>
#include<unistd.h>

void Guide(sting proc)
{
    cout<<"Usage "<<proc<<" port"<<endl;
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
    hp.Init();
    hp.Start();

    delete hp;
    return 0;
}
