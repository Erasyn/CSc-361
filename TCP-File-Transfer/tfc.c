#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// gcc -o tfc tfc.c
// ./tfc 10.10.1.100 8080 sample_text.zip
int main(int argc, char *argv[])
{
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  
	// ip 10.10.1.100 port 8080
  if(argc != 4) {
    printf("An ip and port are required parameters.\n./tfc <ip> <port> <zip_file>\n");
		return -1;
  }
  int sockfd,n;
  struct sockaddr_in sa;
 
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  memset(&sa, 0, sizeof sa);
 
	sa.sin_family=AF_INET;
	sa.sin_port=htons(atoi(argv[2]));
 
  inet_pton(AF_INET,argv[1],&(sa.sin_addr));

  connect(sockfd,(struct sockaddr *)&sa,sizeof(sa));
 
  while(1)
  {
		printf("%s\n", argv[3]);
		FILE *fp = fopen(argv[3],"r");
		if(fp==NULL){
			printf("File open error\n");
			return 1;
		}

		// Determine the requested file size
		fseek(fp, 0L, SEEK_END);
		long int file_size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
	
		// Reads the file into the buffer
		char text[file_size+1];
		memset(text, 0, sizeof text);
		int bytes_read = fread(text, sizeof(char), file_size, fp);
		fclose(fp);

		//printf("File size: %ld bytes\nBytes read: %d bytes\n",file_size, (int)bytes_read);

    printf("sending %ld bytes\n",file_size);
    if (sendall(sockfd, text, &file_size) == -1) {
        perror("sendall");
    }
		shutdown(sockfd, 1);
    system("md5sum sample_file.zip");
    fp = fopen("output.txt", "w+");
    while(1)
    {
      char buffer[1024];
      int bytes_recv = 1;
      int total = 0;

      while(bytes_recv != 0) {
        memset( &buffer, 0, sizeof(buffer));
        bytes_recv = recv(sockfd, buffer, sizeof(buffer-1), 0);
        if (bytes_recv < 0) {
          fprintf(stderr, "err: something wrong\n");
          return;
        }
        fwrite(buffer, sizeof(char), bytes_recv, fp);
        total += bytes_recv;
      }
      fclose(fp);
      system("md5sum output.txt");
      break;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t ts = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("total transfer time was %lums\n", ts/1000 );
    return;
  }
 
}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here
    printf("%d bytes sent\n", *len);

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
