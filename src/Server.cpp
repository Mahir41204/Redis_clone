#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

int make_non_blocking(int fd){
  return fcntl(fd, F_SETFL, fcntl(fd,F_GETFL,0) | O_NONBLOCK);
}
std::string read_line(const std::string & data,size_t & pos){
  size_t end =data.find("\r\n",pos);
  std::string line =data.substr(pos,end-pos);
  pos = end+2;
  return line;
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  make_non_blocking(server_fd);
  
  struct sockaddr_in client_addr{};
  int client_addr_len = sizeof(client_addr);
  std::cout << "Waiting for a client to connect...\n";

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  fd_set master_set,read_set;
  FD_ZERO(&master_set);
  FD_SET(server_fd, &master_set);
  int max_fd = server_fd;

  while(true){
    read_set = master_set;
    select(max_fd + 1,&read_set, nullptr, nullptr, nullptr);

    for(int fd=0;fd<=max_fd;fd++){
      
      if(!FD_ISSET(fd,&read_set))
        continue;
     //for accepting new client
      if(fd==server_fd){
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
        make_non_blocking(client_fd);
        FD_SET(client_fd, &master_set);
        if(client_fd>max_fd)
          max_fd = client_fd;
        std::cout << "New Client Connected\n";  
      }
  
      else{
          char buffer[1024];
          int bytes_read = recv(fd, buffer, sizeof(buffer)-1, 0);
          if (bytes_read <=0){
            std::cout << "Client disconnected\n";
            close(fd);
            FD_CLR(fd, &master_set);
          }
          else{
            buffer[bytes_read] ='\0';
            std::string input(buffer);
            size_t pos=0;
            if(input[0]=="*"){
              int arg_count std::stoi(read_line(input, pos).substr(1));
              std::string command = read_line(input, pos);
              command =read_line(input, pos);
              
              if(command == "PING"){
                std:: string response = "+PONG\r\n";
                send(fd,response.c_str(), response.size(), 0);
              }
              else if(command =="ECHO"){
                std::string mssg_len = read_line(input,pos);
                int len = std::stoi(mssg_len.substr(1));
                std::string mssg =read_line(input,pos);
                mssg = "$" + mssg + "\r\n" ;
                send(fd,mssg.c_str(),mssg.size(),0);
              }
            
            }
            
            
 
          }
      }
    }
 }

  close(server_fd);

  return 0;
}
