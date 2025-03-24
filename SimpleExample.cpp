#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <vector>

std::vector<char> vBuffer(20 * 1024);

void getSomeData(boost::asio::ip::tcp::socket &socket)
{
    socket.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), 
    [&](boost::system::error_code ec, std::size_t length){
        if(!ec){
            std::cout<<"\n\nRead: "<<length<<" bytes\n\n";
            for(const char &ch: vBuffer){
                std::cout<<ch<<" ";
            }
            std::cout<<std::endl;
            getSomeData(socket);
        }
    });
}

void printError(boost::system::error_code &ec)
{
    if (ec)
    {
        std::cerr << "Error: " << ec.message() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    boost::system::error_code ec;
    boost::asio::io_context context;

    boost::asio::io_context::work idleWork(context);
    std::thread contextThread = std::thread([&](){ context.run(); });

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1", ec), 3000);
    boost::asio::ip::tcp::socket socket(context);

    socket.connect(endpoint, ec);
    if (ec)
    {
        printError(ec);
        exit(1);
    }

    if (socket.is_open())
    {
        getSomeData(socket);
        std::string sampleRequest =
            "GET / HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Connection: close\r\n\r\n";
        socket.write_some(boost::asio::buffer(sampleRequest.data(), sampleRequest.size()), ec);
        
        sleep(1);
        context.stop();
        if(contextThread.joinable()) contextThread.join();
    }

    return 0;
}