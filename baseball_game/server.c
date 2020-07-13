#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAXLINE  511
#define MAX_SOCK 1024 // 솔라리스의 경우 64

char *EXIT_STRING = "exit";	// 클라이언트의 종료요청 문자열
char *START_STRING_P = "Connected to chat_server You are Player \n";
char *START_STRING_A = "Connected to chat_server You are audiance \n";
char *DASH_LINE = "=====================================================\n";
char *RESTART = "start";
// 클라이언트 환영 메시지
int maxfdp1;				// 최대 소켓번호 +1
int num_user = 0;			// 채팅 참가자 수
int num_chat = 0;			// 지금까지 오간 대화의 수
int clisock_list[MAX_SOCK];		// 채팅에 참가자 소켓번호 목록
char ip_list[MAX_SOCK][20];		//접속한 ip목록
int listen_sock;			// 서버의 리슨 소켓


int over = 1;				//게임 진행상황을 알려주는flag
int turn = -1;				//사용자의 순서를 알려주는 변수
int round_turn = 0;			//몇번째 게임인지 알려주는 변수
int fd;					//로그파일출력을위한 

int nan_num = 0;			// 난수저장
int num[3];				// 저장한 난수 자리별로 저장 


void makeNumber();			//난수생성 함수
void Start();				//게임시작 함수
void addClient(int s, struct sockaddr_in *newcliaddr);// 새로운 채팅 참가자 처리
int getmax();					// 최대 소켓 번호 찾기
void removeClient(int s);			// 채팅 탈퇴 처리 함수
int tcp_listen(int host, int port, int backlog); // 소켓 생성 및 listen
void errquit(char *mesg) { perror(mesg); exit(1); }
char* baseball(char* str, int num[3]);


time_t ct;
struct tm tm;

void *thread_function(void *arg) { //명령어를 처리할 스레드
	int i;
	printf("명령어 목록 : help, num_user, num_chat, ip_list\n");
	while (1) {
		char bufmsg[MAXLINE + 1];
		fprintf(stderr, "\033[1;32m"); //글자색을 녹색으로 변경
		printf("server>"); //커서 출력
		fgets(bufmsg, MAXLINE, stdin); //명령어 입력
		if (!strcmp(bufmsg, "\n")) continue;   //엔터 무시
		else if (!strcmp(bufmsg, "help\n"))    //명령어 처리
			printf("help, num_user, num_chat, ip_list\n");
		else if (!strcmp(bufmsg, "num_user\n"))//명령어 처리
			printf("현재 참가자 수 = %d\n", num_user);
		else if (!strcmp(bufmsg, "num_chat\n"))//명령어 처리
			printf("지금까지 오간 대화의 수 = %d\n", num_chat);
		else if (!strcmp(bufmsg, "ip_list\n")) //명령어 처리
			for (i = 0; i < num_user; i++)
				printf("%s\n", ip_list[i]);
		else //예외 처리
			printf("해당 명령어가 없습니다.help를 참조하세요.\n");
	}
}

int main(int argc, char *argv[]) {



	struct sockaddr_in cliaddr;
	char buf[MAXLINE + 1]; //클라이언트에서 받은 메시지
	char sbuf[MAXLINE + 1];
	int i, j, nbyte, accp_sock, addrlen = sizeof(struct
		sockaddr_in);
	fd_set read_fds;	//읽기를 감지할 fd_set 구조체
	pthread_t a_thread;

	if (argc != 2) {
		printf("사용법 :%s port\n", argv[0]);
		exit(0);
	}
	
	//난수 발생 
	//makeNumber();

	//printf("답 : %d%d%d\n", num[0], num[1], num[2]);

	// tcp_listen(host, port, backlog) 함수 호출
	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
	//접속을 기다린다 
	

	//서버 명령어를 관리할 스레드 생성
	pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
	

	while (1) {
		
		FD_ZERO(&read_fds); // read_fds 를 초기화함 
		FD_SET(listen_sock, &read_fds); // listen_sock을 read_fds에 추가 
		for (i = 0; i < num_user; i++)
			FD_SET(clisock_list[i], &read_fds); // 클라이언트들의 socket 들을 read_fds에 추가 

		maxfdp1 = getmax() + 1;	// 최대 소켓번호 maxfdp1 재 계산 

		// 변경사항있을때까지 기다림 
		// 변경테스트할 디스크립터 개수, readfds 인자 , writefds 인자, errorfds인자, timeout 인자 
		if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0) // 실패시 -1 반환 
			errquit("select fail");

		//사용자 추가로 들어왔을 경우 
		if (FD_ISSET(listen_sock, &read_fds)) { // 서버의 리슨 소켓인 listen_sock 이 reads_fds 에 들어있어야 출력
			
			accp_sock = accept(listen_sock,(struct sockaddr*)&cliaddr, &addrlen); // accept 하면 새로운 fd 반환..
			if (accp_sock == -1) 
				errquit("accept fail"); // 에러시 -1 반환 
				
			
			addClient(accp_sock, &cliaddr);


			ct = time(NULL);			//현재 시간을 받아옴
			tm = *localtime(&ct);
			write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
			printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(stderr, "\033[33m");//글자색을 노란색으로 변경
			if(num_user<3){
				
				char msg[] = "새로운 사용자가 입장했습니다\n";
				for (int j = 0; j < num_user; j++) {
					send(clisock_list[j], msg, strlen(msg), 0);
				}
	
				printf("사용자 1명 추가. 현재 참가자 수 = %d\n", num_user);
				// 클라이언트에게 시작 메시지 전달 
				send(accp_sock, START_STRING_P, strlen(START_STRING_P), 0); 
				
			}
			if( num_user == 3 ){
				printf(" 현재 3명이 입장했습니다. ");
				printf(" 게임을 시작합니다 !\n");
				char msg[] = "새로운 사용자가 입장했습니다\n";
				for (int j = 0; j < num_user; j++) {
					send(clisock_list[j], msg, strlen(msg), 0);
				}

				Start(); // 게임을 시작함 
				// 클라이언트에게 시작 메시지 전달 
				send(accp_sock, START_STRING_P, strlen(START_STRING_P), 0);
			}
			if(num_user>3){
				printf("관전자 추가. 현재 참가자 수 = %d\n", num_user);
				char msg[] = "새로운 사용자가 입장했습니다\n";
				for (int j = 0; j < num_user; j++) {
					send(clisock_list[j], msg, strlen(msg), 0);
				}
				// 클라이언트에게 시작 메시지 전달 
				send(accp_sock, START_STRING_A, strlen(START_STRING_A), 0);
			}
		
			fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
				fprintf(stderr, "server>"); //커서 출력

		}

		// 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
		for (i = 0; i < num_user; i++) {
			
			if (FD_ISSET(clisock_list[i], &read_fds)) { // 클라이언트들의 소켓들이 readfs에 들어있어야 함 
			
				num_chat++;					//총 대화 수 증가
				nbyte = recv(clisock_list[i], buf, MAXLINE, 0); // 클라이언트 소켓에서부터 데이터 받기 
				

				if (nbyte <= 0) { 
					removeClient(i);	// 클라이언트의 종료
					continue;
				}
			

				buf[nbyte] = 0; //읽어온 배열 문자열로 변환 
				
				// 종료문자 처리
				// exit 이라는 문자열이 들어있으면 
				if (strstr(buf, EXIT_STRING) != NULL ) {
					removeClient(i);	// 클라이언트의 종료
					continue;
				}


				if (over == 0) { // 게임진행중 

					if ((i != -1) && (i != turn)) { 
						strcpy(sbuf, "");
					}
					else{
						// 게임 진행중 일때만 숫자추리결과 저장 
						sprintf(sbuf, "%s", baseball(buf, num));
						
					}




				}
				else {  // 게임중 아님 
					strcpy(sbuf, "");
				}

				// 모든 채팅 참가자에게 메시지 방송
				for (j = 0; j < num_user; j++){
					//client가 말한것 전달 (말한 클라이언트도 서버한테 또 받음) 
					send(clisock_list[j], buf, nbyte, 0);

					//결과 출력....
					send(clisock_list[j], sbuf, strlen(sbuf), 0);
					
					if( over == 0 && turn == j){
					//게임 진행중이고, 현재 차례인 클라이언트에게 순서 알림
						char msg[] = "[System] 당신 차례입니다\n";
						send(clisock_list[turn], msg, strlen(msg), 0);
					}
				}
				printf("\n%s\n", sbuf);// server 에 bass ball 결과 출력  
				
				write(fd, sbuf,strlen(sbuf)); //로그파일에 대화내용 저장

				printf("\033[0G");		//커서의 X좌표를 0으로 이동
				fprintf(stderr, "\033[97m");//글자색을 흰색으로 변경
				printf("%s", buf);	  // 클라이 언트가 말한 메시지를 메시지 출력
				write(fd,buf,strlen(buf));
				fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
				fprintf(stderr, "server>"); //커서 출력
				
				if(strstr(buf, RESTART) != NULL ){
					Start();
					break;
				}
			}
		}
	
	}  

	return 0;
}

void makeNumber(){
	//난수 발생
	while(1)

        {

                time_t t;

                srand ((unsigned) time(&t));

                nan_num = (rand() % 900) + 100;

                num[0] = nan_num / 100;

                num[1] = (nan_num % 100) / 10;

                num[2] = nan_num % 10;

                if ( num[0] != 0 && num[1] != 0 && num[2] != 0 && num[0] != num[1] &&
				 num[0] != num[2] && num[1] != num[2] )

                break;

        }
}

void Start(){
	// 로그파일 텍스트 파일 저장을 위해 파일 기술자 open 
	fd = open("log.txt", O_RDWR|O_CREAT|O_APPEND,0644);
	char buf[100];
	round_turn++; // 시작된 게임 수 증가 
	if(round_turn == 1) // 첫 게임이면 log에 "==..==" 텍스트 출력 
		write(fd, DASH_LINE, strlen(DASH_LINE));



	turn = 0; // 시작 순서 초기화 

	//buf에 문자열 저장 후 log.txt에 출력 
	sprintf(buf,"------------------------------------\n");
	write(fd, buf, strlen(buf)); 
	sprintf(buf,"[round %d ] GAME START \n",round_turn);
	write(fd, buf, strlen(buf));

	sprintf(buf,"\n\n%d Player's turn \n",turn+1);
	write(fd, buf, strlen(buf));

	//server에 출력
	printf("%s",buf);


	if (num_user >= 3) {
		makeNumber();
		over = 0;
		printf("게임을 시작합니다\n");
		printf("답 : %d%d%d\n", num[0], num[1], num[2]);

		char msg[] = "게임을 시작합니다! 순서대로 맞춰보세요\n";
		for (int j = 0; j < num_user; j++) {

			send(clisock_list[j], msg, strlen(msg), 0);

		}
		for (int j = 0; j < 3; j++) {

			sprintf(msg, "%s %d %s","당신은", j+1,"번째 순서 입니다\n");
			send(clisock_list[j], msg, strlen(msg), 0);

		}
		for (int j = 3; j < num_user; j++) {

			sprintf(msg, "%s","관중입니다. 게임에는 참여할수없습니다.\n");
			send(clisock_list[j], msg, strlen(msg), 0);

		}
		char msg2[] = "[System] 당신 차례입니다\n";
		send(clisock_list[0], msg2, strlen(msg2), 0);

	}
	else{
		char msg[] = "인원이 부족합니다\n";
		for (int j = 0; j < num_user; j++) {

			send(clisock_list[j], msg, strlen(msg), 0);

		}

	}
	
	
}




// 새로운 채팅 참가자 처리
void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
	write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");	//글자색을 노란색으로 변경
	printf("new client: %s\n", buf);//ip출력
	// 채팅 클라이언트 목록에 추가
	clisock_list[num_user] = s;
	strcpy(ip_list[num_user], buf);
	num_user++; //유저 수 증가
}

// 채팅 탈퇴 처리
void removeClient(int s) {
	close(clisock_list[s]);


	for( int i = s ; i < num_user -1 ; i++ ){
			clisock_list[i] = clisock_list[i+1];
			strcpy(ip_list[i], ip_list[i+1]);
	}
	num_user--; //유저 수 감소

	if( over == 0  ){
		if( s < 3){
			char msg[] = "게임이 중단됐습니다. 새게임은 strat 를 입력해주세요\n";
			printf("게임이 중단됐습니다\n");
			char buf[100];
			sprintf(buf,"[round%d ] is Closed \n\n",round_turn);
			write(fd, buf, strlen(buf));
			close(fd);
			for (int j = 0; j < num_user; j++) {

				send(clisock_list[j], msg, strlen(msg), 0);

			}
			over = 1;
			turn = -1;
		}

	}

	ct = time(NULL);			//현재 시간을 받아옴
	tm = *localtime(&ct);
	write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m");//글자색을 노란색으로 변경
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("채팅 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_user);
	
	fprintf(stderr, "\033[32m");//글자색을 녹색으로 변경
	fprintf(stderr, "server>"); //커서 출력
}

// 최대 소켓번호 찾기
int getmax() {
	// Minimum 소켓번호는 가정 먼저 생성된 listen_sock
	int max = listen_sock;
	int i;
	for (i = 0; i < num_user; i++)
		if (clisock_list[i] > max)
			max = clisock_list[i];
	return max;
}

// listen 소켓 생성 및 listen
int  tcp_listen(int host, int port, int backlog) {
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket fail");
		exit(1);
	}
	// servaddr 구조체의 내용 세팅
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind fail");  exit(1);
	}
	// 클라이언트로부터 연결요청을 기다림
	listen(sd, backlog);
	return sd;
}

char* baseball(char* str, int num[3]){
		// 입력한값 	// 정답
	
	char* buf;
	buf = malloc(sizeof(char)*100);

	int i,j; /* for문에들어가는 변수*/

        int count[3];

        int s=0,b=0,o=0; //스트라이크, 볼, 아웃 카운터 변수
	

	int idx = 0;
	for(idx = 0 ; idx < strlen(str) ; idx ++){
		
		if( str[idx] == '>')
			break;
	}
	
	idx++;
	
	int k;


	if( idx + 2 > strlen(str)) 
		k = -1;
	else
		k = (str[idx]-'0') * 100 + (str[idx+1]-'0') * 10 + (str[idx+2]-'0');


        count[0] = k / 100;
        count[1] = (k % 100) / 10;
        count[2] = (k % 100) % 10;

        for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			if(i == j){
				if(num[i] == count[j])
					s++;
			}
			else if(num[i] == count[j]){
				b++;
			}
		}
	}
	o = 3 - (s + b);      

	if( s == 3 ){ // 사용자가 정답을 맞춤 

		sprintf(buf, "[System] 3스트라이크!! 정답입니다!\n게임을 다시시작하려면 start 를 입력해주세요\n");
		over = 1;
		turn - 1;
		char buf[100];
		// 형식에 맞게 문자열 저장후 출력 
		sprintf(buf,"[round %d ] is Closed \n %d's Player is win!! \n",round_turn , turn+1);
		write(fd, buf, strlen(buf));
		close(fd); // 파일기술자 close 
	}
	else{
		turn++;
		turn = turn %3;
		sprintf(buf, "[System] %d -> %d스트라이크 %d볼 %d아웃입니다.\n", k, s, b, o);
		char buf[100];
		sprintf(buf,"\n\n%d Player's turn \n",turn+1);
		write(fd, buf, strlen(buf));
	}
	return buf;
}




