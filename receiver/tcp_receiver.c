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
#define MAXCLIENTS 4

typedef struct client{
	int  fd;
	struct sockaddr_in rxaddr;
	int addr_len;
}client;

client client_list[MAXCLIENTS];

static int set_non_block(int sck)
{
    int fl = fcntl(sck, F_GETFL);
    if (fl < 0)
        return -1;
	
    if (fcntl(sck, F_SETFL, fl | O_NONBLOCK) < 0)
        return -1;

    return 0;
}

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

static void rx_tx_data(fd_set* rset){
	char rx_buf[RX_DATA_LEN];
	char tx_buf[TX_DATA_LEN];
	int i = 0;
	int sock = -1;
	int len = 0;
	
	for(i = 0; i < MAXCLIENTS; i++){
		sock = client_list[i].fd;
		if(sock < 0)
			continue;
			
		if(FD_ISSET(sock, rset)) {
			len = recvfrom(sock, 
				rx_buf, 
				RX_DATA_LEN, 
				0,
				NULL, 
				NULL);
			
			if(len > 0){
				printf("%u recv data len:%d from %s:%d\n", 
					time(NULL),
					len, 
					inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
					ntohs(client_list[i].rxaddr.sin_port)
					);
			}else if(len == 0){
				printf("TCP session:%d Error-->rx:peer shut down....socket:%d(%s:%d), close it\n",
					i, 
					sock,
					inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
					ntohs(client_list[i].rxaddr.sin_port)
					);
				
					close(client_list[i].fd);
					client_list[i].fd = -1;
					continue;
					
				}else if (len <0){
					if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS || errno == EALREADY) {
						printf("TCP session:%d rx:read error,socket:%d(%s:%d):%s, try again\n", 
							i, 
							sock,
							inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
							ntohs(client_list[i].rxaddr.sin_port),
							strerror(errno)
							);
					}else{
						printf("TCP session:%d rx:read error,socket:%d(%s:%d):%s, close it\n",
							i, 
							sock,
							inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
							ntohs(client_list[i].rxaddr.sin_port),
							strerror(errno)
							);
						
						close(client_list[i].fd);
						client_list[i].fd = -1;
						continue;
					}
				}


				//send data, not check write is avaliable or not
				len = sendto(sock, 
						tx_buf, 
						TX_DATA_LEN,
						0,
						NULL,
						0
						);
				if(len >=0){			
				printf("send data len: %u %d:%d to %s:%d\n",
		                time(NULL), 
						len, 
						TX_DATA_LEN, 
						inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
						ntohs(client_list[i].rxaddr.sin_port)
						);
			
				}else if (len <0){
					if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS || errno == EALREADY) {
						printf("TCP session:%d tx:send error,socket:%d(%s:%d):%s, try again\n", 
							i, 
							sock,
							inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
							ntohs(client_list[i].rxaddr.sin_port),
							strerror(errno)
							);
					}else{
						printf("TCP session:%d tx:send error,socket:%d(%s:%d):%s, close it\n",
							i, 
							sock,
							inet_ntoa4(client_list[i].rxaddr.sin_addr.s_addr),
							ntohs(client_list[i].rxaddr.sin_port),
							strerror(errno)
							);
						
						close(client_list[i].fd);
						client_list[i].fd = -1;
						continue;
					}
				}
				
		}
		
		
	}		
}

static void rx_tx_loop(){
	struct	timeval timeout;
	fd_set	rset;
	int rc = -1;
	char rx_buf[RX_DATA_LEN];
	char tx_buf[TX_DATA_LEN];
	int len = 0;
	struct sockaddr_in rxaddr;
	int addrlen = sizeof(rxaddr);
	int max_fd = 0;
	int i = 0;
	
	while(1){
		timeout.tv_sec	= 1;
		timeout.tv_usec = 0;
		
		FD_ZERO(&rset);
		for(i = 0; i < MAXCLIENTS; i++){
			if(client_list[i].fd >=0){
				FD_SET(client_list[i].fd, &rset);
				if(client_list[i].fd > max_fd)
					max_fd = client_list[i].fd;
			}
		}
		
		rc = select(max_fd + 1, &rset, NULL, NULL, &timeout);
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
		
		rx_tx_data(&rset);
	}
}



void connection_monitor(int sock){
	struct	timeval timeout;
	fd_set	rset;
	int rc = -1;

	int len = 0;
	int fd;
	int client_index = -1;
	int i;
	
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
		
		client_index = find_free_client();
		if(client_index < 0){
			printf("---->no free client space\n");
			continue;
		}
		
		fd = accept(sock, 
				(struct sockaddr *)&client_list[client_index].rxaddr, 
				&client_list[client_index].addr_len
				);
			
		set_non_block(fd);
		client_list[client_index].fd = fd;
		
	}
}

int find_free_client(){
	client* cli = NULL;
	
	int i;
	for(i = 0; i < MAXCLIENTS; i++){
		if(client_list[i].fd == -1){
			return i;
		}
	}
	
	return -1;
}

void init_client_list(){
	int i;
	
	for(i = 0; i < MAXCLIENTS; i++){
		client_list[i].fd = -1;
		memset(&client_list[i].rxaddr, 0, sizeof(client_list[i].rxaddr));
		client_list[i].addr_len = sizeof(client_list[i].rxaddr);
	}
}

void* rx_tx_worker(void* ptr){
	rx_tx_loop();

	return NULL;
}

int main(){
	int sock = -1;
	int ret = 0;
	pthread_t tid = 0;
	
	init_client_list();
	sock = listen_socket_init(SOCK_STREAM, 0, SERVER_PORT);
	if(sock < 0){
		printf("init listen sock failed\n");
		return -1;
	}
	
	ret = pthread_create(&tid, NULL, rx_tx_worker, NULL);
	if(ret != 0 ) {
		printf("create pthread rx_tx_worker failed\n");
		return -1;
	}else{
		printf("create pthread rx_tx_worker success\n");
	}
	
	connection_monitor(sock);
	
	return 0;
}
