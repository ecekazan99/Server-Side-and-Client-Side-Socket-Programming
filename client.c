/*
    gcc 2017510049_server.c -o outputfile -lpthread
    gcc 2017510049_client.c -o outputfile -lpthread
*/
/*
    There is no problem with the client file, it works correctly.
    But other than the whoami command do not work very well. 
    I couldn't arrange the order of the messages because I use threads. 
    Even though the code works correctly, I could not show it.
    In other words, the bottom line in the code can work first, then the username or password is obtained. 
    This is understood as the code is running incorrectly.
*/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr

#define PORT_NO 3205 
void *connection_handler(void *socket_desc);

int main(){
    char username[11];
    struct sockaddr_in server;
    pthread_t thread;

    int socket_desc = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_desc <0){
        puts("Could not create socket");
        return 1;
    }

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_NO);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(socket_desc, (struct sockaddr*) &server, sizeof(server)) <0) {
        puts("Connection error");
        return 1;
    }
	puts("\n**  WHATSAPP  **\n");
    
    //create thread
    pthread_create(&thread, NULL, connection_handler, (void *) &socket_desc );
  
    //username is sent to the server
  	printf("Please, enter your username: ");
  	scanf("%s", username);
	send(socket_desc,username,15,0);

    //Continues until the user types the exit command
    while(1){
        puts("\n* Commands");
        puts("-gcreate"); puts("  -> phone_number"); puts("  --> group_name");
        puts("-join"); puts("  -> username/group_name");
        puts("-exit"); puts("  -> group_name");
        puts("-send"); puts("  -> message_body");
        puts("-whoami");
        puts("-exit\n");

        //command is sent to the server
        puts("Enter command: ");
        char command[100];
        scanf("%s", command);

        //The group is created
        //Group name and group password are sent to the server.
        if(strcmp(command,"-gcreate") == 0){ 
            
            send(socket_desc,command,100,0);//The command is sent

            char group_name[100];
            puts("Group name: ");
            scanf("%s", group_name);
            send(socket_desc,group_name,100,0);//The group name is sent

            char password[100];
            puts("Setting password: ");
            scanf("%s", password);
            send(socket_desc,password,100,0); //The password is sent
        }

        //The group name and user name are sent to the server.
        else if(strcmp(command,"-join") == 0){ 
            send(socket_desc,command,200,0);//The group name is sent

            char groupNameOrNo[100];
            puts("Phone number/Group name: ");
            scanf("%s", groupNameOrNo);
            send(socket_desc,groupNameOrNo,200,0);//The group name or username is sent

            char password[100];
            puts("Enter group's password: ");
            scanf("%s", password);
            send(socket_desc,password,200,0);//The password is sent
        }

        //If the user is in the group, it sends a message to another user, if not to the group. 
        //The username/group name and message are sent to the server.
        else if(strcmp(command,"-send") == 0){ 

            send(socket_desc,command,200,0);//The command is sent

            char users[11];
            puts("Enter username/group name: ");
            scanf("%s", users);
            send(socket_desc,users,11,0);//The group name or username is sent

            char message[1000];
            puts("Enter message: ");
            scanf("%s", message);
            send(socket_desc,message,1000,0);//The message is sent
        }

        //This command displays the username of the user.
        else if(strcmp(command,"-whoami") == 0){

            send(socket_desc,command,100,0);//The command is sent
        }

        //User can leave either group or program
        else if(strcmp(command,"-exit") == 0){ 

            send(socket_desc,command,100,0);//The command is sent
            puts("If you want to leave the group, write 'y'");
            puts("If you want to exit the program, write 'n'");

            char input[10];
            scanf("%s", input);

            if(strcmp(input,"y")==0||strcmp(input,"Y")==0){
               
                char group_name[100];
                puts("You chose to leave the group.");
                puts("Group name: ");
                scanf("%s", group_name);
                send(socket_desc,group_name,200,0); //The group name is sent
            }
            else{
	            break;
            }
        }
    }
}

//It is a function belonging to the thread.
//It is to show the messages received by the server on the client side.
void *connection_handler(void *socket_desc){

    int socketID = *((int *) socket_desc);

    while (1){

		char arr[100];
		recv(socketID,arr,100,0);
		printf("%s\n",arr);   
    }
}