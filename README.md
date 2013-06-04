Networks_FunWithProtocols
=========================

This is a brief Description for the project.

PROJECT DESCRIPTION:
In this project I simulated a logistics system. The system consists of a set of Outlet stores and one central warehouse. The central warehouse manages the needs for each outlet store and makes sure that the requirements for each outlet store is satisfied. Each outlet store reports its own inventory to the central warehouse. Based on the collected information, the central-warehouse computes the excess supply or shortage of each good. The warehouse then dispatches a truck to visit the stores successively to collect the excessive goods or replenish any shortages. All communications take place over TCP and UDP sockets in client-server architecture. The project has 2 major phases: inventory reporting, stock replenishment.

IMPLEMENTATION DETAILS:

FILE: warehouse.c: this file contains the code for the central warehouse. Initially the central warehouse creates a tcp socket and binds it to the host address and port number 21532 as specified in the project description. The hostname is hardcoded as host=”nunki.usc.edu” and all the static ports are also hardcoded. The central warehouse then listens (wait) for any communication at the tcp port. For each new incoming connection it creates child sockets that are associated with the single parent socket. After it has received all the outlet vectors sent by store-1, store-2, store-3 and store-4, it computes their sum and then creates a truck vector, which contains the supplies that are needed for the stores and this vector is called the truck vector. The truck vector is then send to the store-1 using a udp connection which requires the port number of store-1, which has also been hardcoded. The udp chooses the port dynamically but for the project I have binded the udp socket to port 31532 as it was required. Finally, the warehouse opens another udp connection at port 32532 by binding a udp socket to this port and waits for a incoming udp connection from store-4 which will contain the final updated truck vector.

File: store1.c, store2.c, store3.c, store4.c Common Functionalities:
THE TCP CONNECTION: Each store first reads the corresponding text files form, which the outlet vector has to be read namely Store-1.c, Store-2.c, Store-3.c and Store-4.txt.Each of the store then sets up a tcp connection to the central warehouse by connecting the tcp socket to the central warehouse using the connect () command. The tcp port for sending is chosen dynamically which is printed using getsockname().
Phases:

STORE-1: Store-1 opens a udp connection at port 5532 and waits for the truck vector send by the central warehouse. It then computes the value of the truck vector by comparing it with the outlet vector. The deficiency is compensated and the truck vector takes extra supplies away. The updated truck vector is send to the store-2. Then the store-1 opens another udp connection at pot 7532 and waits to receive the truck vector from store-4. This has been achieved using a for loop which runs two times.

STORE-2 and Store-3: Store -2 opens a udp connection and wait to receives the truck vector from store-1.It then computes the values and send the updated truck vector to store-3  which it self has opened a tcp connection to hear from store2.Store-3 then sends the updated truck vector o store-4.the same procedure is repeated two times.

STORE-4 The store-4 receives the updated truck vector from store 3 at udp port mentioned in the project specification. It then computes the value of its outlet vector and sends the updated truck vector to store-1.After receiving the truck vector for the second time from store3 it sends the computed truck vector to central warehouse which has a open udp connection for receiving the truck vector.

INSTRUCTIONS FOR THE User:
Please compile the 5 files namely warehouse.c,store1.c,store2.c,store3.c and store4.c using cc or gcc and  the command:
cc -o filename filename.c -lnsl -lsocket –lresolv

Execute the output files in the sequence ./warehouse, ./store1, ./store2 , ./store3 , ./store4.
Inventory files should have the name Store-n.txt n=1,2,3,4. Otherwise the program will give an error suggesting to correct the file name.

Reference:
I have used the standard commands given in the Beej’s guide as mentioned in the project specification. Some part of the code has been directly implemented using the same syntax and some part has been altered as per need. I also used Google time to time to study the code in more detail.

