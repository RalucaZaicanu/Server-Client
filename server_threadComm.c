// Include necessary libraries for socket programming, multithreading, and system operations
#include <sys/types.h>        // Defines data types for sockets
#include <sys/socket.h>       // Socket functions (socket, bind, listen, etc.)
#include <netdb.h>            // Functions and constants for network address translation
#include <stdio.h>            // Standard I/O functions
#include <stdlib.h>           // Memory allocation, process control, etc.
#include <string.h>           // String manipulation functions
#include <arpa/inet.h>        // Internet address manipulation functions
#include <unistd.h>           // UNIX standard functions (close, etc.)
#include <signal.h>           // Signal handling (SIGINT, etc.)
#include <pthread.h>          // POSIX thread library

// Macro definitions
#define MSG "GET / HTTP/1.0\r\n\r\n"  // HTTP GET request message
#define FILENAME "index.html"        // File to save the HTTP response
#define MAX_THREADS 5                // Maximum number of concurrent client threads

// Global variables
volatile int running = 1;  // Flag to indicate whether the server is running
int server_fd = -1;        // Global server file descriptor
int client_fd = -1;        // Global client file descriptor
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread safety
int active_threads = 0;    // Counter for the number of active threads (clients)

// Function: sendMsg
// Sends a message over a socket.
// Parameters:
// - socket: The socket descriptor to send the message on
// - msg: The message to send
int sendMsg(int socket, char *msg) {
    int length_msg = strlen(msg);  // Calculate the message length
    printf("Attempting to send\n");

    // Attempt to send the message
    if (send(socket, msg, length_msg, 0) == -1) {
        return -1;  // Return error if sending fails
    } else {
        printf("Message was sent\n");
        return 0;   // Return success
    }
}

// Function: receiveMsg
// Receives data from a socket and saves it to a file.
// Parameters:
// - socket_client: The socket descriptor to receive data from
// - file: The name of the file to save the received data
int receiveMsg(int socket_client, char *file) {
    int length_data;                     // Number of bytes received
    char data_recieve[2000];             // Buffer to store received data
    FILE *webPage;                       // File pointer to save the data
    int nr_recv = 0;                     // Counter for received data blocks

    webPage = fopen(file, "w");          // Open the file for writing
    do {
        // Receive data from the socket
        length_data = recv(socket_client, data_recieve, 2000, 0);
        if (length_data == -1) {
            perror("Error receiving data\n");
            return -1;  // Return error if receiving fails
        }

        nr_recv++;  // Increment received block counter
        fprintf(stderr, "Data received %d\n", nr_recv);

        // Write received data to the file
        fwrite(data_recieve, length_data, 1, webPage);
    } while (length_data != 0);  // Stop when no more data is received

    fclose(webPage);  // Close the file
    return 0;         // Return success
}

// Function: handle_sigint
// Handles the SIGINT (Ctrl+C) signal for graceful shutdown.
// Parameters:
// - sig: The signal number
void handle_sigint(int sig) {
    if (server_fd != -1) {
        printf("\nCaught SIGINT (Ctrl+C). Stopping the server...\n");
        running = 0;          // Stop the server loop
        close(client_fd);     // Close the server socket
	close(server_fd);     // Close the server socket
	 exit(-1);             // Exit with error code
    }
}

// Function: initialize_server_socket
// Initializes the server socket.
// Returns:
// - The server socket descriptor, or -1 on failure
int initialize_server_socket() {
    int server_fd;
    int enable_reuse = 1;  // Option to allow socket address reuse

    server_fd = socket(AF_INET, SOCK_STREAM, 0);  // Create a TCP socket
    if (server_fd == -1) {
        perror("Failed to create socket! \n");
        return -1;  // Return error if socket creation fails
    }

    puts("Socket created successfully!");

    // Set socket options for address reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(int)) == -1) {
        perror("Error setting socket options! \n");
        return -1;  // Return error if setting options fails
    }

    return server_fd;  // Return the server socket descriptor
}

// Function: configure_server_address
// Configures the server address and binds it to the socket.
// Parameters:
// - socket_fd: The server socket descriptor
// - port: The port number to bind the socket to
void configure_server_address(int socket_fd, int port) {
    struct sockaddr_in server_address;   // Struct to store server address details
    server_address.sin_family = AF_INET;          // Use IPv4
    server_address.sin_port = htons(port);        // Set the port number
    server_address.sin_addr.s_addr = INADDR_ANY;  // Accept connections on any IP address

    // Bind the socket to the server address
    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Binding failed! \n");
    } else {
        printf("Successfully bound to port %d \n", port);
    }
}

// Function: enable_listening
// Enables the server to listen for incoming connections.
// Parameters:
// - socket_fd: The server socket descriptor
// - backlog: The maximum number of pending connections
void enable_listening(int socket_fd, int backlog) {
    if (listen(socket_fd, backlog) == -1) {
        perror("Error while waiting for connections! \n");
        exit(EXIT_FAILURE);  // Exit if listening fails
    } else {
        puts("\nWaiting for incoming connections...\n");
    }
}

// Function: handle_client
// Handles communication with a client in a separate thread.
// Parameters:
// - arg: A pointer to the client socket descriptor (passed as a void pointer)
void *handle_client(void *arg) {
    int client_fd = *((int *)arg);  // Extract client socket descriptor
    free(arg);  // Free allocated memory for the client_fd

    int retval = -1;  // Return value for socket operations
    char *mesaj = "Comanda neimplementata\n";  // Message for unimplemented commands
    char *mesajok = "\nComanda implementata\n"; // Message for implemented commands
    char msg_corect[10];  // Correct command string
    strcpy(msg_corect, "10#");  // Assign the correct command value

    char buffer_recv[100];  // Buffer to store received data
    int data_recv = 0;  // Number of bytes received
    char buffer[1000];  // Buffer for reading HTML content
    struct sockaddr_in6 server_addr;  // Struct for HTTP request server address

    server_addr.sin6_family = AF_INET6;               // Use IPv6
    inet_pton(AF_INET6, "::1", &server_addr.sin6_addr); // Set IPv6 address (::1 for localhost)
//   inet_pton(AF_INET6, "2a03:2880:f123:83:face:b00c:0:25de", &server_addr.sin6_addr); // Set IPv6 address (::1 for localhost)
    server_addr.sin6_port = htons(80);                // Set HTTP port (80)

    // Handle the client communication
    do {
        printf("Sending data\n");
        memset(buffer_recv, 0, sizeof(buffer_recv));  // Clear the receive buffer
        data_recv = recv(client_fd, buffer_recv, sizeof(buffer_recv), 0); // Receive data
        printf("Received command: %s\n", buffer_recv);

        if (data_recv == 0) {
            break;  // Client closed connection
        }

        // Check if the received command matches the correct command
        if (strncmp(msg_corect, buffer_recv, strlen(msg_corect)) == 0) {
            printf("Command implemented\n");
            send(client_fd, mesajok, strlen(mesajok), 0);  // Send confirmation

            int conn_socket = socket(AF_INET6, SOCK_STREAM, 0);  // Create a socket for HTTP request
            if (conn_socket == -1) {
                perror("socket() failed");
                return NULL;  // Return error
            }

            // Connect to the server (e.g., localhost)
            retval = connect(conn_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (retval == -1) {
                perror("connect() failed");
                return NULL;  // Return error
            } else {
                puts("Connected");
            }

            // Send HTTP GET message
            int r = sendMsg(conn_socket, MSG);
            if (r == -1) {
                printf("Message not sent\n");
                return NULL;  // Return error
            }

            // Receive HTTP response and save it to a file
            int a = receiveMsg(conn_socket, FILENAME);
            close(conn_socket);  // Close the connection to the HTTP server

            if (a == -1) {
                printf("Message not received\n");
                return NULL;  // Return error
            }

            FILE *html_file = fopen("index.html", "r");  // Open the saved HTML file
            char *header =
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n";  // HTTP response header

            send(client_fd, header, strlen(header), 0);  // Send HTTP header to the client

            // Send the HTML file content to the client
            while (fgets(buffer, 1000, html_file) != NULL) {
                send(client_fd, buffer, strlen(buffer), 0);
            }

            fclose(html_file);  // Close the HTML file

        } else {
            printf("Command not implemented\n");
            send(client_fd, mesaj, strlen(mesaj), 0);  // Send unimplemented command message
        }

    } while (data_recv > 0);  // Continue receiving data while valid

    close(client_fd);  // Close the client connection
    pthread_mutex_lock(&thread_mutex);  // Lock the mutex
    active_threads--;  // Decrease the active thread count
    pthread_mutex_unlock(&thread_mutex);  // Unlock the mutex

    printf("Thread finished, connection closed\n");
    return NULL;  // Exit the thread
}

// Main function: Entry point of the program
int main() {
    int port = 22011;  // Define the server port
    server_fd = initialize_server_socket();  // Initialize the server socket
    if (server_fd == -1) {
        return -1;  // Exit if socket initialization fails
    }

    signal(SIGINT, handle_sigint);  // Register the SIGINT signal handler

    configure_server_address(server_fd, port);  // Configure the server address
    enable_listening(server_fd, 5);  // Enable listening with a backlog of 5
    printf("Server is listening on port %d\n", port);

    // Main loop to accept and handle client connections
    while (running) {
        struct sockaddr_storage client_info;  // Struct to store client information
        socklen_t client_size = sizeof(client_info);  // Size of client_info
        int *client_fd_ptr = malloc(sizeof(int));  // Allocate memory for client_fd

        *client_fd_ptr = accept(server_fd, (struct sockaddr *)&client_info, &client_size);
        if (*client_fd_ptr == -1) {
            perror(" -> Connection was NOT accepted! \n");
            free(client_fd_ptr);  // Free memory on failure
            continue;  // Skip to the next iteration
        } else {
            puts(" -> Connection accepted!");
            send(*client_fd_ptr, "Connection accepted! \n", 100, 0);  // Notify the client

            pthread_mutex_lock(&thread_mutex);  // Lock the mutex
            if (active_threads < MAX_THREADS) {
                active_threads++;  // Increase the active thread count
                pthread_t thread;  // Declare a thread variable
                pthread_create(&thread, NULL, handle_client, client_fd_ptr);  // Create a thread
                pthread_detach(thread);  // Detach the thread to clean up itself
            } else {
                printf("Max threads reached. Connection refused.\n");
                close(*client_fd_ptr);  // Close the connection if max threads are reached
                free(client_fd_ptr);  // Free the memory
            }
            pthread_mutex_unlock(&thread_mutex);  // Unlock the mutex
        }
    }

    // Cleanup shutdown
    close(server_fd);  // Close the server socket
    puts("Server closed successfully.");
    return 0;  // Exit successfully
}

