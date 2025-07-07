# Pristine - High-Performance Reverse Proxy

A lightweight, high-performance reverse proxy built with C++ using Boost.Asio for async I/O operations. Designed to be a simple alternative to Caddy with automatic certificate management, HTTP/1.1 and HTTP/2 support, and WebSocket proxying.

## Features

- ✅ **High Performance**: Built with Boost.Asio for async I/O and multi-threading
- ✅ **HTTP/1.1 Support**: Full HTTP/1.1 proxying with header preservation
- ✅ **Domain-based Routing**: Route requests based on Host header
- ✅ **YAML Configuration**: Easy-to-read YAML configuration files
- ✅ **Automatic Certificate Management**: Self-signed certificates with Let's Encrypt support
- ✅ **WebSocket Support**: Dedicated WebSocket handling
- ✅ **Header Preservation**: All headers are passed through to backends
- 🚧 **HTTP/2 Support**: Implemented but needs testing
- 🚧 **HTTPS/TLS**: Basic implementation (needs debugging)

## Architecture

The reverse proxy consists of several key components:

### Core Components

- **ReverseProxy**: Main orchestrator that manages HTTP/HTTPS servers
- **RequestRouter**: Routes incoming requests to appropriate backends based on domain
- **ConnectionHandler**: Handles individual client connections and request forwarding
- **ConfigManager**: Loads and manages YAML configuration
- **CertificateManager**: Manages SSL certificates (self-signed and Let's Encrypt)

### Protocol Handlers

- **Http2Handler**: Handles HTTP/2 protocol specifics
- **WebSocketHandler**: Manages WebSocket connections and upgrades
- **LoadBalancer**: Distributes requests across multiple backend servers

## Configuration

Create a `config/proxy.yaml` file:

```yaml
# Reverse Proxy Configuration
http_port: 9080
https_port: 9443
email: "admin@example.com"  # Email for Let's Encrypt registration

# Global settings
timeout_seconds: 30
max_connections: 1000

# Site configurations
sites:
  - domain: "example.com"
    backend: "127.0.0.1:3000"
    tls: auto  # auto, manual, or off
  - domain: "api.example.com"
    backend: "127.0.0.1:8080"
    tls: auto
  - domain: "ws.example.com"
    backend: "127.0.0.1:9000"
    tls: auto
    websocket: true

# Certificate storage
cert_dir: "./certs"
acme_server: "https://acme-v02.api.letsencrypt.org/directory"  # Let's Encrypt production
# acme_server: "https://acme-staging-v02.api.letsencrypt.org/directory"  # Let's Encrypt staging
```

## Building

### Prerequisites

- C++17 compatible compiler (GCC 8+ or Clang 7+)
- CMake 3.15+
- Boost libraries (system, thread, filesystem)
- OpenSSL
- yaml-cpp

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake libboost-all-dev libssl-dev libyaml-cpp-dev
```

### Build Steps

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

### Starting the Reverse Proxy

```bash
cd build
./ReverseProxy ../config/proxy.yaml
```

### Testing

1. **Start a backend server** (example using Python):
```bash
python3 -m http.server 3000
```

2. **Test HTTP proxying**:
```bash
curl -H "Host: example.com" http://localhost:9080/
```

3. **Test with custom paths and parameters**:
```bash
curl -H "Host: example.com" http://localhost:9080/api/users?id=123
```

## Performance Features

- **Async I/O**: Non-blocking operations using Boost.Asio
- **Multi-threading**: Configurable worker thread pool
- **Connection Pooling**: Efficient backend connection management
- **Header Optimization**: Minimal header processing overhead

## Security Features

- **Automatic HTTPS**: Self-signed certificates generated automatically
- **Let's Encrypt Integration**: Production-ready certificate management
- **Secure Defaults**: TLS 1.2+ with strong cipher suites

## Monitoring and Logging

The proxy provides detailed logging for:
- Request routing decisions
- Backend connection status
- Certificate management events
- Error conditions and debugging

## Comparison with Caddy

| Feature | Pristine | Caddy |
|---------|----------|-------|
| Language | C++ | Go |
| Performance | High (native) | High |
| Memory Usage | Low | Medium |
| Configuration | YAML | Caddyfile |
| Auto HTTPS | ✅ | ✅ |
| HTTP/2 | ✅ | ✅ |
| WebSockets | ✅ | ✅ |
| Plugin System | ❌ | ✅ |
| Admin API | ❌ | ✅ |

## Development Status

### Working Features
- ✅ HTTP/1.1 reverse proxying
- ✅ Domain-based routing
- ✅ Header preservation
- ✅ YAML configuration loading
- ✅ Self-signed certificate generation
- ✅ Multi-threaded async I/O

### In Progress
- 🚧 HTTPS/TLS stability improvements
- 🚧 HTTP/2 testing and validation
- 🚧 Let's Encrypt ACME protocol implementation
- 🚧 WebSocket upgrade handling

### Planned Features
- 📋 Load balancing algorithms (round-robin, least-connections)
- 📋 Health checks for backend servers
- 📋 Rate limiting and throttling
- 📋 Access logging and metrics
- 📋 Configuration hot-reloading

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is open source. See LICENSE file for details.

## Troubleshooting

### Common Issues

1. **Permission denied on ports 80/443**: Use non-privileged ports (8080/8443) for testing
2. **Backend connection refused**: Ensure backend servers are running and accessible
3. **Certificate errors**: Check certificate directory permissions and disk space

### Debug Mode

Enable verbose logging by setting environment variable:
```bash
export PRISTINE_LOG_LEVEL=debug
./ReverseProxy config.yaml
```

## Examples

See the `examples/` directory for:
- Sample configurations
- Backend server implementations
- Load testing scripts
- Docker deployment examples
