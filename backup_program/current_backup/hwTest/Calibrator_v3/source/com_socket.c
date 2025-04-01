#include	<errno.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<string.h>
#include	<stdio.h>
#include	<sys/time.h>
#include    <sys/param.h>
#include    <netinet/in.h>
#include    <unistd.h>
#include	<time.h>
#include	<arpa/inet.h>
#include	"com_socket.h"
#include	"comm.h"

struct server_status sv_info;
int FD[FD_MAX], SD, stdinoutOK, readMask[20];
//int flag_bindloop =0;
int flag_bindloop = 300; //300 * 1sec = 5min
fd_set readset;

///////////// Server Part ///////////////

void write_ACK(int fd)
{
	int	ok;
	ok = ACCEPT;
	(void)write(fd, (char *)&ok, sizeof(ok));
}

void write_NAK(int fd)
{
	int	ok;
	ok = DENY;
	(void)write(fd, (char *)&ok, sizeof(ok));
}

int BuildMask(void)
{
    int i, max, word;
    word = (FD_MAX - 1) / 32 + 1;
    memset(readMask, 0x00, word * 4);
    max = 0;
    // Array index is the same with socket descriptor
    for(i=0; i < FD_MAX; i++) {
        if(FD[i] != -1) {
            readMask[FD[i] / 32] |= 1 << (i % 32);
            if(i > max) max = FD[i];
        }
    }
    return max;
}

int GetStdInOutOK(void)
{
	return stdinoutOK;
}

void SetStdInOutOK(int ok)
{
	stdinoutOK = ok;
}

void InitFD(void)
{
	int	i;
	for(i=0; i < FD_MAX; i++) FD[i] = -1;
	if(GetStdInOutOK()) {
		FD[0] = 0;
		FD[1] = 1;
	}
	SD = -1;
	memset(&sv_info, 0x00, sizeof(sv_info));
	FD_ZERO(&readset);
}

int SetFD(void)
{
	int	max_fd, i;
	for(i=0, max_fd=0; i < FD_MAX; i++) {
		if(FD[i] > -1) {
			FD_SET(FD[i], &readset);
			max_fd = max_fd < FD[i] ? FD[i] : max_fd;
		}
	}
#ifdef	DEBUGGING
	printf("{SE}max_fd : %d\n",max_fd);
#endif
	return max_fd;
}

char *GetHostName(void)
{
	struct	hostent *he;
	he = (struct hostent *)gethostent();
	return (char *)he->h_name;
}

struct server_status *SetServerInfo(int port)
{
	//sv_info.host = GetHostName();
	sv_info.host = "Elicopower";
	sv_info.user = FD_MAX;
	sv_info.st_time = time(0);
	sv_info.port = port;
	return &sv_info;
}

int SetServerBindLoop(int flag)
{
	return (flag_bindloop = flag);
}

int GetServerSock(int port)
{
	int ret, serv_sock,serv_len, rebind_sec=0, rebind_cnt=0, set;
	struct sockaddr_in serv_addr;
	struct timeval tv;
	fd_set rfds;

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_sock < 0) {
		printf("Socket ERROR : %s\n", strerror(errno));
		return -1;
	}
	memset((char*)&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	serv_len = sizeof(serv_addr);
	printf("Address : %d, Port : %d\n", serv_addr.sin_addr.s_addr,
		serv_addr.sin_port);
	
	set = 1;
	ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set));

	if(flag_bindloop == 0){
		ret = bind(serv_sock, (struct sockaddr *)&serv_addr, serv_len);
		if(ret > -1) {
			printf("(%d) seconds elapsed.\n", rebind_sec);
		}
		if(ret < 0) {
			printf("bind ERROR : %s\n", strerror(errno));
			return	-1;
		}
	} else {
		rebind_cnt = 0;
		while(1) {
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(0, &rfds);
			
			ret = select(1, &rfds, NULL, NULL, &tv);
			if(ret == 0) {
				ret = bind(serv_sock, (struct sockaddr *)&serv_addr,
					serv_len);
				if(ret > -1) {
					printf("(%d) seconds elapsed[%d].\n", rebind_cnt,
						rebind_sec);
					break;
				}
				if(ret < 0) {
					printf("bind ERROR[%d/%d] : %s\n", rebind_cnt+1,
						flag_bindloop, strerror(errno));
					rebind_sec += 1;
					if(rebind_cnt >= flag_bindloop) return -1;
				}
				rebind_cnt++;
			} else {
				ret = getc(stdin);
				if(ret == 'q' || ret == 'Q') return -2;
			}
		}
	}

	// The backlog(5) argument specifies
	// how many connection requests
	// can be queued by th system
	// while it waits for the server to execute
	// the accept() system call

	ret = listen(serv_sock, 1);
	if(ret < 0) {
		printf("listen ERROR : %s\n", strerror(errno));
		return -1;
	}
	printf("Open Server Socket : %d\n", serv_sock);
	return serv_sock;
}

int DoAccept(void)
{
	int i, client_fd;
	struct sockaddr_in client_addr;

	if(SD < -1) {
		printf("Code > SD = GetServerSock(port);\n");
		return -3;
	}
	i = sizeof(struct sockaddr_in);
	client_fd = accept(SD, (struct sockaddr *)&client_addr, &i);
	if(client_fd < 0) {
		printf("Accept ERROR : %s\n", strerror(errno));
		//write_NAK(client_fd);
		return -1;
	} else if(client_fd > FD_MAX-1) {
		printf("Accept ERROR : FD_MAX(%d), Current FD(%d)\n",
			FD_MAX, client_fd);
		close(client_fd);
		//write_NAK(client_fd);
		return -2;
	} else {
		sv_info.user++;
		printf("Accept FD(%d)  USER COUNT [%d]\n", client_fd, sv_info.user);
		FD[client_fd] = client_fd;
		printf("FD : ");
		for(i=0; i < FD_MAX; i++) {
			if(FD[i] != -1) printf("(%d)", FD[i]);
		}
		printf("->[%d]\n", i);
		//write_ACK(client_fd);
		return client_fd;
	}
}

int ProcessFD(int fd, int (*fp)(int))
{
	int ret;
	if(fd == SD) ret = DoAccept();
	else ret = fp(fd);
	return ret;
}

int ProcessMask(int *nfds, fd_set *rdset, int (*fp)(int))
{
	int	i, ret;
	if(*nfds <= 0) return -1;
	for(i=0; i < FD_MAX; i++) {
		if(FD[i] > -1 && FD_ISSET(FD[i], rdset)) {
			sv_info.cur_fd = FD[i];
			ret = ProcessFD(FD[i], fp);
			if(ret == 99) return ret;
			(*nfds)--;
			if(*nfds <= 0) return 0;
			return ret;
		}
	}
	return 1;
}

int CloseSD(void)
{
	int i;
	for(i=0; i < FD_MAX; i++) {
		if(FD[i] > -1 && FD_ISSET(FD[i], &readset)) CloseFD(FD[i]);
	}
	if(SD > -1)	close(SD);
	InitFD();
	return	0;
}

int CloseFD(int fd)
{
	if(fd < 0) return -1;
	FD_CLR(fd, &readset);
	FD[fd] = -1;
	sv_info.user--;
#ifdef	DEBUGGING
	printf("{SE}¢¸ Close  FD(%d)  USER COUNT [%d]\n", fd, sv_info.user);
#else
	//printf("{SEC:%d:%d}\n", fd, sv_info.user);
#endif
	return	close(fd);
}

int ServerEngine(int port, int (*fp)(int))
{
	int max_fd, ret;
	struct timeval tv;
	
	//SetServerInfo(port); //wkjg
	InitFD();
	SD = GetServerSock(port);
	if(SD < 0) return -1;
	FD[SD] = SD;

	while(1) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&readset);
		FD_SET(0, &readset);
		
		max_fd = SetFD();
		//ret = select(max_fd+1, &readset, NULL, NULL, (struct timeval *)0);
		ret = select(max_fd+1, &readset, NULL, NULL, &tv);
		if(ret < 1 && errno > 0)
			printf("{SE}select return : %d(%s)\n", ret, strerror(errno));
		if(ret == 99) {
			printf("server exit[%d]\n", getpid());
			break;
		}

		if(ret != 0) {
			if(max_fd > 0 && FD_ISSET(max_fd, &readset) == 1) {
				ret = ProcessMask(&ret, &readset, fp);
			} else {
				ret = getc(stdin);
				if(ret == 'q' || ret == 'Q') {
					CloseSD();
					return -1;
				}
			}
		}/* else {
				ret = getc(stdin);
				if(ret == 'q' || ret == 'Q') return -1;
		}*/
	}
	return 0;
}


//////////// Client Part /////////////

int SetClientSock(int port, char *addr)
{
	int clSD, clFD;
	struct sockaddr_in serv_addr;

	printf("Connecting...(%s:%d)\n", addr, port);
	memset((char *)&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(addr);
	serv_addr.sin_port = htons(port);

#ifdef	DEBUGGING
	printf("{CL}Address : %lu     Port : %d\n",
		serv_addr.sin_addr.s_addr, serv_addr.sin_port);
#endif
	clSD = socket(AF_INET, SOCK_STREAM, 0);
	if(clSD < 0) {
		printf("{CL}Open Socket ERROR [%d:%s] : %s\n",
			port, addr, strerror(errno));
		return -errno;
    }
    clFD = connect(clSD, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(clFD < 0) {
		printf("{CL}Connect ERROR [%d:%s] : %s\n",
			port, addr, strerror(errno));
		close(clSD);
		return -errno;
	} else {
		printf("{CL}Connect to Server[%d] --->> [fd:%d]\n", clSD, clFD);
		return clSD;
	}
}

int	UseSocket(int port_num, char *ip_addr)
{
	int	socket_d;
	if((socket_d = SetClientSock(port_num, ip_addr)) < 0) {
		printf("Client Socket error Port = [%d]", port_num);
	}
	return socket_d;
}
