#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 4096

#define ROW 20
#define COL 20

#define EMPTY 0
#define PLAYER 1
#define COMPUTER 2

typedef struct {
        int row;
        int col;
} Move;

void read_menu(int client_socket){

	char buffer[MAX_BUFFER_SIZE]; //read
        char user_input[MAX_BUFFER_SIZE]; //write

	ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received == -1) {
                perror("Error reading from server");
                exit(EXIT_FAILURE);
        } else if(bytes_received == 0){
		while(bytes_received == 0){
			read(client_socket, buffer, sizeof(buffer));
		}
	} else {

                printf("%s", buffer);

                printf("Enter your choose: ");
		fflush(stdout);
		memset(buffer, 0, sizeof(buffer));

		// Get user input
		fgets(user_input, sizeof(user_input), stdin);
		user_input[strcspn(user_input, "\n")] = '\0';

                // Send user input to the server
		write(client_socket, user_input, strlen(user_input));
		while(1){
			int num = 0;
			if(user_input[0]=='s' || user_input[0]=='S'){
				break;
			} else if(user_input[0]=='q' || user_input[0]=='Q'){
				printf("Quit game now\n");
				close(client_socket);
				exit(0);
			} else {
				if((num = read(client_socket, buffer, sizeof(buffer))) < 0){
                                        num = read(client_socket, buffer, sizeof(buffer));
                                }
                                printf("%s", buffer);
				printf("Please input again: ");
				memset(buffer, 0, sizeof(buffer));
	
				fgets(user_input, sizeof(user_input), stdin);
                		user_input[strcspn(user_input, "\n")] = '\0';
				
				write(client_socket, user_input, strlen(user_input));
				memset(buffer, 0, sizeof(buffer));
			}
		}
	
	}
}

Move check_input(){
	char input_buffer[MAX_BUFFER_SIZE];
	Move user_move = {0, 0};

	if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
		printf("##       Invalid input format!       ##\n");
		printf("Please input again: ");
		user_move.row = user_move.col = -1;
                return user_move;
	}
	
	if (sscanf(input_buffer, "%d %d", &user_move.row, &user_move.col) != 2) {
        	printf("##       Invalid input format!       ##\n");
		printf("Please input again: ");
		user_move.row = user_move.col = -1;
        	return user_move;
    	}

	if (user_move.row < 1 || user_move.row > ROW || user_move.col < 1 || user_move.col > ROW) {
        	printf("##       Input position error !       ##\n");
		printf("Please input again: ");
		user_move.row = user_move.col = -1;
                return user_move;
    	}

    	return user_move;

}

int get_end(char* msg){

	int i = 0;
	while(i<strlen(msg)){
		if(msg[i]=='Y'){
			return 1;
		}
		i++;
	}
	return 0;
}

void read_chess(int client_socket){
	char buffer[MAX_BUFFER_SIZE]; //read
	Move user_move;
	int end = 0;
	sem_t *sem_read, *sem_write;

	sem_read = sem_open("semread", O_CREAT, 0644, 1);
	sem_write = sem_open("semwrite", O_CREAT, 0644, 0);

	
	pid_t pid = fork();


	if (pid < 0) {
        	perror("fork error");
        	exit(EXIT_FAILURE);
    	}
	if (pid == 0) {
		while (1) {
            Move user_move;	

			sem_wait(sem_write);	

			printf("It's black step: \n");
            		printf("Please you input: ");
            		fflush(stdout);
			bool right = false;
			

            do {
                user_move = check_input();
                right = user_move.row != -1 && user_move.col != -1;
            } while (!right);

            char buffer[MAX_BUFFER_SIZE];
			snprintf(buffer, sizeof(buffer), "%d %d", user_move.row, user_move.col);
        	write(client_socket, buffer, strlen(buffer));

			sem_post(sem_read);
        }

	} else {
		char buffer[MAX_BUFFER_SIZE];
        int end = 0;
		sem_wait(sem_read);
	
		while (end == 0) {
            int num = 0;
		
            memset(buffer, 0, sizeof(buffer));

            num = read(client_socket, buffer, sizeof(buffer));

            printf("%s", buffer);

            fflush(stdout);
			end = get_end(buffer);

            if (end == 1) {
				break;
            }
			
			sem_post(sem_write);

			sem_wait(sem_read);

            num = read(client_socket, buffer, sizeof(buffer));
			printf("%s", buffer);
			fflush(stdout);

			end = get_end(buffer);

            if (end == 1) {
				break;
			}
        }
		kill(pid, SIGTERM);
    	sem_unlink("semread");
		sem_unlink("semwrite");

	}
}

int main() {
	int client_socket;
	struct sockaddr_in server_addr;
	char buffer[MAX_BUFFER_SIZE]; //read
	char user_input[MAX_BUFFER_SIZE]; //write
	int num;

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1) {
        	perror("Socket creation failed");
        	exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Connection failed");
		exit(EXIT_FAILURE);
	}

	read_menu(client_socket);

	while(1){
		read_chess(client_socket);

		read_menu(client_socket);
	}

	close(client_socket);
	return 0;
}

