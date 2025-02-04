# Server-Client Mini Project
Overview

This project is a multithreaded client-server application developed in C for Linux. It facilitates communication between a Windows-based IPv4 client and an HTTP IPv6 server by efficiently handling multiple client requests in parallel.

*Features
-Multithreaded IPv4 Server (Linux)
-Listens for incoming connections from multiple clients.
-IPv4 Client (Windows-based)
-Sends commands in the format xy# to the IPv4 server.
-Receives validated requests from the IPv4 server.
-Establishes a connection to a predefined HTTP IPv6 web server.
-Sends an HTTP GET request and retrieves the webpage.
-Forwards the webpage content back to the original IPv4 client.
-Saves the retrieved webpage as an .html file in the home directory.

*Error Handling
-If an unrecognized command is received, the server responds with a Command not implemented error.

*Usage
-Run the IPv4 Server on a Linux machine.
-Start the IPv4 Client (ClientIPv4_win_2024.exe) on a Windows machine and send a command.
-The server will process the request, forward it to the IPv6 client, and retrieve the webpage.
-The retrieved webpage is sent back to the IPv4 client and saved locally.
-This project demonstrates efficient cross-protocol communication and parallel request handling, ensuring smooth interaction between IPv4 and IPv6 networks.

