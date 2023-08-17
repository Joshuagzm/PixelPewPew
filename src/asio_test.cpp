#include "include/asio_test.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2){
            std::cerr << "Usage client <host> " << std::endl;
        }else{
            boost::asio::io_context io_context;
            tcp::resolver resolver(io_context);
            
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}