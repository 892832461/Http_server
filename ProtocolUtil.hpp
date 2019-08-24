#ifndef __PROTOCOL_UTIL_HPP__
#define __PROTOCOL_UTIL_HPP__

#include<stdlib.h>
#include<iostream>
#include<string>
#include<string.h>
#include<strings.h>
#include<sstream>
#include<unordered_map>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include"Log.hpp"


#define WEB_ROOT "WEB_ROOT"
#define HOME_PAGE "index.html"
#define PAGE_404 "404.html"
#define PAGE_400 "400.html"


#define OK 200
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define SERVER_ERROR 500

#define HTTP_VERSION "http/1.0"

std::unordered_map<std::string,std::string> stuffix_map{
      {".html","text/html"},
      {".htm","text/html"},
      {".css","tet/css"},
      {".js","application/x-javascript"}
};



class ProtocolUtil{

  public:
    static void  MakeMap(std::unordered_map<std::string,std::string>&head_map_,std::string &str_)
        {
            std::size_t pos = str_.find(": ");
            if(pos == std::string::npos)
            {
                return ;
            }
            std::string k_ = str_.substr(0,pos);
            std::string v_ = str_.substr(pos+2);
            head_map_.insert(make_pair(k_,v_));
        }


    static std::string IntToString(int state_)
        {
            std::stringstream ss;
            ss<<state_;
            return ss.str();

        }


    static std::string StateToDesc(int state_)
        {
            switch(state_)
            {
                case(200):
                  return "OK";
                case(400):
                  return "Bad Request";
                case(404):
                  return "Not Found";
                case(500):
                  return "Internal Server Error";
                default:
                  return "Unknow";
            }
        }


    static std::string SuffixToType(std::string &suffix_)
        {
            return stuffix_map[suffix_];
        }
};


class Request{
  public:
    std::string rq_line;//请求行
    std::string rq_head;//请求报头
    std::string blank;  //请求空行
    std::string rq_text;//请求正文

  private:
    std::string method; //请求方法
    std::string uri;    //url
    std::string version;//版本号
    bool cgi;           //是否cgi

    std::string path;   //资源路径
    std::string param;  //参数
    
    int resource_size; //资源大小
    std::string resource_suffix; //资源类型
    std::unordered_map<std::string,std::string> head_map;//请求报头kv表
    int content_length; //正文长度

  public:

    Request():blank("\n"),cgi(false),path(WEB_ROOT),resource_size(0),resource_suffix(".html"),content_length(-1)
        {}


    void ParseRequestLine()
        {
            std::stringstream ss(rq_line);
            ss >> method >> uri >> version;
            printf("%s %s %s\n",method.c_str(),uri.c_str(),version.c_str());
        }

    void ParseUri()
        {
            if(strcasecmp(method.c_str(),"GET") == 0)
            {
                std::size_t pos_ = uri.find('?');
                if(std::string::npos != pos_)
                {
                    printf("this is cgi start\n");
                    cgi = true;
                    path += uri.substr(0,pos_);
                    param = uri.substr(pos_ + 1);
                }
                else path += uri;
            }
            else path += uri;
            if(path[path.size() - 1] == '/')
            {
                path += HOME_PAGE;
            }
        }


    bool ParseRequestHead()
        {
            size_t  start = 0;
            while(start < rq_head.size())
            {
                std::size_t pos = rq_head.find('\n',start);
                if(std::string::npos == pos)
                    break;
              
                std::string sub_string = rq_head.substr(start,pos - start);
                if(!sub_string.empty())
                {
                    ProtocolUtil::MakeMap(head_map,sub_string);
                }
                else 
                {
                    break;
                }
                start = pos + 1;
            }
            return true;
        }


    int GetContentLength()
        {
            std::string cl_ = head_map["Content-Length"];
            if(!cl_.empty())
            {
                std::stringstream ss(cl_);
                ss >> content_length;
            }
            return content_length;
        }

    std::string & GetParam()
        {
            return param;
        }

    int GetResorceSize()
        {
            return resource_size;
        }

    std::string GetSuffix()
        {
            return resource_suffix;
        }

    std::string GetPath()
        {
            return path;
        }

    std::string GetMethod()
        {
            return method;
        }

    void SetResourceSize(int size_)
        {
            resource_size = size_;
        }

    void SetSuffix(std::string suffix_)
        {
            resource_suffix = suffix_;
        }

    void SetPath(std::string path_)
        {
            path = path_;
        }
    
    bool IsMethodLegal()
        {
            if(strcasecmp(method.c_str(),"GET") == 0
                ||(cgi = strcasecmp(method.c_str(),"POST") == 0)
                ||strcasecmp(method.c_str(),"HEAD") == 0)
                return true;
            return false;
        }
    
    bool IsPathLegal()
        {
            struct stat st;
            if(stat(path.c_str(), &st) < 0)
            {
                LOG(WARNING,"path not found!");
                return false;
            }
            if(S_ISDIR(st.st_mode))
            {
                path += "/";
                path += HOME_PAGE;
            }
            else 
            {
                if((st.st_mode & S_IXUSR) ||
                    (st.st_mode & S_IXGRP) ||
                    (st.st_mode & S_IXOTH))
                    cgi = true;
            }
            resource_size = st.st_size;
            std::size_t pos_ = path.rfind(".");
            if(std::string::npos != pos_)
            {
                resource_suffix = path.substr(pos_);
            }
            return true;
        }


    bool IsNeedRecvText()
        {
            if(strcasecmp(method.c_str(), "POST") == 0)
            {
                return true;
            }
            return false;
        }

    bool IsCgi()
        {
            return cgi;
        }


    ~Request()
        {}
};






class Response{

  public:
    std::string rsp_line;//响应行
    std::string rsp_head;//响应报头
    std::string blank;   //空行
    std::string rsp_text;//响应正文
    int fd;
    int state;            //返回状态



  public:

    Response():blank("\n"),fd(-1),state(OK)
        {}
        

    void MakeStatusLine()
        {
            rsp_line= HTTP_VERSION;
            rsp_line += " ";
            rsp_line += ProtocolUtil::IntToString(state);
            rsp_line += " ";
            rsp_line += ProtocolUtil::StateToDesc(state);
            rsp_line += "\n";
        }

    void MakeResponseHead(Request *&rq_)
        {
            rsp_head = "Content-Length: ";
            if(rq_->GetMethod() == "HEAD")
                rsp_head += "0\n";
            else 
            {
                rsp_head += ProtocolUtil::IntToString(rq_->GetResorceSize());
                rsp_head += "\n";
                rsp_head += "Content-Type: ";
                std::string Suffix_ = rq_->GetSuffix();
                rsp_head += ProtocolUtil::SuffixToType(Suffix_);
                rsp_head += "\n";
            }
        }


    void OpenResource(Request *&rq_)
        {
            std::string path_ = rq_->GetPath();
            fd = open(path_.c_str(),O_RDONLY);
        }


    ~Response()
        {
            close(fd);
        }
};






class Connect
{
  public:
    int sock;


    Connect(int sock_):sock(sock_)
        {}
    
    int RecvOneLine(std::string &line_)
        {
            char c = 'X';
            char last = 'x';
            while(c!='\n')
            {   last = c;
                size_t s = recv(sock,&c,1,0);
                if(s > 0)
                {
                    if(c == '\n')
                    {
                        if(last == '\r')
                            line_.pop_back();  
                    }
                    line_.push_back(c);
                }
                else
                  break;
            }
            return line_.size();
        }

    void RecvRequestHead(std::string &head_)
        {
            head_ = "";
            std::string line_ = "";
            while(line_.size() != 1)
            {
                line_ = "";
                RecvOneLine(line_);
                head_ += line_;
            }
            printf("rq_head_ = ；\n%s\n",head_.c_str());
        }


    void RecvRequestText(std::string &rq_text_,int len_,std::string &param_)
        {
            char c_;
            int i_ = 0;
            while(i_< len_)
            {
                recv(sock,&c_,1,0);
                rq_text_.push_back(c_);
                i_++;
            }
            param_ = rq_text_;
            printf("rq_text : %s\n",rq_text_.c_str());
        }


    void SendResponse(Response *&rsp_,Request *&rq_,bool cgi_)
        {
            std::string &rsp_line_ = rsp_->rsp_line;
            std::string &rsp_head_ = rsp_->rsp_head;
            std::string &rsp_blank_ = rsp_->blank;

            send(sock,rsp_line_.c_str(),rsp_line_.size(),0);
            send(sock,rsp_head_.c_str(),rsp_head_.size(),0);
            send(sock,rsp_blank_.c_str(),rsp_blank_.size(),0);
            if(rq_->GetMethod() == "HEAD")
                return ;
            if(cgi_)
            {
                std::string &rsp_text_ = rsp_->rsp_text;
                send(sock,rsp_text_.c_str(),rsp_text_.size(),0);
            }
            else
            {
                int &fd_ = rsp_->fd;
                sendfile(sock,fd_,NULL,rq_->GetResorceSize());
            }
        }


    ~Connect()
        {
            if(sock>=0)
            close(sock);
        }
};







class Run
{

  public:
    
    static void Process400(Connect *&conn_,Request *&rq_,Response *&rsp_)
        {
            std::string path_ = WEB_ROOT;
            path_ += "/";
            path_ += PAGE_400;
            struct stat st;
            stat(path_.c_str(),&st);
            rq_->SetResourceSize(st.st_size);
            rq_->SetSuffix(".html");
            rq_->SetPath(path_);

            ProcessNoCgi(conn_,rq_,rsp_);

        }
  

    static void Process404(Connect *&conn_,Request *&rq_,Response *&rsp_)
        {
            std::string path_ = WEB_ROOT;
            path_ += "/";
            path_ += PAGE_404;
            struct stat st;
            stat(path_.c_str(),&st);

            rq_->SetResourceSize(st.st_size);
            rq_->SetSuffix(".html");
            rq_->SetPath(path_);

            ProcessNoCgi(conn_,rq_,rsp_);
        }


    static void HandlerError(Connect *&conn_,Request *&rq_,Response *&rsp_)
        {
            int &stata_ = rsp_->state;
            switch(stata_)
            {

                case 400: Process400(conn_,rq_,rsp_);break;
                case 404: Process404(conn_,rq_,rsp_);break;
                case 500: break;
                case 503: break;
            }
        }


    static void ProcessNoCgi(Connect *&conn_,Request *&rq_,Response *&rsp_)
        {
            LOG(INFO, "Make status line");
            rsp_->MakeStatusLine();
            LOG(INFO, "Make Response head");
            rsp_->MakeResponseHead(rq_);
            LOG(INFO, "Open resource");
            rsp_->OpenResource(rq_);
            LOG(INFO, "Send Response");
            conn_->SendResponse(rsp_,rq_,false);
        }


    static void ProcessCgi(Connect *& conn_,Request *&rq_,Response *&rsp_)
        {
            int &state_ = rsp_->state;
            int input[2];
            int output[2];
            std::string &param_ = rq_->GetParam();
            std::string &rsp_text_ = rsp_->rsp_text;
            pipe(input);
            pipe(output);

            pid_t id = fork();
            if(id < 0)
            {
                state_ = SERVER_ERROR;
                return ;
            }
            else if(id == 0)    //child
            {
                LOG(INFO, "CGI new course fork success");
                close(input[1]);
                close(output[0]);
                const std::string &path_ = rq_->GetPath();
                std::string cl_env_ = "Content-Length=";
                cl_env_ += ProtocolUtil::IntToString(param_.size());
                putenv((char*)cl_env_.c_str());

                dup2(input[0],0);
                dup2(output[1],1);
                execl(path_.c_str(),path_.c_str(),NULL);
                exit(1);
            }
            else  //parent
            {
                close(input[0]);
                close(output[1]);

                size_t size_ = param_.size();
                const char *p_ = param_.c_str();
                printf("param : %s\n",param_.c_str());

                write(input[1],p_,size_);
                char c;
                while(read(output[0],&c,1) > 0)
                {
                    rsp_text_.push_back(c);
                }
                printf("rsp_text : %s\n",rsp_text_.c_str());
                waitpid(id,NULL,0);

                close(input[1]);
                close(output[0]);

                LOG(INFO,"Make status line");
                rsp_->MakeStatusLine();
            
                rq_->SetResourceSize(rsp_text_.size());
                LOG(INFO,"Make response head");
                rsp_->MakeResponseHead(rq_);

                LOG(INFO,"Secd response");
                conn_->SendResponse(rsp_,rq_,true);
            }
        }

    static void ProcessResponse(Connect *&conn_,Request *&rq_,Response *& rsp_)
        {
            if(rq_->IsCgi())
            {
                LOG(INFO, "Response CGI");
                ProcessCgi(conn_,rq_,rsp_);
            }
            else 
            {   
                LOG(INFO, "Response without CGI");
                ProcessNoCgi(conn_,rq_,rsp_);
            }   
        }


    static void* HandlerRequest(void* arg)
        {
            int sock_ = *(int *)arg ;
            Connect *conn_ = new Connect(sock_);
            Request *rq_ = new Request();
            Response *rsp_ = new Response();

            int &state_ = rsp_->state;
            LOG(INFO,"Recv request line");

            conn_->RecvOneLine(rq_->rq_line); //获取第一行。请求行
            LOG(INFO,"Parse request line");
            rq_->ParseRequestLine();          //请求行分析
            if(!rq_->IsMethodLegal())         //产看请求方法是否支持
            { //不支持
                LOG(ERROR,"Methof is not legal");
                conn_->RecvRequestHead(rq_->rq_head);//直接接收请求报头
                state_ = BAD_REQUEST;                 //设定响应状态
                goto end;                            //转到发送阶段
            }

            LOG(INFO,"Parse url");
            rq_->ParseUri();              //解析出请求行中的uri

            if(!rq_->IsPathLegal())       //判断路径是否合法
                  //判断文件是否存在，是否cgi。访问资源类型和大小
            {
                LOG(ERROR,"Path is not legal");
                conn_->RecvRequestHead(rq_->rq_head); 
                  //有问题，粗暴接收剩下的内容
                state_ = NOT_FOUND;       //设定返回状态
                goto end;
            }

            LOG(INFO, "Recv request head");
            conn_->RecvRequestHead(rq_->rq_head);

            LOG(INFO,"Parse request head");
            if(!rq_->ParseRequestHead())
            {
                LOG(ERROR,"Request head is not legal");
                state_ = BAD_REQUEST;
                goto end;
            }

            if(rq_->IsNeedRecvText())
            {
                LOG(INFO, "Recv request text");
                conn_->RecvRequestText(rq_->rq_text,rq_->GetContentLength(),rq_->GetParam());

            }

            LOG(INFO, "Respond to the request");
            ProcessResponse(conn_,rq_,rsp_);

end:
            if(state_ != OK)
            {
                HandlerError(conn_,rq_,rsp_);
            }
            LOG(INFO,"This connection is over");
            delete conn_;
            delete rq_;
            delete rsp_;

            
        }


};



#endif
