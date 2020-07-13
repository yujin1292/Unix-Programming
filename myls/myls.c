#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>


void print_mode(int mode) {

	int kind = mode & S_IFMT;
	if(kind == S_IFDIR) //파일의 종류에맞게 출
		printf("d");
	else if( kind == S_IFBLK)
		printf("b");
	else if( kind == S_IFCHR)
		printf("c");
	else if(kind == S_IFLNK)
		printf("l");
	else if( kind == S_IFSOCK)
		printf("s");
	else if( kind == S_IFIFO)
		printf("p");
	else
		printf("-");
	

	int read = S_IREAD;
	int write= S_IWRITE;
	int exec = S_IEXEC;

	for(int i = 0 ; i < 3 ; i++ ){
		if(( mode & read ) != 0 )
			printf("r");
		else
			printf("-");

		if( (mode & write ) != 0 )
			printf("w");
		else
			printf("-");
		if( (mode & exec ) !=0 )
			printf("x");
		else
			printf("-");

		read = read >> 3;
		write = write >> 3;
		exec = exec >> 3; //3비트 shift 하면서 user->group->other
	}
	printf(" ");
}

void printFILE(struct stat buf,char * name , int l, int i ,int A, int a, int G , int Q,int m,int F){
	//inode i == 1일때
	if(i)
		printf("%d ", (int)buf.st_ino);

	//접근권한 l==1
	// link 수 l==1
	// 소자의 사용자명 l==1
	// 파일 소유자의 그룹명 l==1
	// 파일크기 ( 바이트) l==1
	// 파일이 마지막으로 변경된 시간 l==1
	if(l){
		print_mode((int)buf.st_mode);	
		printf("%d ", (int) buf.st_nlink);

		struct passwd *pw = getpwuid(buf.st_uid);
		printf("%s ", pw->pw_name );
		
		if( !G){
			
			gid_t group_id = buf.st_gid;
			struct group* group_entry;
			group_entry = getgrgid(group_id);
			printf("%s ", group_entry->gr_name);

		}
		printf("%6d ", (int)buf.st_size);
		struct tm * tm;
		char temp[257];
		time_t t = buf.st_ctime;
		tm = localtime(&t);
		strftime(temp, sizeof(temp),"%m월 %m %H:%M" , tm);
		printf("%s ", temp);
	
		
	}	
	// 파일명
	if(Q){
		printf("\"%s",name);
		if(F){
			int kind = (int)buf.st_mode & S_IFMT;
			if(kind == S_IFDIR) //파일의 종류에맞게 출
				printf("/");
			else if(kind == S_IFLNK)
				printf("@");
			else if( kind == S_IFSOCK)
				printf("=");
			else if( kind == S_IFIFO)
				printf("|");
		}
		printf("\"");
	}
	else{
		printf("%s", name);
		if(F){
			int kind = (int)buf.st_mode & S_IFMT;
			if(kind == S_IFDIR) //파일의 종류에맞게 출
				printf("/");
			else if(kind == S_IFLNK)
				printf("@");
			else if( kind == S_IFSOCK)
				printf("=");
			else if( kind == S_IFIFO)
				printf("|");

		}
	}

}

void printDIR(char * dir , int l , int i, int A, int a, int G ,int Q ,int m,int F){
	//하나씩 읽어서 printFILE 하기.
	DIR *dp;
	struct dirent *dent;
	if( (dp = opendir(dir)) == NULL ){
		perror("opendir");
		exit(1);
	}
	int item = 0;
	while( (dent = readdir(dp) )) {
		item ++;
	}
	rewinddir(dp);
	
	while( (dent = readdir(dp) )) {
		item --;
		struct stat sbuf;
		char path[BUFSIZ];
		char name[BUFSIZ];
		strcpy(path , dir);
		strcat(path,"/");
		sprintf(name, "%s", dent->d_name);
		strcat(path,name);		
		lstat(path,&sbuf);


		if( strcmp( name, ".") ==0 ){ // -a 일때만 출력
			if(a == 1){
			
				
				printFILE(sbuf, dent->d_name,  l,i,A,a,G,Q,m,F);

				if(!l){
					
					if(m && item)
						printf(", ");
					else
						printf(" ");

				}
				else
					printf("\n");
			}
		}
		else if( strcmp( name, "..") ==0 ){ // -a 일때만 출력
			if(a == 1){
			
				
				printFILE(sbuf, dent->d_name,  l,i,A,a,G,Q,m,F);

				if(!l){
					
					if(m && item)
						printf(", ");
					else
						printf(" ");

				}
				else
					printf("\n");
			}
		}
		else if( name[0] == '.' ){ //숨김파
			
			if( a == 1 || A == 1 ){
				
				printFILE(sbuf, dent->d_name,  l,i,A,a,G,Q,m,F);

				if(!l){
					
					
					if(m && item)
						printf(", ");
					else
						printf(" ");

				}
				else
					printf("\n");
			}

		}
		else{
			printFILE(sbuf, name ,  l,i,A,a,G,Q,m,F);

			
			if(!l){
				
				if(m && item)
					printf(", ");
				else
					printf(" ");

			}
			else
				printf("\n");
		}
	}
	
	if(!l)	
		printf("\n");
}


int main(int argc, char *argv[]){
	int n;
	struct stat buf;
	int kind;
	int l, i, A, a, G , Q,m,F;
	l = i = A = G =a = Q=m=F= 0;
	extern int optopt, optind;

	

	while((n=getopt(argc, argv, "liAaGoQmF")) != -1){
		if(optopt != 0)
			return 0;

		switch (n) {
			case 'l' : 
				l = 1;
				m  = 0;
				break;
			case 'i' : 
				i = 1;
				break;
			
			case 'A' : 
				A = 1;
				break;
			
			case 'a' : 
				a = 1;
				break;
			case 'o' :
				G = 1;
				l = 1;
				break;
			case 'G' : 
				G = 1;
				break;	
			case 'Q' :
				Q = 1;
				break;	
			case 'm' :
				m = 1;
				l = 0;
				break;
			case 'F' :
				F=1;
				break;

		}

	}



	if( argc  == optind ){
		char* cwd = getcwd(NULL, BUFSIZ);
		//printf("%s", cwd);
		printDIR(cwd, l,i,A,a,G,Q,m,F);
	}

	else{		
		lstat(argv[argc-1], &buf);
	
		kind = buf.st_mode & S_IFMT;

		if( kind == S_IFDIR){ // 디렉토리
			
			printDIR(argv[argc-1], l,i,A,a,G,Q,m,F);
		}
		else{ // 다른파일들

			printFILE(buf,argv[argc-1], l,i,A,a,G,Q,m,F);
			printf("\n");
		}
	}



	return 0;
}
