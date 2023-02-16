#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

int id[24 * 1024];
int max = 0, next_id = 0;
fd_set active, readyRead, readyWrite;
char bufRead[200000], str[200000], bufWrite[200040];

void fatal_error()
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

void send_all(int es) {
    for (int i = 0; i <= max; i++)
        if (FD_ISSET(i, &readyWrite) && i != es)
            send(i, bufWrite, strlen(bufWrite), 0);
    bzero(&bufWrite, sizeof(bufWrite));
}

int main(int ac, char **av) {
    if (ac != 2) {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }
    bzero(&id, sizeof(id));
    FD_ZERO(&active);
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0)
        fatal_error();
    max = serverSock;
    FD_SET(serverSock, &active);
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

    if ((bind(serverSock, (const struct sockaddr *)&addr, sizeof(addr))) < 0)
        fatal_error();
    if (listen(serverSock, 128) < 0)
        fatal_error();

    while (1) {
        readyRead = readyWrite = active;
        if (select(max + 1, &readyRead, &readyWrite, NULL, NULL) < 0)
            continue ;

        for (int s = 0; s <= max; s++) {

            if (FD_ISSET(s, &readyRead) && s == serverSock) {
                int clientSock = accept(serverSock, (struct sockaddr *)&addr, &addr_len);
                if (clientSock < 0)
                continue ;

                max = (clientSock > max) ? clientSock : max;
                id[clientSock] = next_id++;
                FD_SET(clientSock, &active);

                sprintf(bufWrite, "server: client %d just arrived\n", id[clientSock]);
                send_all(clientSock);
                break ;
            }
            if (FD_ISSET(s, &readyRead) && s != serverSock) {
				int ret_recv = 1;
				while (ret_recv == 1 && bufRead[strlen(bufRead) - 1] != '\n'){
					ret_recv = recv(s, bufRead + strlen(bufRead), 1, 0);
					if (ret_recv <= 0)
						break ;
					}
                if (ret_recv <= 0) {
                    sprintf(bufWrite, "server: client %d just left\n", id[s]);
                    send_all(s);
                    FD_CLR(s, &active);
                    close(s);
                    break ;
                }
                else {
                    for (int i = 0, j = 0; i < strlen(str); i++, j++) {
                        str[j] = bufRead[i];
                        if (str[j] == '\n') {
                            str[j] = '\0';
                            sprintf(bufWrite, "client %d: %s\n", id[s], str);
                            send_all(s);
                            bzero(&str, sizeof(str));
                            j = -1;
                        }
                    }
                }
                bzero(&bufRead, sizeof(bufRead));
            }
        }
    }
}