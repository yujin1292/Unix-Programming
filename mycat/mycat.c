#include <stdlib.h>
#include <stdio.h>
#include <string.h>
void print(FILE* fp){ //stdout으로 출력하는 함수
	char buf[BUFSIZ];
	while(fgets(buf, BUFSIZ , fp) != NULL )
		printf("%s",buf);

	rewind(fp);
}

FILE* squeez(FILE* fp){
	FILE * tmp_fp;
	char* empty = "\n";
	char buf[BUFSIZ];
	int squeez_ = 0; // squeez 여부

	if((tmp_fp = tmpfile() ) == NULL ) {
		perror("tempfile()");
		exit(1);
	}

	while(fgets(buf, BUFSIZ , fp) != NULL ){
		if(squeez_){  

			if( strcmp(buf, empty)==0 ){ // 빈줄
				continue;		
			}
			else{
				squeez_ = 0; 
				fputs(buf,tmp_fp);
			}

		}
		else{
			if( strcmp(buf, empty)==0 ){ // 빈줄 squeez ready시킴
 				squeez_ = 1;		
			}

			
				fputs(buf,tmp_fp);
		}

	}
	fclose(fp);
	rewind(tmp_fp);
	return tmp_fp;

}


FILE* changeTab (FILE* fp){
	FILE * tmp_fp;
	char buf[BUFSIZ];
	

	if((tmp_fp = tmpfile() ) == NULL ) {
		perror("tempfile()");
		exit(1);
	}

	while(fgets(buf, BUFSIZ , fp) != NULL ){
		
		for(int i = 0 ; i < strlen(buf) ; i++){
			
			if(buf[i] == '\t'){
				//한글자인 '\t' 가 "^I"로 한글자 늘어나므로 한칸씩 뒤로 밈
				for(int j = strlen(buf) ; j > i ; j--){
					buf[j+1] = buf[j];
				}
				buf[i] = '^';
				buf[i+1] = 'I';
				i++;
				buf[strlen(buf)+2] = '\0';
			}
		}
				
		
		fputs(buf,tmp_fp);
	}
	fclose(fp);
	rewind(tmp_fp);
	return tmp_fp;

}
int numlen(int a){
	int result = 0;
	int ten = 1;
	if( a== 0)
		return 1;
	
	while( (a/ten) != 0) {

		ten *= 10;
		result ++;
	}
	
	return result;
}

FILE* lineNum (FILE* fp){
	// 출력 형식 : "\t"+ num + "\t" + str 
	
	FILE * tmp_fp;
	char buf[BUFSIZ];
	int num = 1;
	char* empty = "\n";
	if((tmp_fp = tmpfile() ) == NULL ) {
		perror("tempfile()");
		exit(1);
	}
	
	while(fgets(buf, BUFSIZ , fp) != NULL ){
		
		int length = numlen(num); //Line number 의 자릿수 
		int temp = num;

		if( strcmp(buf,empty) == 0) {
			buf[0] = '\t';
			//10진수 숫자를 문자열로 바꿈 
			for(int i = length ; i >=1 ; i--){
				//숫자적어
				buf[i] = '0'+(temp % 10);
				temp/=10;
			}
			//문자열 끝에 개행문자와 '\0' 세팅
			buf[length+1] = '\n';
			buf[length+2] = '\0';


		}
		else{
			// line number 크기 만큼 뒤로 밀어냄
			for(int i = strlen(buf)  ; i >= 0 ; i--){
				buf[i+2+length] = buf[i];
			}
			
			//문자열 끝에 개행문자와 '\0' 세팅
			buf[strlen(buf)+3+length] = '\0';
			buf[0] = '\t';


			//10진수 숫자를 문자열로 바꿈 
			for(int i = length ; i >=1 ; i--){
				//숫자적어
				buf[i] = '0'+(temp % 10);
				temp/=10;
			}
			buf[length+1] = '\t';
		


		}
		
			fputs(buf,tmp_fp);
			num++;

	}

	fclose(fp);
	rewind(tmp_fp);

	return tmp_fp;

}

FILE* lineDollar (FILE* fp){ // 끝에 $기호를 붙이는 함수 
	char buf[BUFSIZ];
	FILE * tmp_fp;
		
	if((tmp_fp = tmpfile() ) == NULL ) {
		perror("tempfile()");
		exit(1);
	}
	
	while(fgets(buf, BUFSIZ , fp) != NULL ){
		int len = strlen(buf);
		buf[len-1] = '$';
		buf[len] = '\n';
		buf[len+1] = '\0';
		fputs(buf,tmp_fp);

	}

	fclose(fp);
	rewind(tmp_fp);

	return tmp_fp;
}


int main(int argc, char * argv[]){
	
	FILE *rfp, *tmp_fp ;
	char buf[BUFSIZ];
	extern int optopt;
	int n;
	int flag[] = { 0, 0, 0, 0} ;// squeez, tab, dollar, num 
	
	if( argc < 2 ){
		printf("no File\n");
		exit(1);
	}
	// 읽어들일 파일 포인터 open 
	if((rfp = fopen(argv[argc-1], "r")) == NULL ){
		perror("open err");
		exit(1);
	}

	//임시파일 파일 포인터 open 
	if((tmp_fp = tmpfile() ) == NULL ) {
		perror("tempfile()");
		exit(1);
	}
	//임시파일에 읽을 파일 파일 복사 
	while(fgets(buf, BUFSIZ , rfp) != NULL )
		fputs(buf,tmp_fp);


	fclose(rfp);
	rewind(tmp_fp);

	while((n=getopt(argc, argv, "stTeEnbA")) != -1){
		if(optopt != 0 )
			return 0;
		switch (n) {
			case 's' : // squeez
				flag[0] = 1;
				break;
			case 't' :
			case 'T' : // tab문자 ^I 로 바꾸
				flag[1] = 1;
				break;
			case 'e' : // 맨마지막 끝에 $
			case 'E' : 
				flag[2] = 1;
				break;

			case 'n' : //line number 표시
			case 'b' : 
				flag[3] = 1;
				break;	
			case 'A' : 
				flag[1]=flag[2] = 1;
				break;
			
		}

	}
	if( flag[0])
		tmp_fp = squeez(tmp_fp);
	if( flag[1])
		tmp_fp = changeTab(tmp_fp);
	if(flag[2])
		tmp_fp = lineDollar(tmp_fp);
	if( flag[3] )
		tmp_fp = lineNum(tmp_fp);
	print(tmp_fp);

	fclose(tmp_fp);
	return 0;
}

