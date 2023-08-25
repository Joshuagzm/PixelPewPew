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

//synchronous UDP daytime server
boost::asio::ip::udp::endpoint networkInstance::syncDTServerUDP(boost::asio::io_context& dt_io, std::string& latestMessage)
{
    using boost::asio::ip::udp;
    try
    {
        //initialise socket and bind to local endpoint
        udp::endpoint localEndpoint(udp::v4(), serverPort);
        socket_.open(udp::v4()); 
        socket_.bind(localEndpoint);

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');//clear buffer
        udp::endpoint remote_endpoint;//client endpoint

        std::cout<<"Waiting for client connection..."<<std::endl;
        //wait for connection
        socket_.receive_from(//receive first handshake and client contact information
                boost::asio::buffer(*recv_buf), remote_endpoint);

        //send response
        std::string message{""};
        boost::system::error_code ignored_error;
        socket_.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);//0 is the flags

        //receive confirmation of receipt
        socket_.receive_from(//receive first handshake and client contact information
                boost::asio::buffer(*recv_buf), remote_endpoint);

        //connection complete
        std::cout<<"Client connected!"<<std::endl;

        //async receive from
        socket_.async_receive_from(//receive contact and store endpoint information in the remote_endpoint
            boost::asio::buffer(*recv_buf), remote_endpoint,
            boost::bind(&handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, latestMessage));

        std::cout<<"Async receive task allocated"<<std::endl;

        for (;;)
        {
            std::cout<<"Enter message: "<<std::endl;
            std::getline(std::cin, message);
            //send to client
            socket_.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);//0 is the flags
        }
        return remote_endpoint;

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    std::cerr<<"ERROR SOMETHING BAD HAPPENED IN ASIO_TEST_CPP"<<std::endl;
    udp::endpoint dummyEP;
    return dummyEP;
}


boost::asio::ip::udp::endpoint networkInstance::syncDTClientUDP(boost::asio::io_context& dt_io, std::string& latestMessage)
{
    using boost::asio::ip::udp;
    try
    {
        //initialise socket and bind to local endpoint
        udp::endpoint localEndpoint(udp::v4(), clientPort);
        socket_.open(udp::v4()); 
        socket_.bind(localEndpoint);

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');//clear buffer
        
        //three-way handshake
        // boost::array<char, 128> send_buf = {{0}};//dummy message to make contact with the server
        std::cout<<"Connecting..."<<std::endl;
        std::string message{""};
        boost::system::error_code ignored_error;
        socket_.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);//send handshake to server
        //wait for response
        socket_.receive_from(boost::asio::buffer(*recv_buf), remote_endpoint);
        //send confirmation of receipt
        socket_.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);
        std::cout<<"Connected!"<<std::endl;

        bool DEBUGSOCKET = socket_.is_open();
        if(DEBUGSOCKET){
            std::cout<<"SOCKET IS OK AT CLIENT OPEN"<<std::endl;
        }

        //start message receiving process
        //async receive from
        socket_.async_receive_from(//receive contact and store endpoint information in the remote_endpoint
            boost::asio::buffer(*recv_buf), 
            remote_endpoint,
            // [this, &socket, &receiver_endpoint, &latestMessage](const boost::system::error_code& error, std::size_t bytesTransferred) {
            //     networkInstance::handleReceive(error, bytesTransferred, receiver_endpoint, recv_buf, &socket, latestMessage);
            // });
            boost::bind(&handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, latestMessage));

        std::cout<<"Async receive task allocated"<<std::endl;

        return remote_endpoint;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cerr<<"ERROR SOMETHING BAD HAPPENED IN ASIO_TEST_CPP"<<std::endl;
    udp::endpoint dummyEP;
    return dummyEP;
}

//reception handler for async receive
void networkInstance::handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred, std::string& latestMessage)
{
    //DEBUG
    bool isSocketOpen = socket_.is_open();
    if(isSocketOpen){std::cout<<"SOCKET IS OK"<<std::endl;}
    std::cout<<"RECEIVE HANDLER CALLED"<<std::endl;
    if(!error)
    {
        //print out message
        std::cout<<"Received: "<<bytesTransferred<<" many bytes... " << std::endl;

        //reassigns latestMessage
        latestMessage.assign(recv_buf->data(), bytesTransferred);

        //prints out message
        std::cout.write(recv_buf->data(), bytesTransferred);
        std::cout<<std::endl;

        //async receive again
        socket_.async_receive_from(//receive contact and store endpoint information in the remote_endpoint
            boost::asio::buffer(*recv_buf), 
            remote_endpoint,
            boost::bind(&handleReceive, 
                        this, 
                        boost::asio::placeholders::error, 
                        boost::asio::placeholders::bytes_transferred, 
                        latestMessage)
        );

    }else{
        std::cout<<"ERROR: ";
        std::cout<<error.message()<<std::endl;
    }
}

void sendMessageLoop()
{

}
