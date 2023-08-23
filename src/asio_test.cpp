#include "include/asio_test.h"

//recurringTimer constructer
recurringTimer::recurringTimer(boost::asio::io_context& io)
    :   strand_(boost::asio::make_strand(io)),
        timer_(io, boost::asio::chrono::seconds(duration_)),
        timer2_(io, boost::asio::chrono::seconds(duration_))
{
    std::cout<<"Timer initialised, async wait...\n";
    this->timer_.async_wait(boost::asio::bind_executor(strand_,boost::bind(printMsg, this, 9)));//call new async wait binding arguments to the completion handler (like a callback)
    this->timer2_.async_wait(boost::asio::bind_executor(strand_,boost::bind(printMsg2, this, 9)));//timer 2
}

recurringTimer::~recurringTimer(){
    std::cout<<"Timer Destroyed"<<std::endl;
}


//completion handler
void recurringTimer::printMsg(int orange)
{
    orange += 1;
    std::cout<<orange<<std::endl;
    if(count_ < 10){
        ++(count_);
        std::cout<<"TIMERS ELAPSED: "<<count_<<std::endl;
        this->timer_.expires_at(this->timer_.expiry() + boost::asio::chrono::seconds(duration_));
        this->timer_.async_wait(boost::asio::bind_executor(strand_,boost::bind(printMsg, this, 9)));//call new async wait binding arguments to the completion handler (like a callback)
    }else{
        std::cout<<"TIMER FINISHED"<<std::endl;
    }
}

//completion handler
void recurringTimer::printMsg2(int orange)
{
    orange += 1;
    std::cout<<orange<<std::endl;
    if(count_ < 10){
        ++(count_);
        std::cout<<"TIMERS ELAPSED: "<<count_<<std::endl;
        this->timer2_.expires_at(this->timer2_.expiry() + boost::asio::chrono::seconds(duration_));
        this->timer2_.async_wait(boost::asio::bind_executor(strand_,boost::bind(printMsg2, this, 9)));//call new async wait binding arguments to the completion handler (like a callback)
    }else{
        std::cout<<"TIMER FINISHED"<<std::endl;
    }
}

int asio_timer_test()
{

    // io.run();
    return 0;
}

//synchronous TCP daytime server
void syncDTServerTCP()
{
    using boost::asio::ip::tcp;
    try
    {
        boost::asio::io_context dt_io;
        tcp::acceptor acceptor(dt_io, tcp::endpoint(tcp::v4(), 13));//a tcp acceptor in io context dt_io with an endpoint using ipv4 on port 13

        for(;;)//essentiall while(true);
        {
            tcp::socket socket(dt_io);
            acceptor.accept(socket);//wait for connection on socket in io context (blocking)
            std::string message = makeDaytimeString();//generate message
            boost::system::error_code ignored_error;//represent error
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);//write to buffer
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

//synchronous UDP daytime server
void syncDTServerUDP()
{
    using boost::asio::ip::udp;
    try
    {
        boost::asio::io_context dt_io;
        udp::socket socket(dt_io, udp::endpoint(udp::v4(), 13));//create udp socket 
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');//clear buffer
        for (;;)
        {
            boost::array<char, 128> recv_buf;//wait for the client to make contact
            udp::endpoint remote_endpoint;//client endpoint
            size_t len = socket.receive_from(//receive contact and store endpoint information in the remote_endpoint
                boost::asio::buffer(recv_buf), remote_endpoint);
            std::cout<<"Received:"<<std::endl;
            std::cout.write(recv_buf.data(), len);
            std::cout<<std::endl;
            std::string message = makeDaytimeString();
            std::cout<<"Enter message: "<<std::endl;
            std::getline(std::cin, message);
            //send to client
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);//0 is the flags
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

}

void syncDTClientUDP()
{
    using boost::asio::ip::udp;
    try
    {
        boost::asio::io_context dt_io;

        udp::resolver resolver(dt_io);//the resolver finds the remote endpoint based on host and service names
        //convert IP address and port from string to correct type
        std::string ipAddrStr{""};
        unsigned short port {13};
        std::cout<<"Enter server ip address:"<<std::endl;
        std::cin >> ipAddrStr;
        boost::asio::ip::address ipAddress = boost::asio::ip::address::from_string(ipAddrStr);
        udp::endpoint receiver_endpoint(ipAddress, port);//restrict to ipv4
        udp::socket socket(dt_io);
        socket.open(udp::v4());//create a socket with our ipv4 information
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');//clear buffer
        for(;;){
            std::string message{""};
            std::cout<<"Enter message: "<<std::endl;
            std::getline(std::cin, message);
            // boost::array<char, 128> send_buf = {{0}};//dummy message to make contact with the server
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer(message), receiver_endpoint, 0, ignored_error);//send to server
            //get ready to receive from the server 
            boost::array<char, 128> recv_buf;
            //create and initialise endpoint using receive_from
            udp::endpoint sender_endpoint;
            size_t len = socket.receive_from(
                boost::asio::buffer(recv_buf), sender_endpoint
            );
            std::cout<<"Received:"<<std::endl;
            std::cout.write(recv_buf.data(), len);
            std::cout<<std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

std::string makeDaytimeString()
{
    std::time_t now = std::time(0);
    return std::ctime(&now);
}