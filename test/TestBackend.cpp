#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

void handle_session(tcp::socket socket) {
    try {
        boost::beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "CppTestBackend");
        res.set(http::field::content_type, "text/plain");
        res.body() = "OK from C++ backend";
        res.prepare_payload();

        http::write(socket, res);
        socket.shutdown(tcp::socket::shutdown_send);
    } catch (const std::exception& e) {
        std::cerr << "Backend session error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    unsigned short port = 9999;
    if (argc >= 2) {
        port = static_cast<unsigned short>(std::stoi(argv[1]));
    }

    try {
        boost::asio::io_context ioc;
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), port)};
        std::cout << "TestBackend listening on port " << port << std::endl;

        for (;;) {
            tcp::socket socket{ioc};
            boost::system::error_code ec;
            acceptor.accept(socket, ec);
            if (ec) {
                if (ec == boost::asio::error::operation_aborted) {
                    break;
                }
                std::cerr << "Backend accept error: " << ec.message() << std::endl;
                continue;
            }
            std::thread(handle_session, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "TestBackend fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

