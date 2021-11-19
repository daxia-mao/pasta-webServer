#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#define SERV_PORT 8080
#define MAXLEN 1024
#define MAXSIZE 100000

void sig_SIGCHLD(int signo){
	pid_t pid;
	int stat;
	while( (pid = waitpid(-1,&stat,WNOHANG)) > 0){
		printf("child %d closed\n",pid);
	}
	return;
}

int doit(int sockfd){

	struct {
		char *ext;
		char *filetype;
	} extensions [10] = {
		{"gif", "image/gif" },
		{"jpg", "image/jpeg"},
		{"jpeg","image/jpeg"},
		{"png", "image/png" },
		{"zip", "image/zip" },
		{"gz",  "image/gz"  },
		{"tar", "image/tar" },
		{"htm", "text/html" },
		{"html","text/html" },
		{"exe","text/plain" }};

    char buffer[MAXLEN] = {};
	ssize_t n;

	FILE *input;
	input = fdopen(sockfd,"r");

	//讀取http req的方法 e.g. GET,POST
	fgets(buffer,MAXLEN,input);
	char *method = strtok(buffer," ");
	printf("method:%s\n",method);

	if(!strcmp(method,"GET")){
		char *filename = strtok(NULL , " ");
		printf("filename:%s\n",filename);

		// 擋掉回上層路徑的檔案請求 e.g. ../index.html
		for(int i=0; i<strlen(filename); i++){
			if(filename[i] == '.' && filename[i+1] =='.'){
				sprintf(buffer,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<a>can't open ..file<a>");
				write(sockfd, buffer,strlen(buffer));
				exit(3);
			}
		}

		// 獲取並確認filetype，以及獲得要回傳給client的content-type
		char filetype[10] = "";
		for(int i=0;i<strlen(filename); i++){
			if(filename[i] == '.'){
				strcpy(filetype,&filename[i+1]);
			}
		}
		printf("filetype:%s\n",filetype);
		
		char *contentType = 0;
		for(int i=0;i<10;i++){
			if(!strcmp(filetype,extensions[i].ext)){
				contentType = extensions[i].filetype;
				break;
			}
		}
		//檢查檔案型別是否有支援，若無支援回傳error page
		if(contentType == 0){
			sprintf(buffer,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<a>this filetype is not supported.<a>");
			write(sockfd, buffer,strlen(buffer));
			exit(3);
		}

		//打開檔案，若無法打開回傳error page
		int filefd = 0;
		char *tmpfp = &filename[1]; 
		printf("tmpfp:%s\n",tmpfp);
		if((filefd = open(tmpfp,O_RDONLY)) < 0){
			printf("error open\n");
			sprintf(buffer,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<a>Can't open this file.<a>");
			write(sockfd, buffer,strlen(buffer));
			exit(3);
		}

		//正常回傳目的檔案
		sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", contentType);
		write(sockfd,buffer,strlen(buffer));
		while ((n=read(filefd, buffer, MAXLEN))>0) {
			write(sockfd,buffer,n);
		}
		exit(1);
	}
	else if(!strcmp(method,"POST")){
		int content_length = 0;
		char flag = 0;
		char catchBuffer[MAXSIZE] = {0};
		while(fgets(buffer,MAXLEN,input) != NULL){
			if(strncmp(buffer,"------WebKitFormBoundary",24)){
				flag = !flag;
			}
			if(flag == 1){
				printf("%s",buffer);
			}
		}

		

	}

	close(sockfd);
	fclose(input);
	exit(1);
	// 接收HTTP Request的資料
	// if((n = recv(sockfd, buffer, MAXLEN,0)) <= 0){
	// 	exit(3);
	// }

	// printf("res:\n%s\nsize:%ld\n",buffer,strlen(buffer));

	// char *token = strtok(token , " ");
	// printf("%s\n",token); 

	// 擷取HTTP Request的第一行 e.g. GET /index.html HTTP/1.1
	// for(int i=0; i<n; i++){
	// 	if(buffer[i] == '\r' || buffer[i] == '\n'){
	// 		buffer[i] = '\0';
	// 		break;
	// 	}
	// }

	// // 擷取http method、filename，到reqArray之中。 e.g. reqArray[0] -> GET , reqArray[1] -> index.html
	// char *reqArray[2] = {NULL};
	// int c = 0;
	// char *token = strtok(buffer," ");
	// while(token){
	// 	reqArray[c] = (char *)malloc(sizeof(strlen(token)));
	// 	strcpy(reqArray[c],token);
	// 	c++;
	// 	token = strtok(NULL," ");
	// }
	// printf("method:%s\nfilename:%s\n",reqArray[0],reqArray[1]);

	// // 擋掉回上層路徑的檔案請求 e.g. ../index.html

	// for(int i=0; i<strlen(reqArray[1]); i++){
	// 	if(reqArray[1][i] == '.' && reqArray[1][i+1] =='.'){
	// 		fprintf(stderr,"error:Can't oepn the../file\n");
	// 		exit(3);
	// 	}
	// }

	// if(!strcmp(reqArray[0],"GET")){
	// 		// 獲取並確認filetype，以及獲得要回傳給client的content-type
	// 	char filetype[10] = "";
	// 	for(int i=0;i<strlen(reqArray[1]); i++){
	// 		if(reqArray[1][i] == '.'){
	// 			strcpy(filetype,&reqArray[1][i+1]);
	// 		}
	// 	}
	// 	printf("filetype:%s\n",filetype);
		
	// 	char *contentType = 0;

	// 	for(int i=0;i<10;i++){
	// 		if(!strcmp(filetype,extensions[i].ext)){
	// 			contentType = extensions[i].filetype;
	// 			break;
	// 		}
	// 	}

	// 	//檢查檔案型別是否有支援，若無支援回傳error page
	// 	if(contentType == 0){
	// 		sprintf(buffer,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<a>this filetype is not supported.<a>");
	// 		write(sockfd, buffer,strlen(buffer));
	// 		exit(3);
	// 	}

	// 	//打開檔案，若無法打開回傳error page
	// 	int filefd = 0;
	// 	char *tmpfp = &reqArray[1][1]; 
	// 	printf("tmpfp:%s\n",tmpfp);
	// 	if((filefd = open(tmpfp,O_RDONLY)) < 0){
	// 		printf("error open\n");
	// 		sprintf(buffer,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<a>Can't open this file.<a>");
	// 		write(sockfd, buffer,strlen(buffer));
	// 		exit(3);
	// 	}

	// 	//正常回傳目的檔案
	// 	sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", contentType);
	// 	write(sockfd,buffer,strlen(buffer));
	// 	while ((n=read(filefd, buffer, MAXLEN))>0) {
	// 		write(sockfd,buffer,n);
	// 	}
	// }
	// else if(!strcmp(reqArray[0],"POST")){
	// 	char recvbuff[256];
	// 	char fbufer[256];
	// 	n=read(sockfd,recvbuff,256);
	// 	int i =0;
	// 	for(i=0;i<n;i++){
	// 		if(recvbuff[i]=='\r'&&recvbuff[i+1]=='\n'&&recvbuff[i+2]=='\r'&&recvbuff[i+3]=='\n'){
	// 			printf("I am find!:%d\n",i);
	// 			recvbuff[i]=recvbuff[i+1]=recvbuff[i+2]=recvbuff[i+3]='$';
	// 			break;
	// 		}
	// 	}
	// 	recvbuff[i+3] = '\0';
	// 	printf("\n----head----:\n%s",recvbuff);

	// 	char *token = strtok(recvbuff,"\n");

	// 	printf("\n----data----:\n%s\n",&recvbuff[i+4]);
	// 	char *data = &recvbuff[i+4];
		
		// FILE *fp;
		// fp = fopen("sample_file.txt", "wb+");
		// //寫入前面剩下的部份
		// fwrite(data,1,strlen(data),fp);

		// while((n = read(sockfd, recvbuff, 256)) > 0){
		// 	printf("Bytes received %ld\n",n);
		// 	fwrite(recvbuff, 1,n,fp);
		// }
		// if(n < 0)
		// {
		// 	printf("\n Read Error \n");
		// }

		// fseek()

	// }

	
	exit(1);
}


int main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	/*設定Server資訊*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("140.123.158.65");
	servaddr.sin_port        = htons(SERV_PORT);

	bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	listen(listenfd, 1024);

	/*註冊signal函數，SIGCHLD為子程序退出後傳給父程序的signal，不做處理會有zombie*/
	signal(SIGCHLD,sig_SIGCHLD);	

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
		printf("%d\n",connfd);
		if ( (childpid = fork()) == 0) {	//分出子程序處理需求
			close(listenfd);	/* 因為是子程序所以關閉listenfd */
			doit(connfd);	/* 執行處理函數 */
			exit(0);
		}
		close(connfd);			/* 父程序斷掉和子程序的連接 */
	}
}

