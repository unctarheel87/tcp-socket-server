#include <iostream>
#include <fstream>
#include <iostream>
#include <fstream>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <fcntl.h> 
#include<sys/time.h>

#define PORT 8080 
#define BUFLEN 256
#define TO_SEC 1
#define TO_USEC 50000

void logErr() {
    printf("Error code: %d\n", errno);
}

int recv_full(int s, char *buffer) {
	int size_recv, total_size = 0;
    char chunk[BUFLEN];
    struct timeval tv = { TO_SEC, TO_USEC };
     // set custom timeout for partial data packet retrieval
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        std::cout << "Socket bind error" << std::endl;
        logErr();
    } 
	
	while(1) {
        if ((size_recv = recv(s, chunk, BUFLEN, 0)) < 0) {
            std::cout << "data transfer complete" << std::endl;
            break;
        }
        total_size += size_recv;
        // strcat(buffer, chunk);
	}
	
	return total_size;
}

std::string buildRes(std::string body, std::string contentType) {
    std::string h = "HTTP/1.1\r\n";
    std::string ct = "Content-Type:" + contentType + "\r\n\r\n";
    return h + ct + body;
}

std::string readHTML(std::string fileName) {
    std::ifstream file(fileName);
    std::string str; 
    std::string file_contents;
    while (std::getline(file, str)) {
        file_contents += str;
        file_contents.push_back('\n');
    } 
    return file_contents;
}

std::string parseRes(char *buffer, int count) {
    std::string body;
    bool firstBlankLine; 
    for (int i = 0; i < count; i++) {
        if (buffer[i-3] == '\r' && 
            buffer[i-2] == '\n' && 
            buffer[i-1] == '\r' && 
            buffer[i] == '\n'
        ) {
            firstBlankLine = true;
            continue;
        }
        if (firstBlankLine) {
            body += buffer[i];
        }
    }
    return body;
}

int main(int argc, char const *argv[]) { 
    int fd, connfd, rcount; 
    struct sockaddr_in addr, claddr; 
    int opt = 1; 
    int addrlen = sizeof(addr); 
    socklen_t cliaddrlen = sizeof(addrlen);
    char buffer[100000];

    // Creating socket file descriptor 
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = INADDR_ANY; 
    addr.sin_port = htons(PORT); 

    // allow server to reuse port if already open
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        std::cout << "Socket bind error" << std::endl;
        logErr();
    } 
       
    // bind to address
    if (bind(fd, (struct sockaddr *)&addr, addrlen) == -1) { 
        std::cout << "Socket bind error" << std::endl;
        logErr();
        exit(1);
    } 

    // listen
    if (listen(fd, 3) == -1) { 
        std::cout << "Socket listen error" << std::endl; 
        logErr();
    } 
    std::cout << "Server is listening at PORT: " + std::to_string(PORT) << std::endl; 

    // keep open
    while(1) { 
        // accept incoming socket connections
        connfd = accept(fd, (struct sockaddr *)&claddr, &cliaddrlen);
        if (connfd == -1) { 
            std::cout << "Socket accept error" << std::endl;
            logErr();
        } 
        std::cout << "accepting new connection" << std::endl;

        std::string msg = readHTML("index.html");
        std::string res = buildRes(msg, "text/html");
        // send response to client
        send(connfd, res.data(), res.size(), 0); 
        
        // if (buffer[0] == 'P') {
        //     res = buildRes(parseRes(buffer, rcount), "text/plain");
        //     // std::ofstream ofs ("test.txt", std::ofstream::out);
        //     // ofs << parseRes(buffer, rcount);
        //     // ofs.close();
        //     std::cout << buffer << std::endl;
        // }
       
        // recv_timeout(connfd, buffer, 4);
        rcount = recv_full(connfd, buffer);

        std::cout << rcount << std::endl;
        std::cout << "buffer: ";
        std::cout << buffer << std::endl;
        std::cout << "<---------- buffer end------------>" << std::endl;

        close(connfd);
    }
    close(fd);

    return 0; 
} 
