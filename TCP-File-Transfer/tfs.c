/*Required Headers*/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>


// gcc -o tfs tfs.c
// ./tfs 10.10.1.100 8080
int main(int argc, char *argv[])
{

  if(argc != 3) {
    printf("An ip and port are required parameters.\n./tfs <ip> <port>\n");
		return -1;
  }
 
  int sock_fd, comm_fd;
  FILE *fp = fopen("temp.zip", "w+");
 
  struct sockaddr_in sa;

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 
  memset( &sa, 0, sizeof(sa));
 
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  sa.sin_port = htons(atoi(argv[2]));
 
  bind(sock_fd, (struct sockaddr *) &sa, sizeof(sa));
 
  listen(sock_fd, 10);
 
  comm_fd = accept(sock_fd, (struct sockaddr*) NULL, NULL);

  while(1)
  {
    char buffer[1024];
    int bytes_recv = 1;
    int total = 0;

    while(bytes_recv != 0) {
      memset( &buffer, 0, sizeof(buffer));
      bytes_recv = recv(comm_fd, buffer, sizeof(buffer-1), 0);
      if (bytes_recv < 0) {
        fprintf(stderr, "err: something wrong\n");
        return;
      }
      fwrite(buffer, sizeof(char), bytes_recv, fp);
      total += bytes_recv;
    }
    fclose(fp);
    printf("%d bytes read\n", total);
    system("md5sum temp.zip");
    system("unzip -p -o temp > temp_text.txt");
    break;
  }
  fp = fopen("temp_text.txt", "r");
  if(fp==NULL){
	  printf("File open error\n");
		return 1;
	}
	long int file_size;


  while(1)
  {
    fseek(fp, 0L, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		char text[file_size+1];
		memset(text, 0, sizeof text);
		int bytes_read= fread(text, sizeof(char), file_size, fp);
		fclose(fp);

		//printf("File size: %ld bytes\nBytes read: %d bytes\n",file_size, (int)bytes_read);

    printf("sending %ld bytes\n",file_size);
    if (sendall(comm_fd, text, &file_size) == -1) {
        perror("sendall");
    }

    system("md5sum temp_text.txt");
    break;

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
