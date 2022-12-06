#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define NAME_SZ 32
#define BUFFER_SZ 4096

//Client Structure
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[NAME_SZ];
} client_t;
client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

static _Atomic unsigned int cli_count = 0;
static int uid = 10;


// Function to print IP address of the client
void print_ip_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}


//Function for Adding clients to the Queue
// Lock the clients_mutex
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

//Function to remove clients from the queue
void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


//Function to send message to all clients using user id except sender 
void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

// Handle all communication with the client 
void *handle_client(void *arg){
	char buffer[BUFFER_SZ];
	char name[NAME_SZ];
	int leave_flag = 0;

	cli_count++;
	client_t *cli = (client_t *)arg;

	// Name
	if(recv(cli->sockfd, name, NAME_SZ, 0) <= 0 || strlen(name) <  2 || strlen(name) >= NAME_SZ-1){
		printf("Enter the name correctly\n");
		leave_flag = 1;
	} else{
		strcpy(cli->name, name);
		sprintf(buffer, "\n%s has joined the chat!\n", cli->name);
		printf("\n%s", buffer);
		send_message(buffer, cli->uid);
	}

	bzero(buffer,BUFFER_SZ);

	while(1){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);
		if (receive > 0){
			if(strlen(buffer) > 0){
				send_message(buffer, cli->uid);

				str_trim_lf(buffer, strlen(buffer));
				printf("%s\n", buffer);
				// printf(" %s,%s", buffer, cli->name);
			}
		} else if (receive == 0 || strcmp(buffer, "exit") == 0){
			sprintf(buffer, "%s has left\n", cli->name);
			printf("%s", buffer);
			send_message(buffer, cli->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buffer, BUFFER_SZ);
	}

// If client has left the chat, close their corresponding file descriptor,
// Remove the client from the queue
// Free up the memory initially occupied by the client
// Reduce the number of clients in the chat by 1
//Detach the client thread

	close(cli->sockfd);
	queue_remove(cli->uid);
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());

	return NULL;
}



int main(int argc, char *argv[]){
	if(argc !=2){
	    printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	//Socket settings 
	//

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//
	
	serv_addr.sin_family = AF_INET;
	//

	serv_addr.sin_addr.s_addr = inet_addr(ip);
	//
	
	serv_addr.sin_port = htons(port);

   //Ignore pipe signals 
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

	// Bind 
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    	perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
    }


    // Listen
    if (listen(listenfd, 10) < 0) {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
	}
	printf("Welcome to Exliar Group Chat!\n");

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		// Check if max clients is reached
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Maximum number of clients reached. Connection Rejected: ");
			print_ip_addr(cli_addr);
			// print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		// Client settings 
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		//Add client to the queue and fork thread 
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		sleep(1);
	}
	return 0;
}