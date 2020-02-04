#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#define MAX 800
#define PORT 12345
#define SA struct sockaddr
#include "cJSON.h"
#include "cJSON.c"

void identify(char* pusername, char* ppassword)
{
     printf("Enter Username\n");
    scanf("%s", pusername);
    printf("Enter Password\n");
    scanf("%s", ppassword);
}
void error(char* buffer)
{
    cJSON* r = cJSON_Parse(buffer);
    printf("%s\n", cJSON_GetObjectItem(r, "content")->valuestring);
}
int issuccessful(char* buffer)
{
    cJSON* r = cJSON_Parse(buffer);
    if(strcmp(cJSON_GetObjectItem(r, "type")->valuestring, "Successful") == 0)
        return 1;
}
int client_socket;
int create_socket(){
    //int client_socket;
	struct sockaddr_in servaddr, cli;

	WORD wVersionRequested;
    WSADATA wsaData;

	// Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    wVersionRequested = MAKEWORD(2, 2);

    int err;
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        // Tell the user that we could not find a usable Winsock DLL.
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
        // Create and verify socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        // Assign IP and port
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servaddr.sin_port = htons(PORT);
        // Connect the client socket to server socket

        connect(client_socket, (SA*)&servaddr, sizeof(servaddr));
        /*if (connect(client_socket, (SA*)&servaddr, sizeof(servaddr)) != 0) {
            printf("Connection to the server failed...\n");
            exit(0);
        }*/
}

void servercontant (char* buffer, size_t packet_size)
{
    create_socket();
    send(client_socket, buffer, packet_size , 0);
    memset(buffer, 0, packet_size);
    recv(client_socket, buffer, packet_size, 0);
    closesocket(client_socket);
}

int main()
{
    int status = 1;
    while(1){
        char buffer[MAX], username[30], password[30], channelname[30], message[40];
        char *pusername = &username, *ppassword = &password;
        int response;
        cJSON * token;
        memset(buffer, 0, sizeof(buffer));
        if(status == 1)//Account Menu
        {
            printf("Account Menu:\n1: Register\n2: Login\n");
            scanf("%d" , &response);
            identify(pusername , ppassword);
            if(response == 1)//Register
            {
                sprintf(buffer , "register %s, %s\n" , &username , &password);
                servercontant(buffer, sizeof(buffer));
                if(issuccessful(buffer) == 1)
                    printf("Successful\n");
                else error(buffer);
                continue;
            }
            else//login
            {
                sprintf(buffer , "login %s, %s\n" , username , password);
                servercontant(buffer, sizeof(buffer));
                cJSON* r = cJSON_Parse(buffer);
                if(strcmp(cJSON_GetObjectItem(r , "type")->valuestring , "AuthToken") == 0)
                {
                    printf("Successful\n");
                    token = cJSON_GetObjectItem(r , "content");
                    status++;
                }
                else
                    error(buffer);
                continue;

            }
        }
        if(status == 2)//Main Menu
        {
            printf("1: Create Channel\n2:Join Channel\n3:Logout\n");
            scanf("%d" , &response);
            if(response == 1 || response == 2)
            {
                printf("Enter Channel Name\n");
                scanf("%s" , &channelname);
                if(response == 1)//Create Channel
                    sprintf(buffer , "create channel %s, %s\n" , channelname , token->valuestring);
                else sprintf(buffer, "join channel %s, %s\n", channelname, token->valuestring);//Join Channel
                servercontant(buffer, sizeof(buffer));
                if(issuccessful(buffer) == 1)
                {
                    printf("Successful\n");
                    status++;
                }
                else error(buffer);
                continue;
            }
            else//Logout
            {
                sprintf(buffer, "logout %s\n", token->valuestring);
                servercontant(buffer, sizeof(buffer));
                if(issuccessful(buffer) == 1)
                {
                    printf("Successful\n");
                    status--;
                }
                else error(buffer);
            }
        }
        if(status == 3)//Conversation Menu
        {
            printf("1: Send Message\n2: Refresh\n3: Channel Members\n4: Leave Channel\n");
            scanf("%d", &response);
            if(response == 1)//Send Message
            {
                scanf("%s", &message);
                sprintf(buffer, "send %s, %s\n", message, token->valuestring);
                servercontant(buffer, sizeof(buffer));
                if(issuccessful(buffer) == 1)
                {
                    printf("Successful\n");
                    continue;
                }
            }
            if(response == 2)//Refresh
            {
                sprintf(buffer, "refresh %s\n", token->valuestring);
                servercontant(buffer, sizeof(buffer));
                cJSON* r = cJSON_Parse(buffer);
                printf("%s\n", cJSON_GetObjectItem(r, "content")->valuestring);
                continue;
            }
            if(response == 3)//Channel Members
            {
                sprintf(buffer, "channel members %s\n", token->valuestring);
                servercontant(buffer, sizeof(buffer));
                cJSON* r = cJSON_Parse(buffer);
                printf("%s", cJSON_GetObjectItem(r, "content")->valuestring);
                continue;
            }
            if(response == 4)//Logout
            {
                sprintf(buffer, "leave %s\n", token->valuestring);
                servercontant(buffer, sizeof(buffer));
                if(issuccessful(buffer) == 1)
                {
                    printf("Successful\n");
                    status--;
                }
                else error(buffer);
                continue;
            }
        }
    }
}
