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
void networkInstance::syncDTServerUDP(boost::asio::io_context& dt_io)
{
    using boost::asio::ip::udp;
    try
    {
        //initialise socket and bind to local endpoint
        udp::endpoint localEndpoint(udp::v4(), serverPort);
        socket_.open(udp::v4()); 
        socket_.bind(localEndpoint);

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
            boost::bind(&handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

        // for (;;)
        // {
        //     std::cout<<"Enter message: "<<std::endl;
        //     std::getline(std::cin, message);
        //     //send to client
        //     socket_.send_to(boost::asio::buffer(message), remote_endpoint, 0, ignored_error);//0 is the flags
        // }
    }

    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr<<"ERROR SOMETHING BAD HAPPENED IN ASIO_TEST_CPP"<<std::endl;
    }
}


void networkInstance::syncDTClientUDP(boost::asio::io_context& dt_io)
{
    using boost::asio::ip::udp;
    try
    {
        //initialise socket and bind to local endpoint
        udp::endpoint localEndpoint(udp::v4(), clientPort);
        socket_.open(udp::v4()); 
        socket_.bind(localEndpoint);        
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

        //start message receiving process
        //async receive from
        socket_.async_receive_from(//receive contact and store endpoint information in the remote_endpoint
            boost::asio::buffer(*recv_buf), 
            remote_endpoint,
            [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
                networkInstance::handleReceive(error, bytesTransferred);
            });
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}

//reception handler for async receive
void networkInstance::handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
{
    if(!error)
    {
        //prints out message
        std::string receivedData;
        //assign string from buffer
        receivedData.assign(recv_buf->data(), bytesTransferred);
        //lock
        std::unique_lock<std::mutex> lock(queueMutex);
        receivedDataQueue.push_back(receivedData);
        lock.unlock();

        //clear the buffer
        recv_buf->fill(0);

        //async receive again
        socket_.async_receive_from(//receive contact and store endpoint information in the remote_endpoint
            boost::asio::buffer(*recv_buf), 
            remote_endpoint,
            boost::bind(&handleReceive, 
                        this, 
                        boost::asio::placeholders::error, 
                        boost::asio::placeholders::bytes_transferred)
        );

    }else{
        std::cout<<"ERROR: ";
        std::cout<<error.message()<<std::endl;
    }
}

//function to automatically form and serialise the header and serialise the body and send to the recipient
void networkInstance::sendMessage(std::string header, std::string body)
{
    boost::system::error_code ignored_error;
    std::string msg{header + body};
    std::cout<<"SENDING BODY: "<<body<<std::endl;
    this->socket_.send_to(boost::asio::buffer(header + body), this->remote_endpoint, 0, ignored_error);
}