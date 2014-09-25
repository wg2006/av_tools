#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define SERVER_PORT 4000
#define RX_DATA_LEN 1024
#define TX_DATA_LEN 512
#define MAXCLIENTS 8

static char *inet_ntoa4(unsigned int uIp)
{
	struct in_addr addr;
	addr.s_addr = uIp;
	return inet_ntoa(addr);
}

static int listen_socket_init(int type, int protocol,int port) {
    struct sockaddr_in serv_addr;
    int fd = -1;
    int opt = 1;

	printf("begin create socket %s:type:%d protocol:%d port:%d\n", __FUNCTION__, type, protocol, port);
	
    if((fd = socket(AF_INET, type, protocol)) < 0) {
        printf("error: create socket failed:%s\n", strerror(errno));
        return -1;
    }
	
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)<0) {
        printf("error: scntl set NONBLOCK failed\n");
        close(fd);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if(bind(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("error: bind: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if(type == SOCK_STREAM && listen(fd, MAXCLIENTS) < 0) {
        printf("error: listen: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

	printf("Create listen socket:%d, type:%d, protocal:%d successfully\n", fd, type, protocol);

    return fd;
}

static void rx_tx_loop(int sock){
	struct	timeval timeout;
	fd_set	rset;
	int rc = -1;
	char rx_buf[RX_DATA_LEN];
	char tx_buf[TX_DATA_LEN];
	int len = 0;
	struct sockaddr_in rxaddr;
	int addrlen = sizeof(rxaddr);
	
	while(1){
		timeout.tv_sec	= 1;
		timeout.tv_usec = 0;
		
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		
		rc = select(sock + 1, &rset, NULL, NULL, &timeout);
		if(rc <0){
			if (errno == EINTR){
					continue;
			}
			
			printf("select error:%s\n", strerror(errno));
			break;
		}
		
		if ( rc == 0 ) {
			//printf("select timeout, continue:%u\n", (unsigned int)time(NULL));
			continue;
		}
		
		//recv data
		if(FD_ISSET(sock, &rset)) {
			len = recvfrom(sock, 
				rx_buf, 
				RX_DATA_LEN, 
				0,
				(struct sockaddr *)&rxaddr, 
				&addrlen);
			
			printf("recv data len:%d from %s:%d\n", len, inet_ntoa4(rxaddr.sin_addr.s_addr), ntohs(rxaddr.sin_port));		
		}
		
		//send data, not check write is avaliable or not
		len = sendto(sock, 
				tx_buf, 
				TX_DATA_LEN,
				0,
				(struct sockaddr *)&rxaddr,
				addrlen
				);
					
		printf("send data len:%d:%d to %s:%d\n", 
				len, 
				TX_DATA_LEN, 
				inet_ntoa4(rxaddr.sin_addr.s_addr),
				ntohs(rxaddr.sin_port)
				);
	}	
}

int main(){
	int sock = -1;
	sock = listen_socket_init(SOCK_DGRAM, 0, SERVER_PORT);
	if(sock < 0){
		printf("init listen sock failed\n");
		return -1;
	}
	
	rx_tx_loop(sock);
	
	return 0;
}