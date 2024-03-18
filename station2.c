// sender_udp2 with gtk

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MC_PORT 5433
#define BUF_SIZE 64000

int main(int argc, char *argv[])
{
	int s_udp;
	struct sockaddr_in sin;
	char buf[BUF_SIZE];
	int len;
	socklen_t sin_len;
	sin_len = sizeof(sin);

	char *mcast_addr;

	char *video[5];
	video[0] = "vid6.mp4";
	video[1] = "vid7.mp4";
	video[2] = "vid8.mp4";
	video[3] = "vid9.mp4";
	video[4] = "vid10.mp4";

	if (argc == 2)
		mcast_addr = argv[1];
	else
	{
		fprintf(stderr, "usage: sender multicast_address\n");
		exit(1);
	}

	if ((s_udp = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("server UDP: socket");
		exit(1);
	}

	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(mcast_addr);
	sin.sin_port = htons(MC_PORT);

	// if (bind(s_udp, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	// {
	// 	perror("server UDP: bind");
	// 	exit(1);
	// }

	printf("Connected in second station\n\n ");
	memset(buf, 0, sizeof(buf));
	FILE *fp = NULL;
	while (1)
	{
		for (int i = 0; i < 5; i++)
		{
			fp = fopen(video[i], "rb");

			if (fp == NULL)
				printf("\nFile not found\n");
			else
			{
				int tot_frame;
				fseek(fp, 0, SEEK_END);
				long fsize = ftell(fp);
				if ((fsize % BUF_SIZE) != 0)
				{
					tot_frame = (fsize / BUF_SIZE) + 1;
				}
				else
					tot_frame = (fsize / BUF_SIZE);

				printf("\n\nVideo Number            : %d", i);
				printf("\nFile size is            : %ld", fsize);
				printf("\nTotal number of packets : %d\n", tot_frame);

				fseek(fp, 0, SEEK_SET);

				if (tot_frame == 0 || tot_frame == 1)
				{
					char *string = malloc(fsize + 1);
					fread(string, 1, fsize + 1, fp);
					fseek(fp, 0, SEEK_SET);
					sendto(s_udp, string, fsize + 1, 0, (struct sockaddr *)&sin, sin_len);
				}
				else
				{

					for (int i = 1; i <= tot_frame; i++)
					{
						char *string = malloc(BUF_SIZE);
						len = fread(string, 1, BUF_SIZE, fp);
						fseek(fp, 0, SEEK_CUR);
						sendto(s_udp, string, len, 0, (struct sockaddr *)&sin, sin_len);
						usleep(300000);
						printf("sent frame %d\n", i);
					}
				}
				fclose(fp);
			}
		}
	}
	return 0;
}