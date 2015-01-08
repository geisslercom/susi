#include "apiserver/TCPClient.h"



Susi::Api::TCPClient::TCPClient( std::string _address ) : address{_address} {
    startRunloop();
}

Susi::Api::TCPClient::TCPClient( TCPClient && other ) {
    isClosed.store(other.isClosed.load());
    std::swap( address,other.address );
    std::swap( sock,other.sock );
    std::swap( runloop,other.runloop );
}


void Susi::Api::TCPClient::close() {
    if( !isClosed.load() ) {
        isClosed.store(true);
        try {
            ::close(sock);
            join();
            onClose();
        }
        catch( const std::exception & e ) {
            //std::cout<<"error while closing/joining: "<<e.what()<<std::endl;
            onClose();
        }
    }
}
void Susi::Api::TCPClient::send( std::string msg ) {
    write(sock,msg.c_str(),msg.size());
}
void Susi::Api::TCPClient::join() {
    if(runloop.joinable())runloop.join();
}
Susi::Api::TCPClient::~TCPClient() {
    close();
    join();
}

void Susi::Api::TCPClient::startRunloop(){
    
    runloop = std::move( std::thread{
        [this]() {
            auto pos = address.find_last_of(':');
            if(pos == std::string::npos){
                throw std::runtime_error{"malformed address"};
            }
            std::string host = address.substr(0,pos);
            std::string port = address.substr(pos+1);
            struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
            struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
            memset(&host_info, 0, sizeof host_info);
            host_info.ai_family = AF_UNSPEC;     // IP version not specified. Can be both.
            host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
            int status = getaddrinfo(host.c_str(), port.c_str(), &host_info, &host_info_list);
            if (status != 0)  throw std::runtime_error{gai_strerror(status)};
            
            sock = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
            if (sock == -1)  throw std::runtime_error{"can't create socket"};
            status = connect(sock, host_info_list->ai_addr, host_info_list->ai_addrlen);
            if (status == -1) throw std::runtime_error{"connect error"};
            
            struct timeval timeout;      
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;
            if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                    sizeof(timeout)) < 0)
                LOG(ERROR) << "setsockopt failed";
            if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                    sizeof(timeout)) < 0)
                LOG(ERROR) << "setsockopt failed";
            onConnect();

            char buff[1024];
            int bs;
            while( !isClosed.load() ) { // data chunk loop
                try {
                    ////std::cout<<"wait for bytes"<<std::endl;
                    bs = read(sock,buff,1024);
                    if(errno == EAGAIN){
                        continue;
                    }
                    ////std::cout<<"got "<<bs<<std::endl;
                    if( bs<=0 ) {
                        throw std::runtime_error{"got zero bytes -> connection reset by peer"};
                    }
                    std::string data {buff,static_cast<size_t>( bs )};
                    onData( data );
                }
                catch( const std::exception & e ) {
                    LOG(DEBUG)<<"Exception in receive loop: "<<e.what();
                    size_t retryCount = 0;
                    bool success = false;
                    while(!isClosed.load() && retryCount < maxReconnectCount){
                        LOG(DEBUG)<<"try reconnect... to "<<host<<" : "<<port ;
                        try{
                            retryCount++;
                            ::close(sock);
                            sock = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
                            if (sock == -1)  throw std::runtime_error{"can't create socket"};
                            status = connect(sock, host_info_list->ai_addr, host_info_list->ai_addrlen);
                            if (status == -1) throw std::runtime_error{"connect error"};
                            onReconnect();
                            success = true;
                            if(sendbuff != ""){
                                send(sendbuff);
                                sendbuff = "";
                            }
                            break;
                        }catch(const std::exception & e){
                            //std::cout<<"Error in reconnect: "<<e.what()<<std::endl;
                        }               
                        std::this_thread::sleep_for(std::chrono::milliseconds{500});
                    }
                    if(!success){
                        onClose();
                        break;
                    }else{
                        LOG(DEBUG) << "reconnected tcp client...";
                    }
                }
            }
        }
    } );
}
