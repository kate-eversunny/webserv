# webserv

An HTTP server that can respond to GET, HEAD, POST, PUT and DELETE methods.

To clone and compile code:
1. git clone https://github.com/kate-eversunny/webserv.git webserv
2. cd webserv
3. make

To launch server with a website:
1.  ./webserv
2. open a web browser and go to 127.0.0.1:8080

To launch server for testing:
1.  ./webserv ./config/tests.conf
2. in a new tab of terminal: ./tester http://127.0.0.1:8080


Contributors (you can see all commits on original page of the project: https://github.com/gbroccol/HTTP_server):
- https://github.com/gbroccol - requests parsing, DELETE request handling, POST request with python CGI script handling, html pages etc.
- https://github.com/helenSsnow - config parsing, autoindex pages creating, request path parsing etc.
- me - connection establishment, reading from/writing to sockets, GET and PUT requests handling, POST with cgi-tester (Go script) handling.

