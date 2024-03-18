#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 512000

/* Site info */
struct site_info
{
	uint8_t site_name_size;
	char site_name[20];
	uint8_t site_desc_size;
	char site_desc[100];
	uint8_t station_count;
};

/* Station info */
struct station_info
{
	uint8_t station_number;
	char station_name[50];
	char multicast_address[32];
	uint16_t data_port;
	uint16_t info_port;
	uint32_t bit_rate;
};

int main()
{
	struct station_info stat1, stat2, stat3;
	struct site_info site1, site2, site3;
	struct sockaddr_in sin;

	char buffer[MAX_LINE];
	int len;
	int sock_tcp, new_sock_tcp;
	char str[INET_ADDRSTRLEN];
	int num;

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	if ((sock_tcp = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("simplex-talk: socket");
		exit(1);
	}

	inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
	printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

	if ((bind(sock_tcp, (struct sockaddr *)&sin, sizeof(sin))) < 0)
	{
		perror("simplex-talk: bind");
		exit(1);
	}
	else
		printf("Server bind done.\n");

	listen(sock_tcp, MAX_PENDING);

	while (1)
	{
		if ((new_sock_tcp = accept(sock_tcp, (struct sockaddr *)&sin, &len)) < 0)
			printf("Error in accepting\n");
		else
			printf("Accepted\n\n");

		recv(new_sock_tcp, buffer, sizeof(buffer), 0); // "Start" signal to make sure tcp is connected
		printf("Data from client : %s\n", buffer);

		/* Site info for station 1 */
		bzero(&site1, sizeof(site1));
		strcpy(site1.site_name, "www.sports.com");
		strcpy(site1.site_desc, "Cricket, Football, Tennis etc ");

		/* station 1 */
		bzero(&stat1, sizeof(stat1));
		stat1.station_number = 1;
		strcpy(stat1.station_name, "SPORTS");
		strcpy(stat1.multicast_address, "239.192.4.1");
		stat1.data_port = 5433;
		stat1.info_port = 5432;
		stat1.bit_rate = 1087;

		/* Site info for station 2 */
		bzero(&site2, sizeof(site2));
		strcpy(site2.site_name, "www.nature.com");
		strcpy(site2.site_desc, "Forest, Animals, Hills etc");

		/* station 2 */
		bzero(&stat2, sizeof(stat2));
		stat2.station_number = 2;
		strcpy(stat2.station_name, "NATURE");
		strcpy(stat2.multicast_address, "239.192.4.2");
		stat2.data_port = 5433;
		stat2.info_port = 5432;
		stat2.bit_rate = 891;

		send(new_sock_tcp, &(site1), sizeof(site1) + 1, 0);
		send(new_sock_tcp, &(site2), sizeof(site2) + 1, 0);

		send(new_sock_tcp, &(stat1), sizeof(stat1) + 1, 0);
		send(new_sock_tcp, &(stat2), sizeof(stat2) + 1, 0);

		close(new_sock_tcp);
	}
	return 0;
}