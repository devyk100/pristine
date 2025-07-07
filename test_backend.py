#!/usr/bin/env python3
import http.server
import socketserver
import sys

class TestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        
        response = f"""
        <html>
        <head><title>Test Backend Server</title></head>
        <body>
            <h1>Backend Server Response</h1>
            <p>This is a test backend server running on port {PORT}</p>
            <p>Request path: {self.path}</p>
            <p>Headers received:</p>
            <ul>
        """
        
        for header, value in self.headers.items():
            response += f"<li><strong>{header}:</strong> {value}</li>"
        
        response += """
            </ul>
        </body>
        </html>
        """
        
        self.wfile.write(response.encode())

if __name__ == "__main__":
    PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
    
    with socketserver.TCPServer(("", PORT), TestHandler) as httpd:
        print(f"Test backend server running on port {PORT}")
        print(f"Visit http://localhost:{PORT} to test")
        httpd.serve_forever()
