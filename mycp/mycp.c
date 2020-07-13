#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

void copy(char * source , char * copy ){
	
	int rfd = open(source, O_RDONLY);
	int wfd = open(copy, O_CREAT|O_WRONLY|O_TRUNC , S_IRUSR|S_IWUSR);
	int n;
	char buf[10];	
	

	if( rfd == -1){
		perror("open source err");
		exit(1);
	}

	if( wfd == -1){
		perror("open copy err");
		exit(1);
	}

	while( (n = read(rfd, buf, 4)) > 0 ){
		if( write(wfd, buf, n) != n )
			perror("Write");
		if( n == -1 )
			perror("Read");
	}

	close(rfd);
	close(wfd);

}

void symlink_cp(char* source,char *path){
	
	symlink(source, path);
}



void go_copy(int i, int n, int s, int u, int is_dir , char *source, char* cp){
	
	int check;
	int exist = 0;
	int go = 0;
	char aws[10];
	int update = 0;

	char path[50]; //복사할 파일의 정확한 위치 

	//cp source cp
	if(is_dir){ //cp 가 디렉토리임
		
		strcpy(path, cp);
		strcat(path,"/");
		strcat(path, source); // path ==> 디렉토리/source이름
		

	}
	else{ // cp가 복사본 이름
		strcpy(path, cp);
	}

	//cp 파일 존재 확인
	if( (check = access(path, 0) ) == 0 ) // 존재
		exist = 1;


	// 존재함 
	if( exist ==1 ){
		if( s== 1) //같은파일이름파일존재하면 symlink 불가
			printf("cp: cannot create symbolic link 'test3.txt' to 'test.txt': File exists\n");

		if(u){ //update 확인해보고 
			
			struct stat buf1, buf2;
			stat(source, &buf1);
			stat(path,&buf2);
			time_t t1 = buf1.st_ctime;
			time_t t2 = buf2.st_ctime;
			if( t1 > t2 ){ // go
				//printf("goupdate\n");
				if( n == 1 ) // 존재하니까 복사x
					go = 0;
			
				else if( i == 1) { // n == 0 
					printf("cp : overwrite '%s' ?", path);
					scanf("%s", aws);
					if( (aws[0] == 'y' || aws[0] == 'Y')){
							go = 1;
		
					}
					else
						go = 0;
				}

				else{
					go = 1;
				}
	
			}
			else {
				//printf("noupdate\n");
				go = 0; //복사필요 x 
			}
			

		}
		else { //update flag 없음 
			
			if( n == 1 ) // 존재하니까 복사x
				go = 0;
			
			else if( i == 1) { // n == 0 
				printf("cp : overwrite '%s' ?", path);
				scanf("%s", aws);
				if( (aws[0] == 'y' || aws[0] == 'Y')){
						go = 1;
		
				}
				else
					go = 0;
			}

			else{
				go = 1;
			}
		}
		
	}
	else
		go = 1;
	
	// copy start! 
	if(go){
		if(s) //복사대신 심볼릭 링크
			symlink(source,path);
		
		else
			copy(source, path);
	}
}


int main(int argc, char * argv[]){
	extern int optopt , optind;
	int nn;


	int i,n,s,u;
	i = n = s = u =0;

	char * source;
	char * copy;
	char * copypath;
	
	if( argc < 3 ){
		printf("err\n");
		exit(1);
	}
	
	


	while((nn=getopt(argc, argv, "insu")) != -1){
		if(optopt != 0 )
			return 0;
		switch (nn) {
			case 'i' : // overwrtie 하기전에 물어보기 
				i = 1;
				break;
		
			case 'n' : // 물어보지말고 걍 overwirte 금지 
				n = 1;
				break;			

			case 's' : // symbolic link 
				s = 1;
				break;
			case 'u' :
				u = 1;
				break;
		}

	}
	
	//printf("%d %d \n", argc, optind);

	if( argc < (optind +2 ) ){
		printf("missing\n");
		return 0;
	}
	else if( argc == (optind+ 2) ){
		
		struct stat buf;
		stat(argv[argc-1], &buf);
		int kind = buf.st_mode & S_IFMT;
		int is_dir;
		if( kind == S_IFDIR)// cp 파일1 디렉토리
			is_dir =1;
		else // cp 파일  파일
			is_dir = 0;
		go_copy(i,n,s,u,is_dir, argv[argc-2] , argv[argc-1]);


	}
	else{
	// cp 파일1 파일2 파일3 ... 디렉토리
		struct stat buf;
		for(int idx = optind ; idx < argc-1 ; idx++){				
			stat(argv[idx], &buf);
			int kind = buf.st_mode & S_IFMT;
			
			if( kind == S_IFDIR){
				printf("err");
				return 0;
			}
	
			go_copy(i,n,s,u,1, argv[idx] , argv[argc-1]);
			
		}


	}


	return 0;
}

