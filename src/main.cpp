#include "ProxyServer.h"
#include "LoadBalancer.h"
#include "WebSocketHandler.h"
#include "Http2Handler.h"
#include "Utils.h"

int main() {
    // Initialize server components
    ProxyServer proxyServer;
    LoadBalancer loadBalancer;
    WebSocketHandler webSocketHandler;
    Http2Handler http2Handler;

    // Load configuration settings
    Utils::loadConfig("config/config.json");

    // Start the server loop
    proxyServer.start();
    loadBalancer.start();
    webSocketHandler.start();
    http2Handler.start();

    return 0;
}
