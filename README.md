# Reverse Proxy and Load Balancer

This project is a lightweight reverse proxy and load balancer built using C++20, Boost Asio, and CMake. It supports WebSockets, HTTP/2, and can be configured to route traffic to different servers based on subdomains or IP addresses.

## Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/reverse-proxy-load-balancer.git
   cd reverse-proxy-load-balancer
   ```

2. **Build the project:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

To start the server, run the following command from the build directory:
```bash
./ReverseProxyLoadBalancer
```

## Features

- Reverse proxy to local and external servers
- Load balancing with round-robin and least connections algorithms
- WebSocket and HTTP/2 support
- Configurable via JSON

## Configuration

Edit the `config/config.json` file to set up server routes and load balancing rules.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License.

## Contact

For questions or support, please contact [your email].
