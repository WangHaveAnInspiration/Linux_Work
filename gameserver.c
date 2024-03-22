#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>

#define PORT 8080
#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 4096

#define ROW 20
#define COL 20

#define EMPTY 0
#define PLAYER 1
#define COMPUTER 2

#define DIRECTIONS 4

//cilent state
typedef struct {
	int socket_fd;
	int type;
	char msg[MAX_BUFFER_SIZE];
	char action;
} MsgInfo;

typedef struct {
	int row;
	int col;
} Move;


void set_nonblocking(int fd){
        int opts;
        opts = fcntl(fd, F_GETFL);
        if(opts < 0){
                perror("fcntl(sock,GETFL)");
                exit(1);
        }
        opts = opts|O_NONBLOCK;
        if(fcntl(fd,F_SETFL,opts)<0){
                perror("fcntl(sock, SETFL, opts)");
                exit(1);
        }


}

void initialize_board(int board[ROW][COL]) {
    	for (int i = 0; i < ROW; i++) {
        	for (int j = 0; j < COL; j++) {
            		board[i][j] = EMPTY;
        	}
    	}
}

void show_board(int client_socket, int board[ROW][COL]){
	char buffer[MAX_BUFFER_SIZE];
	int bytes_sent;	
	char mbuf[128];	

	memset(buffer, 0, sizeof(buffer));
	//clear
    	snprintf(buffer, sizeof(buffer), "\e[1;1H\e[2J   ");
	
	for (int i = 1; i <= COL; i++) {
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%2d ", i);
    	}

	strcat(buffer, "\n");
	for (int i = 0; i < ROW; i++) {
		snprintf(mbuf, sizeof(mbuf), "%2d ", i + 1);
        	for (int j = 0; j < COL; j++) {
            		if (board[i][j] == PLAYER) {
				// Assuming 1 represents "●" (Player)
				snprintf(mbuf + strlen(mbuf), sizeof(mbuf) - strlen(mbuf), " ● ");				
            		} else if (board[i][j] == COMPUTER) {
                		// Assuming 2 represents "○" (computer)
				snprintf(mbuf + strlen(mbuf), sizeof(mbuf) - strlen(mbuf), " ○ ");				
            		} else {
			 	snprintf(mbuf + strlen(mbuf), sizeof(mbuf) - strlen(mbuf), " • ");           			
			}
        	}
		strcat(mbuf, "\n");
		strncat(buffer, mbuf, sizeof(buffer) - strlen(buffer) - 1);
		memset(mbuf, 0, sizeof(mbuf));		
    	}

	bytes_sent = write(client_socket, buffer, strlen(buffer));
        if (bytes_sent == -1) {
                perror("Error sending data to client");
                return;
        }
        memset(buffer, 0, sizeof(buffer));
}


int main_input(char *buffer){
	//judge	
	if (buffer[0] == 'S' || buffer[0] == 's') {
        	return 1; // start
    	} else if (buffer[0] == 'Q' || buffer[0] == 'q') {
    	    	return 0; // quit
    	} else {
        	return -1; // error
   	}
}

int choose_play(int client_socket){
	char buffer[MAX_BUFFER_SIZE];
        int choose = 0;
        int num;
	
	while(1){
        if((num = read(client_socket, buffer, sizeof(buffer))) <= 0 ){
		    continue;
		}
        choose = main_input(buffer);
		memset(buffer, 0, sizeof(buffer));
		printf("%s",buffer);
            if(choose==0){//Quit
                close(client_socket);
                pthread_exit(NULL);
			    return 0;
            } else if(choose==1){//Start
			    return 1;
                break;
            } else {
                snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##         Invalid input!        ##\n");
                nprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##       Please input again!     ##\n");

                write(client_socket, buffer, strlen(buffer));
              		
                memset(buffer, 0, sizeof(buffer));
            }
    }
			
}

void show_menu(int client_socket){
	char buffer[MAX_BUFFER_SIZE]; 
        int bytes_sent;
	int choose = 0;
	int num;

	//clear
	snprintf(buffer, sizeof(buffer), "\e[1;1H\e[2J");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "###################################\n");       
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##    Welcome to play gobang     ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##       Below is the rule       ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##  Input position to put chess  ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##     For example:Input 1 12    ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##     Hope you can have fun     ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##       Input S/s to start      ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##       Input Q/q to quit       ##\n");
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "###################################\n");
	bytes_sent = write(client_socket, buffer, strlen(buffer));
        if (bytes_sent == -1) {
                perror("Error sending data to client");
                return;
        }
        memset(buffer, 0, sizeof(buffer));

	choose_play(client_socket);
	
}

void end_game(int client_socket){

	char buffer[MAX_BUFFER_SIZE];
        int bytes_sent;
        int num;

	memset(buffer, 0, sizeof(buffer));

        //clear
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "###################################\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##          Game is end          ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##     Enter C/c to continue     ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "##       Enter Q/q to quit       ##\n");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "###################################\n");

	
	bytes_sent = write(client_socket, buffer, strlen(buffer));
        if (bytes_sent == -1) {
               	perror("Error sending data to client");
               	return;
       	}
        memset(buffer, 0, sizeof(buffer));

}


bool check_winner(int board[ROW][COL], int sb) {
	//line
	for (int i = 0; i < ROW; i++) {
        	for (int j = 0; j <= COL - 5; j++) {
            		bool win = true;
            		for (int k = 0; k < 5; k++) {
                		if (board[i][j + k] != sb) {
                    			win = false;
                    			break;
                		}
            		}
            		if (win) {
                		return true;
            		}
        	}
    	}

	for (int j = 0; j < COL; j++) {
        	for (int i = 0; i <= ROW - 5; i++) {
            		bool win = true;
            		for (int k = 0; k < 5; k++) {
                		if (board[i + k][j] != sb) {
                    			win = false;
                    			break;
                		}
            		}
            		if (win) {
                		return true;
        	    	}
        	}
    	}

	for (int i = 0; i <= ROW - 5; i++) {
                for (int j = 0; j <= COL - 5; j++) {
                        bool win = true;
                        for (int k = 0; k < 5; k++) {
                                if (board[i + k][j + k] != sb) {
                                        win = false;
                                        break;
                                }
                        }
                        if (win) {
                                return true;
                        }
                }
        }

	for (int i = 4; i < ROW; i++) {
                for (int j = 0; j <= COL - 5; j++) {
                        bool win = true;
                        for (int k = 0; k < 5; k++) {
                                if (board[i - k][j + k] != sb) {
                                        win = false;
                                        break;
                                }
                        }
                        if (win) {
                                return true;
                        }
                }
        }
	return false;
}


int evaluate_direction(int board[ROW][COL], int player, int dir, int row, int col) {
    	int score = 0;
	int middle_row = ROW / 2;
    	int middle_col = COL / 2;
	int player_count = 0;

	const int dx[DIRECTIONS] = {1, 0, 1, 1};
	const int dy[DIRECTIONS] = {0, 1, 1, -1};

	for (int i = 0; i < 5; i++) {
        	int x = row + i * dx[dir];
        	int y = col + i * dy[dir];

        	if (x >= 0 && x < ROW && y >= 0 && y < COL) {
            		if (board[x][y] == player) {
            		    	player_count++;
				            score++;
            		}
        	}
    	}

	if (row == middle_row && col == middle_col) {
        	score += 2;
    	}

	if (player_count == 3 && score > 0) {
		score += 5;
	}

    	return score;
}


int evaluate_position(int board[ROW][COL], int row, int col) {
        int total_score = 0;
        
	const int weights[DIRECTIONS] = {3, 3, 4, 4};

	for (int dir = 0; dir < DIRECTIONS; dir++) {
                total_score += evaluate_direction(board, COMPUTER, dir, row, col) *  weights[dir];
                total_score += evaluate_direction(board, PLAYER, dir,row, col) * weights[dir];
        }

        return total_score;
}


Move computer_do(int board[ROW][COL]) {
    	Move best_move = {0, 0};
    	int max_score = -1;

    	for (int i = 0; i < ROW; i++) {
        	for (int j = 0; j < COL; j++) {
            		if (board[i][j] == EMPTY) {
                		board[i][j] = COMPUTER;
				
				    if(check_winner(board, COMPUTER)){
					    board[i][j] = EMPTY;
					    best_move.row = i;
                        best_move.col = j;
					    return best_move;
				    }

				board[i][j] = PLAYER;
				if(check_winner(board, PLAYER)){
                                        board[i][j] = EMPTY;
                                        best_move.row = i;
                                        best_move.col = j;
                                        return best_move;
                                }

				board[i][j] = COMPUTER;
                		int score = evaluate_position(board, i, j);

                		board[i][j] = EMPTY;

                		if (score > max_score) {
                    			max_score = score;
                    			best_move.row = i;
                    			best_move.col = j;
                		}
            		}
        	}
    	}

    	return best_move;
}

Move get_user_move(int client_socket){

	char buffer[MAX_BUFFER_SIZE];
	int num = 0;

	Move user_move = {0, 0};
	while(num <= 0){
		num = read(client_socket, buffer, sizeof(buffer));
	}

	sscanf(buffer, "%d %d", &user_move.row, &user_move.col);

	printf("%d %d",user_move.row,user_move.col);	
	return user_move;
}

void show_end(int client_socket, int board[ROW][COL]){
	char end_msg[MAX_BUFFER_SIZE];
        memset(end_msg, 0, sizeof(end_msg));
        if(check_winner(board, COMPUTER)){
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "###################################\n");
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "##           You lose !          ##\n");
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "###################################\n");
        } else {
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "###################################\n");
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "##           You win !           ##\n");
                snprintf(end_msg + strlen(end_msg), sizeof(end_msg) - strlen(end_msg), "###################################\n");
        }
        write(client_socket, end_msg, strlen(end_msg));
	sleep(1);
}

void handle_client(int client_socket) {
	int board[ROW][COL];

	Move user_move = {0, 0};
	Move computer_move = {0, 0};
	if (client_socket < 0) {
        	perror("Invalid client socket");
        	return;
    	}
	
	//pre wirte menu and play method
	initialize_board(board);
	show_board(client_socket, board);
	

	//do chess
        int turn; 
        while (!check_winner(board, COMPUTER) && !check_winner(board, PLAYER)) {
		turn = PLAYER;

		//player input
		Move user_move = {0, 0};
		Move compuer_move = {0, 0};
		user_move = get_user_move(client_socket);
		char next[MAX_BUFFER_SIZE];
		//change
		board[user_move.row - 1][user_move.col - 1] = turn;
		show_board(client_socket, board);

		if(check_winner(board, COMPUTER) || check_winner(board, PLAYER)){
			break;
		}

		turn = COMPUTER;
		computer_move = computer_do(board);
		board[computer_move.row][computer_move.col] = turn;
		show_board(client_socket, board);

	}
	
	show_end(client_socket, board);
	end_game(client_socket);

}


void* start_routine(void* arg) {
    	int client_socket = *((int*)arg);
	char buffer[MAX_BUFFER_SIZE];
	int outcome = 1;

	show_menu(client_socket);
	while(outcome==1){
		
		handle_client(client_socket);
		outcome = choose_play(client_socket);
	}
	

    	close(client_socket);
    	pthread_exit(NULL);
}

int main() {
	int server_socket, epoll_fd, client_socket, events_count, conn_socket;
	struct epoll_event event, events[MAX_EVENTS];
	struct sockaddr_in server_addr;
	pthread_t tid;

	char buf[MAX_BUFFER_SIZE];

	signal(SIGPIPE, SIG_IGN);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(server_socket,(const struct sockaddr *)&server_addr,sizeof(struct sockaddr_in))==-1){
                perror("cannot bind");
                exit(EXIT_FAILURE);
        }

        listen(server_socket, 1);

        epoll_fd = epoll_create(10);
        if(epoll_fd == -1){
                perror("epoll cannot create");
                exit(EXIT_FAILURE);
        }

        event.events = EPOLLIN;
        event.data.fd = server_socket;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1){
                perror("Epoll control failed");
                exit(EXIT_FAILURE);
        }


    	while (1){

		events_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if(events_count == -1){
            perror("epoll wait");
            exit(EXIT_FAILURE);
        }

        	for (int i = 0; i < events_count; i++) {
			    if (events[i].data.fd == server_socket) {
				    conn_socket = accept(server_socket, NULL, NULL);
                    if(conn_socket == -1){
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    set_nonblocking(conn_socket);
                	event.events = EPOLLIN | EPOLLET; 
                	event.data.fd = conn_socket;
				    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_socket, &event)==-1){
					    perror("epoll_ctl: conn_socket");
                        exit(EXIT_FAILURE);
				    }

				if (pthread_create(&tid, NULL, start_routine, &conn_socket) != 0) {
                    perror("pthread_create");
                    exit(EXIT_FAILURE);
                }

			}
		}
	}

	close(server_socket);
	close(epoll_fd);

	return 0;
}





