#include <iostream>
#include <boost/asio.hpp>

void on_accept(std::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec) {
    if (!ec) {
        std::cout << "Async: Connection accepted from "
                  << sock->remote_endpoint().address().to_string()
                  << ":" << sock->remote_endpoint().port() << std::endl;
        // Here you'd start async_read or something else
    } else {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    }
}

int main() {
    boost::asio::io_context ioc;
    boost::system::error_code ec;

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1", ec), 3490);
    boost::asio::ip::tcp::acceptor acceptor(ioc);

    acceptor.open(endpoint.protocol(), ec);
    acceptor.bind(endpoint, ec);
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

    if (ec) {
        std::cerr << "Error setting up acceptor: " << ec.message() << std::endl;
        return 1;
    }

    std::cout << "Async listening on 127.0.0.1:3490..." << std::endl;

    std::function<void()> start_accept;

    start_accept = [&]() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioc);

        acceptor.async_accept(
            *socket,
            [socket, &start_accept](const boost::system::error_code& ec) {
                on_accept(socket, ec);
                start_accept(); // accept next connection
            }
        );
    };

    start_accept();

    ioc.run();

    return 0;
}
