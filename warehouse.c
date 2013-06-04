#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

/* defining the port numbers to be used
 */

#define tcp_port "21532"
#define udp_port1 "31532"
#define udp_port2 "32532"
#define udp_store1port "5532"

//defining the local host
#define host "nunki.usc.edu"
#define queue_limit 10
#define buffer_size 100


int main(){

    int status,k;
    struct addrinfo hints,*res,*p,udphint,*udpres,*pr;
    struct sockaddr_storage c_addr;
    socklen_t clientaddr_size;
    int childS_fd[10],i,j,truckVector[20],aggr[20];
    int bytes_sent,sockfd,recvd_bytes;
    void *address;
    char ipstr[100];
    int recValue[20][20];
    struct hostent *hn;
    struct in_addr **addr_list;

    

    //making sure the structure is empty
    memset(&hints, 0, sizeof hints);
    //using IPV4
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    //as suggested by beej guide using getaddrinfo which gives much more information
   if ((status = getaddrinfo(host,tcp_port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    //creating a socket which returns a socket descriptor 
     for(p = res; p != NULL; p = p->ai_next) {
         if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
       perror("reciever:socket");
       continue;
         }

         // binding the  socket with the ip address
   
         if((bind(sockfd, p->ai_addr,p->ai_addrlen))==-1){
             close(sockfd);
             perror("reciever:bind");
             continue;
         }
         break;
     }
    
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    address = &(ipv4->sin_addr);
    inet_ntop(res->ai_family,address,ipstr,sizeof(ipstr));


    printf("\n\nPhase-1:the central warehouse has TCP port number %s and the IP address  %s\n ",tcp_port,ipstr) ;

    /* listening  to connection froms client by giving the socket descriptor and 
     * giving upper limit to the number of clients that could be waiting
     */
    
    if((listen(sockfd,queue_limit))==-1){
        perror("listen");
    }
    
    
    // main accept() loop
    clientaddr_size = sizeof c_addr;
   
    i=0;
    while(i<4){
        
        childS_fd[i] = accept(sockfd, (struct sockaddr *)&c_addr, &clientaddr_size);
        if (childS_fd[i] == -1) {
            perror("accept");
            continue;
        }
        i++;
    }
   
    printf("\n");

    // ready to recieve on socket descriptor childS_fd! which is a child socket
    for(i=0;i<4;i++){

        if ((recvd_bytes = recv(childS_fd[i], recValue[i],sizeof recValue, 0)) == -1) {
             perror("recv");
            exit(1);
        }
        
        printf("The central warehouse received information from store#%d\n", i+1);
    }
    
    close(sockfd);
    printf("\n\nEND OF PHASE-1 FOR THE CENTRAL WAREHOUSE\n");
    

    for(j=0;j<3;j++){

        aggr[j]=recValue[0][j]+recValue[1][j]+recValue[2][j]+recValue[3][j];
    }
    
    printf("\n\n");
    printf("Phase2: The central warehouse has UDP port number %s and IP address %s\n",udp_port1,ipstr); 
    printf("Sending the truck vector to outlet store_1");
    printf("\nThe truck vector value is <");
    for(k=0;k<3;k++){
        if(aggr[k]<0){
            truckVector[k]= (aggr[k]*(-1));
            printf(" %d",truckVector[k]);
        }else{
            truckVector[k]=0;
            printf(" %d",truckVector[k]);
        }
    }
    printf(" >\n\n");
    freeaddrinfo(res);

    //START OF PHASE- 2

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    //getting the address information of the reciever
    if ((status = getaddrinfo(host,udp_store1port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }


    // binding datagram socket to the static udp port of the sender(warehouse) as given in the project specification 
     memset(&hints, 0, sizeof udphint);
     udphint.ai_family = AF_INET;
     udphint.ai_socktype = SOCK_DGRAM;

     if ((status = getaddrinfo(host,udp_port1, &udphint, &udpres)) != 0) {
         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
         exit(1);
     }

     for(pr = udpres; pr != NULL; pr = pr->ai_next) {
         if ((sockfd = socket(pr->ai_family, pr->ai_socktype,pr->ai_protocol)) == -1){
             perror("sender:socket");
             continue;
         }

         if((bind(sockfd, pr->ai_addr,pr->ai_addrlen))==-1){
             close(sockfd);
             perror("sender:bind");
             continue;
         }
         break;
     }
     
     ipv4 = (struct sockaddr_in *)udpres->ai_addr;
     address = &(ipv4->sin_addr);
     inet_ntop(udpres->ai_family,address,ipstr,sizeof(ipstr));



    if ((bytes_sent = sendto(sockfd,truckVector,sizeof truckVector, 0, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("sender: sendto");
        exit(1);
    }
    
    //START OF PHASE_2 PART-3
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
     hints.ai_socktype = SOCK_DGRAM;
     if ((status = getaddrinfo(host,udp_port2, &hints, &res)) != 0) {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
         return 1;
     }
     
     for(p =res; p != NULL; p = p->ai_next) {
         if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
             perror("reciever:socket");
             continue;
         }
         
         if((bind(sockfd,p->ai_addr,p->ai_addrlen))==-1){
             close(sockfd);
             perror("reciever:bind");
             continue;
         }
         break;
     }
    
     //getting the ip-adderss

     ipv4 = (struct sockaddr_in *)res->ai_addr;
     address = &(ipv4->sin_addr);
     inet_ntop(res->ai_family,address,ipstr,sizeof(ipstr));


     printf("Phase2: The central warehouse has UDP port number %s and IP address %s\n",udp_port2,ipstr);
     
     clientaddr_size = sizeof c_addr;
     if ((recvd_bytes =recvfrom(sockfd, truckVector, sizeof truckVector , 0,(struct sockaddr *)&c_addr, &clientaddr_size)) == -1){
         perror("recv");
         exit(1);
     }


     printf("The final truck-vector is received from the outlet store store_4,"); 
     printf("the truck-vector value is:<");
     
     for(i=0;i<3;i++){
         printf(" %d",truckVector[i]);
     }
     
     printf(" >\n\n");
     printf("\nEND OF PHASE-2 FOR THE CENTRAL WAREHOUSE\n\n");



    close(sockfd);
    return 0;
}






