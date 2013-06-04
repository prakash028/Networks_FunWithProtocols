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

/*
 * defining the port numbers and host to be used
 */

# define warehouse_tcp_port "21532"
# define udp_store1port "7532"
# define udp_warehouseport "32532"
# define udp_port1 "17532"
# define udp_port2 "18532"
# define udp_port3 "19532"
# define udp_port4 "20532"
# define host "nunki.usc.edu"


int file_reader(char *filename);
int outletVector[10];

int main(){
    char *selectedport,*portforstore,*recfrom,*port,*sento;
    int sockfd, recvd_bytes,status;
    struct addrinfo hints, *serverinfo, *p,udphint,*udpres,*pr;
    socklen_t clientaddr_size;
    struct sockaddr_storage c_addr;
    int i,j,bytes_sent;
    void *address;
    char ipstr[100];
    int truckVector[10];
    struct hostent *hn;
    struct in_addr **addr_list;




    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
   
    /*
     * getting the address iformation of host at warehouse_tcp_port
     * if successful,the results are used to obtain a socket descriptor
     *
     */

    if ((status = getaddrinfo(host,warehouse_tcp_port, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                        return 1;
    }


    /*
     * Reading the file Store-4.txt by calling the function file_reader
     * The  vector read,is stored in global outlet vector
     */
    
    if(!file_reader("Store-4.txt")){
        return 0;
    
    }
    
    /*
     * obtaining the socket descriptor and establishing connection
     */
    
    for(p = serverinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("sender:socket");
            continue;
        }
        
        if(connect(sockfd, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1){
         close(sockfd);
         perror("sender: connect");
         continue;
        }
        break;
    }

    if(p == NULL){
         fprintf(stderr, "sender: failed to connect\n");
         return 2;
    }
    
    if((hn = gethostbyname(host)) == NULL){
      herror("gethostbyname");
      return 2;
    }
    addr_list = (struct in_addr **)hn->h_addr_list;
   
    struct sockaddr_in s;
    socklen_t len= sizeof(s);
    if((status=getsockname(sockfd,(struct sockaddr *)&s,&len))==-1){
        perror("getsockname");
        exit(1);
    }

    printf("\n\nPhase 1: store_4 has TCP port number %d and IP address %s\n",ntohs(s.sin_port),inet_ntoa(*addr_list[0]));

    /*
     * Sending the outlet vector read from the file
     */

    
    if((bytes_sent = send(sockfd, outletVector, sizeof outletVector, 0)) == -1) {
        perror("send");
    }else{
       printf("Phase 1: The outlet vector <");
       for(i=0;i<3;i++){
           printf("%d ",outletVector[i]);
       }
       printf(">");
       printf( "for Store4 has been sent to the central warehouse\n");
    }
    
    printf("\nEnd of Phase 1 for Store 4\n\n\n");
    
    freeaddrinfo(serverinfo);
    close(sockfd);


    /*
     * Start of Phase 2
     * that will be repeated for two rounds
     * !st round : Store4 will recieve the truck vector from store3 and send updated truck vector to store1
     * 2nd round: Store4 will recieve the truck vector from store3 and send the updated truck vector to the central warehouse
     */
    

    for(j=0;j<2;j++){
        
        if(j==0){
            //for round 1
            selectedport=udp_port1;
            recfrom="store 3";
        }else{
            //for round 2
            selectedport= udp_port3;
            recfrom="store 3";
        }
        
        /*
         * Setting up a UDP connection to recieve the truck vector
         */
        
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
       
        /*
         * getting the address iformation of host at selectedport
         * if successful,the results are used to obtain a socket descriptor
         */
        
        if ((status = getaddrinfo(host,selectedport, &hints, &serverinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return 1;
        }
       
        /*
        * obtaining the socket descriptor and  binding to the socket
        */

        for(p = serverinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("reciever:socket");
                continue;
            }
            
            //binding

            if((bind(sockfd,p->ai_addr,p->ai_addrlen))==-1){
                close(sockfd);
                perror("reciever:bind");
                continue;
            }
            break;
        }
        
        if(p == NULL){
            fprintf(stderr, "reciever: failed to bind\n");
            return 2;
        }


        struct sockaddr_in *ipv4 = (struct sockaddr_in *)serverinfo->ai_addr;
        address = &(ipv4->sin_addr);
        inet_ntop(serverinfo->ai_family,address,ipstr,sizeof(ipstr));
        
        printf("Phase 2: store_4 has UDP port number %s  and IP address %s",selectedport,ipstr);


        clientaddr_size = sizeof c_addr;
        if ((recvd_bytes =recvfrom(sockfd, truckVector, sizeof truckVector , 0,(struct sockaddr *)&c_addr, &clientaddr_size)) == -1){
             perror("reciever:recv");
            exit(1);
        }


        printf("\nPhase 2: Store_4 received the truck-vector <");
        for(i=0;i<3;i++){
            printf("%d ",truckVector[i]);
        }
        printf("> from %s\n",recfrom);

        /*
         * Performing the updation by checking each elemnt of the outlet vector 
         * Comparing it with the  recieved truck vector and deciding on the new values
         */

        for(i=0;i<3;i++){
            if(outletVector[i]<0){
                outletVector[i]=outletVector[i]+truckVector[i];
                if(outletVector[i]>0){
                    truckVector[i]=outletVector[i];
                    outletVector[i]=0;
                }else{
                    truckVector[i]=0;
                }
            }else{
                truckVector[i]=truckVector[i]+outletVector[i];
                outletVector[i]=0;
            }
        }

        

        freeaddrinfo(serverinfo);
        close(sockfd);

        if(j==0){
            //round 1
            portforstore = udp_store1port;
            port = udp_port2;
            sento ="store 1";
        }else{
            //round 2
            portforstore =udp_warehouseport;
            port = udp_port4;
            sento = "the central warehouse";
        }

        /*
         * START OF PHASE_2 PART 2
         * Sending the updated truck vector to the store1 in first round and warehouse in second round
         * repeating the same procedure for sending 
         * this time using datagram socket
         */
        
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if ((status = getaddrinfo(host,portforstore, &hints, &serverinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return 1;
        }
        
        // binding datagram socket to the static udp port of the sender(store4) as given in the project specification.
        
        memset(&hints, 0, sizeof udphint);
        udphint.ai_family = AF_INET;
        udphint.ai_socktype = SOCK_DGRAM;

        if ((status = getaddrinfo(host,port, &udphint, &udpres)) != 0) {
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
        
        //getting the ip address
        ipv4 = (struct sockaddr_in *)serverinfo->ai_addr;
        address = &(ipv4->sin_addr);
        inet_ntop(serverinfo->ai_family,address,ipstr,sizeof(ipstr));
        
        
        printf("\nPhase 2: Store_4 has UDP port  %s  and IP address %s ",port,ipstr);
        

        if ((bytes_sent = sendto(sockfd,truckVector,sizeof truckVector, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("sender: sendto");
        exit(1);
        }
        
        printf("\nPhase2: The updated truck-vector<");
        for(i=0;i<3;i++){
            printf("%d ",truckVector[i]);
        }
        printf(">has been sent to %s\n",sento);
        printf("Phase2: Store_4 updated outlet-vector is <");
        
        for(i=0;i<3;i++){
            printf("%d ",outletVector[i]);
        }
        printf("> \n\n");

        if(j==1){
            printf("\nEND OF PHASE-2 FOR STORE-4\n\n\n");
        }

        close(sockfd);
    }

    return 0;

}


int file_reader(char *filename){
    int i;
    FILE *file;
    char lines[100];
    char *tk1,*tk2;
    //getting the file Store-4.txt and extracting the vector
    if((file = fopen(filename,"r")) == NULL){
        perror("Please change the outlet vector filename to Store-4.txt");
        return 0;
    }
    //reading lines  
    i=0;
    
    while(fgets(lines,100, file) != NULL){
        // using string token to extract characters
        tk1 = strtok(lines," ");
        tk2= strtok(NULL," ");
        if(tk2){
            outletVector[i]= atoi(tk2);
            i++;
        }
    }
    fclose(file);return 1;
}







