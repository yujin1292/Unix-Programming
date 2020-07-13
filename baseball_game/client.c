#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


#define MAXLINE     1000
#define NAME_LEN    20

char *EXIT_STRING = "exit";

// 소켓 생성 및 서버 연결, 생성된 소켓리턴
int tcp_connect(int af, char *servip, unsigned short port);
void errquit(char *mesg) { perror(mesg); exit(1); }

int main(int argc, char *argv[]) {
	
	char bufname[NAME_LEN];	// 이름
	char bufmsg[MAXLINE];	// 메시지부분
	char bufall[MAXLINE + NAME_LEN];
	int maxfdp1;	// 최대 소켓 디스크립터
	int s;		// 소켓
	int namelen;	// 이름의 길이
	fd_set read_fds;
	time_t ct;
	struct tm tm;

	if (argc != 4) {
		printf("사용법 : %s sever_ip  port name \n", argv[0]);
		exit(0);
	}
				// server ip  , port 
	s = tcp_connect(AF_INET, argv[1], atoi(argv[2]));
	if (s == -1)
		errquit("tcp_connect fail");

	puts("서버에 접속되었습니다.");
	maxfdp1 = s + 1; //최대 소켓디스크립터 값 증가 

 

	FD_ZERO(&read_fds); // read_fds 0으로 초기화 

	while (1) {
		FD_SET(0, &read_fds); // 0 을 read_fds 에 추가
		FD_SET(s, &read_fds); // s 을 read_fds 에 추가 
 
		// 변경테스트할 디스크립터 개수, readfds 인자 , writefds 인자, errorfds인자, timeout 인자 
		if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");
		if (FD_ISSET(s, &read_fds)) { // s 가 read_fds에 있어야 출력 
			int nbyte;
			// 읽어들임 
			if ((nbyte = recv(s, bufmsg, MAXLINE, 0)) > 0) {
				
				bufmsg[nbyte] = 0;
				write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
				for(int i = 0 ; i < strlen(bufmsg) ; i++){
					printf("%c",bufmsg[i]);
					if(bufmsg[i] == '[' && bufmsg[i+1] =='S') 
						fprintf(stderr, "\033[1;36m");

				}
				
				fprintf(stderr, "\033[1;32m");	//글자색을 녹색으로 변경
				fprintf(stderr, "%s>", argv[3]);//내 닉네임 출력( 출력창 )


			}
		}
		if (FD_ISSET(0, &read_fds)) {
			//내가 쓰는겨
			if (fgets(bufmsg, MAXLINE, stdin)) { // 표준출력에서 읽어서 bufmsg에 저장 
				fprintf(stderr, "\033[1;33m"); //글자색을 노란색으로 변경
				fprintf(stderr, "\033[1A"); //Y좌표를 현재 위치로부터 -1만큼 이동
				ct = time(NULL);	//현재 시간을 받아옴
				tm = *localtime(&ct);
				//bufall에 형식에 맞게 출력 
				sprintf(bufall, "[%02d:%02d:%02d]%s>%s", tm.tm_hour, tm.tm_min, 
					tm.tm_sec, argv[3], bufmsg);//메시지에 현재시간 추가
				//buffall 을 전송 
				if (send(s, bufall, strlen(bufall), 0) < 0)
					puts("Error : Write error on socket.");
				if (strstr(bufmsg, EXIT_STRING) != NULL ) {
					puts("See you later~");
					close(s);
					exit(0);
				}
			}
		}
	} // end of while
}

int tcp_connect(int af, char *servip, unsigned short port) { // 소켓을 생성하고 연결함 
	struct sockaddr_in servaddr;
	int  s;
	// 소켓 생성
	if ((s = socket(af, SOCK_STREAM, 0)) < 0)
		return -1; // 에러

	// 채팅 서버의 소켓주소 구조체 servaddr 초기화
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	// 연결요청
	if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr))
		< 0)
		return -1; // 연결 실패 
	return s;
}

