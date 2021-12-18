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

#include<stdio.h>
#include<string.h>    // for strlen
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#include<unistd.h>    // for write
#include<pthread.h>   // for threading, link with lpthread

#define PORT_NO 3205 

struct user{

	int userId; //user id
	int socketId; // socket id
	struct sockaddr_in client;
    char userName[100];	
	int inGroup; //If it is 1, user is in group, if it is 0, user is not in group
    int groupID; //the group id the user is in
};

struct group{

	int groupId; //group id
	char groupName[100]; //group name
	struct user users[100]; //user name in group
	int userNumInGroup; //number of users in the group
	char password[100]; //group password
};

int count = 0; //counting users
int gropuCount = 0; //counting groups
struct user usersArr[100]; //To keep all users
struct group Group[100]; //to kep all the groups

void * connection_handler(void * client); //It is a thread function

int main(){
    pthread_t thread[100];
    struct sockaddr_in server;

	int socket_desc = socket(PF_INET, SOCK_STREAM, 0);
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_NO);
	server.sin_addr.s_addr = htons(INADDR_ANY);

	if(bind(socket_desc,(struct sockaddr *) &server , sizeof(server)) <0){
        puts("Binding failed");
        return 1;
    }
	
	if(listen(socket_desc,100) <0){
        puts("Listen failed");
        return 1;
    }

	printf("\n\t** WHATSAPP **\n");

	while(1){    

        //A new user and thread is created with a new client
        int c = sizeof(struct sockaddr_in);
        int new_socket =accept(socket_desc, (struct sockaddr*) &usersArr[count].client, (socklen_t*)&c);
        if(new_socket<0){
            printf("Accept failed");
            return 0;
        }  
		usersArr[count].socketId = new_socket; //Socket id is the return value of the accept function
		usersArr[count].userId = count; //Since 'count' counts users each time they are created, it becomes a user id
		pthread_create(&thread[count], NULL, connection_handler, (void *) &usersArr[count]); //threads are created for each user
        count ++;	 
	}

	for(int i = 0 ; i < count ; i ++)
		pthread_join(thread[i],NULL);

    return 0;
}

//It is a thread function
//It is shown with messages on the client as a result of the commands from the user
void * connection_handler(void * client){

    struct user clientTemp = *((struct user*) client);
	int currentId = clientTemp.userId;
	int clientSocket = clientTemp.socketId;
    char userName[11];
    

	if(recv(clientSocket,userName,11,0)<0){ //Username received from the user is stored in the 'userName' variable
        printf("Recv failed");
        return NULL;
    }

    strcpy(clientTemp.userName, userName);
    strcpy(usersArr[currentId].userName, userName);

	printf("%s downloaded whatsapp.\n", usersArr[currentId].userName);//When a new client is created, a message is sent to the server.
 

    while(1){
        
        char getCommand[10];
     
        if(recv(clientSocket,getCommand,10,0) < 0){//Command received from the user is stored in the 'getCommand' variable
            printf("Recv failed");
            return NULL;
        }

        //A new group is created with the group name and group password received from the client.
        if(strcmp(getCommand,"-gcreate") == 0){ 

            char userNo[11];
            char password[100];
            char groupName[100];
            char returnMessage[100]="\n";
            if(recv(clientSocket,password,100,0) < 0){//Password received from the user is stored in the 'password' variable
                printf("Recv failed");
                return NULL;
            }

            if(recv(clientSocket,groupName,100,0) < 0){//Group name received from the user is stored in the 'groupName' variable
                printf("Recv failed");
                return NULL;
            }
            char group_name[100];
            char pass[100];

            strcpy(group_name,groupName);
            strcpy(pass,password);
            strcat(returnMessage, userName);
            strcat(returnMessage," create group -> ");
            strcat(returnMessage,groupName);

            strcpy(Group[gropuCount].groupName,group_name); //The group name taken from the user becomes the name of the group.
            Group[gropuCount].groupId=gropuCount;   //Since groupCount increases every time a group is created, it becomes the group's id.
            Group[gropuCount].users[0]= usersArr[currentId];    //The person who created the group is added as the first person in the group
            strcpy(Group[gropuCount].password,pass);    //The password received from the user becomes the password of the group.
            
            send(usersArr[currentId].socketId,returnMessage,100,0); //A message is sent to the user when a new group is created.
            usersArr[currentId].inGroup=1; //The user becomes the first member of the group
            Group[gropuCount].userNumInGroup++; //The number of users in the group increases by one
            gropuCount++; //The number of groups increases by one
        }

        //The user can join a group with the group name and group password
        else if(strcmp(getCommand,"-join") == 0){ 

            int i;
            char group_name[100];
            char password[100];
            char groupID;
            char returnMessage[100]="\n";

            if(recv(clientSocket,group_name,100,0)){//Group name received from the user is stored in the 'groupName' variable
                printf("Recv failed");
                return NULL;
            }
            if(recv(clientSocket,password,100,0)){//Password received from the user is stored in the 'password' variable
                printf("Recv failed");
                return NULL;
            }

            //If the group name entered by the user is available and the password is correct, he / she joins that group.
            for(i=0; i<gropuCount; i++){
                if(strcmp(Group[i].groupName,group_name)==0&&strcmp(Group[i].password,password)==0){
                    groupID=Group[i].groupId;
                    usersArr[i].inGroup=1;
                    usersArr[i].groupID=groupID;
                    break;
                }
            }

        }

        //User can send a message to another user
        else if(strcmp(getCommand,"-send") == 0){

            char sendUser[11];
            char tempUser[11];
            char message[100];
            char tempMes[100];
            char returnMessage[100]="\n";

            if(recv(clientSocket,sendUser,11,0) < 0){//Username received from the user is stored in the 'sendUser' variable
                printf("Recv failed");
                return NULL;
            }

            strcpy(tempUser,sendUser);

            if(recv(clientSocket,message,100,0) < 0){//Message received from the user is stored in the 'message' variable
                printf("Recv failed");
                return NULL;
            }

            strcpy(tempMes,message);

            strcat(returnMessage,userName);
            strcat(returnMessage," : ");
            strcat(returnMessage,tempMes);
            strcat(returnMessage,"\n");        
         
            int i;
            for(i=0;i<count;i++){//The user name entered by the user is found and a message is sent to that client.
                if(strcmp(usersArr[i].userName,tempUser)==0){
                    send(usersArr[currentId].socketId,returnMessage,100,0);
                    send(usersArr[i].socketId,returnMessage,100,0);
                }
            }
        }

        //The username of the current user is displayed
        else if(strcmp(getCommand,"-whoami") == 0){
            char whoami[100]="\n -->Your username: ";
            strcat(whoami,userName);
            send(usersArr[currentId].socketId,whoami,100,0);
            
        }

        //If the user wants to leave the group, the user's inGroup variable is set to 0. So this user is not in the group.
        //The groupID variable of the user is also changed to -1.
        else if(strcmp(getCommand,"-exit") == 0){
            
            char group_name[100];
            char returnMessage[100]="\n";
            if(recv(clientSocket,group_name,100,0) < 0){
                printf("Recv failed");
                return NULL;
            }

            if(usersArr[currentId].inGroup == 1){
                int i;
                for(i=0;i<gropuCount;i++){ //The group the user is in is found and the user is removed from the group
                    if(strcmp(Group[i].groupName,group_name)==0){
                        usersArr[currentId].inGroup=0; //the user's inGroup variable is set to 0, because the user leaves the group
                        usersArr[currentId].groupID=-1; 
                        break;
                    }
                }
                if(i==gropuCount){
                    send(usersArr[currentId].socketId,"You are not already in the group you wrote.",100,0);	
            		
                }
                else{
                    send(usersArr[currentId].socketId,"You left this group.",100,0);	
            		
                }
            }
            else{
                send(usersArr[currentId].socketId,"Grupta değilsin.",100,0);
                
            }
        }
    }
	return NULL;
}