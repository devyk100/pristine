# Pristine Reverse Proxy - Architecture Design Document

## System Overview

The Pristine Reverse Proxy is a high-performance, asynchronous reverse proxy built with C++ and Boost.Asio. This document provides comprehensive architectural diagrams and design patterns used in the system.

## 1. System Architecture Diagram

```mermaid
graph TB
    subgraph "Client Layer"
        C1[Web Browser]
        C2[Mobile App]
        C3[API Client]
    end
    
    subgraph "Pristine Reverse Proxy"
        subgraph "Network Layer"
            HTTP[HTTP Server :9080]
            HTTPS[HTTPS Server :9443]
        end
        
        subgraph "Core Components"
            RP[ReverseProxy]
            RR[RequestRouter]
            CH[ConnectionHandler]
            CM[ConfigManager]
            CertM[CertificateManager]
        end
        
        subgraph "Protocol Handlers"
            H2H[Http2Handler]
            WSH[WebSocketHandler]
            LB[LoadBalancer]
        end
        
        subgraph "Configuration"
            YAML[proxy.yaml]
            CERTS[SSL Certificates]
        end
    end
    
    subgraph "Backend Services"
        B1[Backend Server 1<br/>:3000]
        B2[API Server<br/>:8080]
        B3[WebSocket Server<br/>:9000]
    end
    
    C1 --> HTTP
    C2 --> HTTPS
    C3 --> HTTP
    
    HTTP --> RP
    HTTPS --> RP
    
    RP --> RR
    RR --> CH
    CH --> H2H
    CH --> WSH
    CH --> LB
    
    CM --> YAML
    CertM --> CERTS
    
    CH --> B1
    CH --> B2
    CH --> B3
    
    style RP fill:#e1f5fe
    style RR fill:#f3e5f5
    style CH fill:#e8f5e8
```

## 2. Class Diagram

```mermaid
classDiagram
    class ReverseProxy {
        -io_context& io_context_
        -ConfigManager config_manager_
        -RequestRouter router_
        -CertificateManager cert_manager_
        -tcp::acceptor http_acceptor_
        -tcp::acceptor https_acceptor_
        -ssl::context ssl_context_
        +ReverseProxy(config_path)
        +start()
        +stop()
        -accept_http_connections()
        -accept_https_connections()
        -handle_accept()
    }
    
    class RequestRouter {
        -std::map~string, SiteConfig~ sites_
        +RequestRouter(config)
        +route_request(host) SiteConfig
        +add_site(domain, config)
        +remove_site(domain)
    }
    
    class ConnectionHandler {
        -tcp::socket socket_
        -ssl::stream ssl_socket_
        -RequestRouter& router_
        -std::string buffer_
        +ConnectionHandler(socket, router)
        +start()
        +handle_request()
        -parse_http_request()
        -forward_to_backend()
        -send_response()
    }
    
    class ConfigManager {
        -std::string config_path_
        -ProxyConfig config_
        +ConfigManager(path)
        +load_config() ProxyConfig
        +reload_config()
        +validate_config()
    }
    
    class CertificateManager {
        -std::string cert_dir_
        -std::map~string, ssl::context~ ssl_contexts_
        +CertificateManager(cert_dir)
        +get_ssl_context(domain) ssl::context&
        +generate_self_signed(domain)
        +request_lets_encrypt(domain)
        +load_certificate(domain)
    }
    
    class Http2Handler {
        -ConnectionHandler& connection_
        +Http2Handler(connection)
        +handle_http2_request()
        +send_http2_response()
        -parse_http2_frame()
    }
    
    class WebSocketHandler {
        -ConnectionHandler& connection_
        +WebSocketHandler(connection)
        +handle_websocket_upgrade()
        +forward_websocket_data()
        -validate_websocket_headers()
    }
    
    class LoadBalancer {
        -std::vector~BackendServer~ backends_
        -BalancingStrategy strategy_
        +LoadBalancer(backends)
        +select_backend() BackendServer
        +add_backend(server)
        +remove_backend(server)
        +health_check()
    }
    
    ReverseProxy --> RequestRouter
    ReverseProxy --> ConfigManager
    ReverseProxy --> CertificateManager
    ReverseProxy --> ConnectionHandler
    ConnectionHandler --> Http2Handler
    ConnectionHandler --> WebSocketHandler
    ConnectionHandler --> LoadBalancer
    RequestRouter --> ConfigManager
```

## 3. Sequence Diagram - HTTP Request Flow

```mermaid
sequenceDiagram
    participant Client
    participant ReverseProxy
    participant RequestRouter
    participant ConnectionHandler
    participant Backend
    
    Client->>ReverseProxy: HTTP Request (Host: example.com)
    ReverseProxy->>ConnectionHandler: Create new connection
    ConnectionHandler->>ConnectionHandler: Parse HTTP headers
    ConnectionHandler->>RequestRouter: Route request by Host header
    RequestRouter->>RequestRouter: Lookup domain configuration
    RequestRouter-->>ConnectionHandler: Return backend config
    ConnectionHandler->>Backend: Forward request with headers
    Backend-->>ConnectionHandler: HTTP Response
    ConnectionHandler->>ConnectionHandler: Process response headers
    ConnectionHandler-->>Client: Forward response
    
    Note over Client,Backend: All headers preserved including WebSocket upgrade headers
```

## 4. Sequence Diagram - HTTPS with Certificate Management

```mermaid
sequenceDiagram
    participant Client
    participant ReverseProxy
    participant CertificateManager
    participant ConnectionHandler
    participant Backend
    
    Client->>ReverseProxy: HTTPS Request (SNI: example.com)
    ReverseProxy->>CertificateManager: Get SSL context for domain
    
    alt Certificate exists
        CertificateManager-->>ReverseProxy: Return SSL context
    else Certificate missing
        CertificateManager->>CertificateManager: Generate self-signed cert
        CertificateManager-->>ReverseProxy: Return new SSL context
    end
    
    ReverseProxy->>ConnectionHandler: Create SSL connection
    ConnectionHandler->>ConnectionHandler: SSL handshake
    ConnectionHandler->>ConnectionHandler: Parse HTTPS request
    ConnectionHandler->>Backend: Forward to backend
    Backend-->>ConnectionHandler: Response
    ConnectionHandler-->>Client: Encrypted response
```

## 5. State Diagram - Connection Lifecycle

```mermaid
stateDiagram-v2
    [*] --> Listening
    
    Listening --> Accepting : New connection
    Accepting --> SSL_Handshake : HTTPS connection
    Accepting --> Reading_Request : HTTP connection
    
    SSL_Handshake --> Reading_Request : Handshake success
    SSL_Handshake --> Closed : Handshake failed
    
    Reading_Request --> Parsing_Headers : Request received
    Parsing_Headers --> Routing : Headers parsed
    Routing --> Connecting_Backend : Route found
    Routing --> Error_Response : Route not found
    
    Connecting_Backend --> Forwarding_Request : Backend connected
    Connecting_Backend --> Error_Response : Backend unavailable
    
    Forwarding_Request --> Reading_Response : Request sent
    Reading_Response --> Sending_Response : Response received
    Sending_Response --> Keep_Alive : HTTP/1.1 Keep-Alive
    Sending_Response --> Closed : Connection close
    
    Keep_Alive --> Reading_Request : Next request
    Keep_Alive --> Closed : Timeout/Close
    
    Error_Response --> Closed
    Closed --> [*]
```

## 6. Component Diagram

```mermaid
graph TB
    subgraph "Pristine Reverse Proxy System"
        subgraph "Network Interface"
            HTTP_PORT[":9080<br/>HTTP Listener"]
            HTTPS_PORT[":9443<br/>HTTPS Listener"]
        end
        
        subgraph "Core Engine"
            MAIN[Main Thread<br/>Event Loop]
            WORKERS[Worker Thread Pool<br/>16 threads]
        end
        
        subgraph "Request Processing"
            PARSER[HTTP Parser]
            ROUTER[Domain Router]
            FORWARDER[Request Forwarder]
        end
        
        subgraph "Protocol Support"
            HTTP1[HTTP/1.1 Handler]
            HTTP2[HTTP/2 Handler]
            WS[WebSocket Handler]
        end
        
        subgraph "Backend Management"
            POOL[Connection Pool]
            LB[Load Balancer]
            HEALTH[Health Checker]
        end
        
        subgraph "Security & Certificates"
            SSL[SSL/TLS Engine]
            CERT_GEN[Certificate Generator]
            ACME[ACME Client]
        end
        
        subgraph "Configuration"
            YAML_CONFIG[YAML Config Loader]
            HOT_RELOAD[Hot Reload Watcher]
        end
    end
    
    HTTP_PORT --> MAIN
    HTTPS_PORT --> MAIN
    MAIN --> WORKERS
    WORKERS --> PARSER
    PARSER --> ROUTER
    ROUTER --> FORWARDER
    FORWARDER --> HTTP1
    FORWARDER --> HTTP2
    FORWARDER --> WS
    HTTP1 --> POOL
    HTTP2 --> POOL
    WS --> POOL
    POOL --> LB
    LB --> HEALTH
    HTTPS_PORT --> SSL
    SSL --> CERT_GEN
    CERT_GEN --> ACME
    YAML_CONFIG --> HOT_RELOAD
```

## 7. Data Flow Diagram

```mermaid
flowchart TD
    START([Client Request]) --> ACCEPT{Accept Connection}
    ACCEPT -->|HTTP| HTTP_PARSE[Parse HTTP Request]
    ACCEPT -->|HTTPS| SSL_HANDSHAKE[SSL Handshake]
    SSL_HANDSHAKE --> HTTP_PARSE
    
    HTTP_PARSE --> EXTRACT_HOST[Extract Host Header]
    EXTRACT_HOST --> ROUTE_LOOKUP[Domain Route Lookup]
    
    ROUTE_LOOKUP -->|Found| BACKEND_SELECT[Select Backend Server]
    ROUTE_LOOKUP -->|Not Found| ERROR_404[Return 404 Error]
    
    BACKEND_SELECT --> PROTOCOL_CHECK{Check Protocol}
    PROTOCOL_CHECK -->|HTTP/1.1| HTTP1_FORWARD[HTTP/1.1 Forward]
    PROTOCOL_CHECK -->|HTTP/2| HTTP2_FORWARD[HTTP/2 Forward]
    PROTOCOL_CHECK -->|WebSocket| WS_UPGRADE[WebSocket Upgrade]
    
    HTTP1_FORWARD --> BACKEND_CONNECT[Connect to Backend]
    HTTP2_FORWARD --> BACKEND_CONNECT
    WS_UPGRADE --> BACKEND_CONNECT
    
    BACKEND_CONNECT -->|Success| FORWARD_REQUEST[Forward Request + Headers]
    BACKEND_CONNECT -->|Failed| ERROR_502[Return 502 Error]
    
    FORWARD_REQUEST --> BACKEND_RESPONSE[Receive Backend Response]
    BACKEND_RESPONSE --> FORWARD_RESPONSE[Forward Response to Client]
    
    FORWARD_RESPONSE --> KEEP_ALIVE{Keep-Alive?}
    KEEP_ALIVE -->|Yes| HTTP_PARSE
    KEEP_ALIVE -->|No| CLOSE_CONN[Close Connection]
    
    ERROR_404 --> CLOSE_CONN
    ERROR_502 --> CLOSE_CONN
    CLOSE_CONN --> END([End])
    
    style START fill:#e1f5fe
    style END fill:#ffebee
    style ERROR_404 fill:#ffcdd2
    style ERROR_502 fill:#ffcdd2
```

## 8. Deployment Diagram

```mermaid
graph TB
    subgraph "Production Environment"
        subgraph "Load Balancer Tier"
            LB1[Pristine Proxy 1<br/>:80, :443]
            LB2[Pristine Proxy 2<br/>:80, :443]
        end
        
        subgraph "Application Tier"
            APP1[App Server 1<br/>:3000]
            APP2[App Server 2<br/>:3000]
            APP3[App Server 3<br/>:3000]
        end
        
        subgraph "API Tier"
            API1[API Server 1<br/>:8080]
            API2[API Server 2<br/>:8080]
        end
        
        subgraph "WebSocket Tier"
            WS1[WebSocket Server 1<br/>:9000]
            WS2[WebSocket Server 2<br/>:9000]
        end
        
        subgraph "Configuration"
            CONFIG[Shared Config<br/>proxy.yaml]
            CERTS[Certificate Store<br/>Let's Encrypt]
        end
    end
    
    subgraph "External"
        INTERNET[Internet Traffic]
        DNS[DNS Provider]
        ACME_SERVER[Let's Encrypt<br/>ACME Server]
    end
    
    INTERNET --> LB1
    INTERNET --> LB2
    DNS --> INTERNET
    
    LB1 --> APP1
    LB1 --> APP2
    LB1 --> APP3
    LB1 --> API1
    LB1 --> API2
    LB1 --> WS1
    LB1 --> WS2
    
    LB2 --> APP1
    LB2 --> APP2
    LB2 --> APP3
    LB2 --> API1
    LB2 --> API2
    LB2 --> WS1
    LB2 --> WS2
    
    LB1 --> CONFIG
    LB2 --> CONFIG
    LB1 --> CERTS
    LB2 --> CERTS
    
    CERTS --> ACME_SERVER
```

## 9. CRC (Class-Responsibility-Collaboration) Cards

### ReverseProxy
**Responsibilities:**
- Manage HTTP/HTTPS server lifecycle
- Coordinate all system components
- Handle incoming connections
- Manage worker thread pool

**Collaborators:**
- ConfigManager (configuration)
- RequestRouter (routing decisions)
- CertificateManager (SSL contexts)
- ConnectionHandler (request processing)

### RequestRouter
**Responsibilities:**
- Route requests based on Host header
- Maintain domain-to-backend mappings
- Validate routing rules
- Handle routing failures

**Collaborators:**
- ConfigManager (site configurations)
- ConnectionHandler (routing results)

### ConnectionHandler
**Responsibilities:**
- Handle individual client connections
- Parse HTTP requests and responses
- Forward requests to backends
- Manage connection lifecycle

**Collaborators:**
- RequestRouter (routing decisions)
- Http2Handler (HTTP/2 protocol)
- WebSocketHandler (WebSocket upgrades)
- LoadBalancer (backend selection)

### CertificateManager
**Responsibilities:**
- Generate and manage SSL certificates
- Handle Let's Encrypt ACME protocol
- Maintain SSL contexts per domain
- Certificate renewal and validation

**Collaborators:**
- ReverseProxy (SSL context provision)
- ConfigManager (certificate settings)

## 10. Performance Characteristics

```mermaid
graph LR
    subgraph "Performance Metrics"
        LATENCY[Latency: <1ms<br/>proxy overhead]
        THROUGHPUT[Throughput: 10K+ RPS<br/>per core]
        MEMORY[Memory: <50MB<br/>base footprint]
        CONNECTIONS[Concurrent: 10K+<br/>connections]
    end
    
    subgraph "Optimization Techniques"
        ASYNC[Async I/O<br/>Non-blocking]
        THREADS[Thread Pool<br/>CPU cores × 2]
        POOLING[Connection<br/>Pooling]
        ZERO_COPY[Zero-copy<br/>Forwarding]
    end
    
    ASYNC --> LATENCY
    THREADS --> THROUGHPUT
    POOLING --> MEMORY
    ZERO_COPY --> CONNECTIONS
```

This architectural design document provides a comprehensive view of the Pristine Reverse Proxy system, covering all major components, interactions, and design decisions. The diagrams use Mermaid syntax for easy rendering and maintenance.
