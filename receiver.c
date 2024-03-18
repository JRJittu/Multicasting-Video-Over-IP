// Receiver with gtk
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>

#define MC_PORT 5433
#define BUF_SIZE 64000

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

int done = 0;
int r = 1;

/* Function for Pause */
int func1(GtkWidget *widget, gpointer data)
{
    g_print("video is pause...\n");
    done = 1;
    return 0;
}

/* Function for Resume */
int func2(GtkWidget *widget, gpointer data)
{
    g_print("video resumed...\n");

    done = 0;
    r = 0;
    usleep(5000000);
    return 0;
}

/* Function for Change Station */
void func3(GtkWidget *widget, gpointer data)
{
    g_print("Request to change the station\n");
    system("pkill ffplay");
    remove("live_data.mp4");
    exit(0);
}

/* Function for Terminate */
int func4(GtkWidget *widget, gpointer data)
{
    g_print("Terminated from current station...\nBYE BYE...\n");
    system("pkill ffplay");
    remove("live_data.mp4");
    exit(0);
    return 0;
}

void *threadFunction(void *args)
{

    printf("in thread\n");

    int s_udp;

    struct hostent *hp;
    struct sockaddr_in sin, cliaddr; /* socket struct */
    char *if_name;                   /* name of interface */
    struct ifreq ifr;                /* interface struct */

    char buf[BUF_SIZE];

    char *mcast_addr;               /* multicast address */
    struct ip_mreq mcast_req;       /* multicast join struct */
    struct sockaddr_in mcast_saddr; /* multicast sender*/
    socklen_t mcast_saddr_len;
    char add[32];

    mcast_addr = args;
    if_name = "eth0";

    if ((s_udp = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("receiver: socket");
        exit(1);
    }
    else
        printf("udp Socket created\n");

    int x = sizeof(sin);

    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(MC_PORT);

    /*Use the interface specified */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(if_name) - 1);

    if ((setsockopt(s_udp, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0)
    {
        perror("receiver: setsockopt() error");
        close(s_udp);
        exit(1);
    }
    else
        printf("setsockopt\n");

    /* bind the socket */
    if ((bind(s_udp, (struct sockaddr *)&sin, sizeof(sin))) < 0)
    {
        perror("receiver: bind()");
        close(s_udp);
        exit(1);
    }
    else
        printf("udp binded\n");

    /* build IGMP join message structure */
    mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
    mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

    /* send multicast join message */
    if ((setsockopt(s_udp, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mcast_req, sizeof(mcast_req))) < 0)
    {
        perror("mcast join receive: setsockopt()");
        exit(1);
    }

    printf("\nReady to listen!\n\n");
    int i = 0;
    int len;
    FILE *fp;
    fp = fopen("live_data.mp4", "wb");

    memset(&mcast_saddr, 0, sizeof(mcast_saddr));
    mcast_saddr_len = sizeof(mcast_saddr);

    while (1)
    {
        if (done == 0)
        {
            memset(&buf, 0, sizeof(buf));
            int err, l;

            len = recvfrom(s_udp, buf, sizeof(buf), 0, (struct sockaddr *)&mcast_saddr, &mcast_saddr_len);

            if (len < 0)
            {
                printf("Error in receiving\n");
            }
            else
            {
                printf("%d Receiving  %d\n", i, len);
                fwrite(buf, 1, len, fp);
                if (i == 8)
                {
                    system("gnome-terminal -- sh -c 'ffplay -i live_data.mp4;'");
                }
            }
            i++;
        }
    }

    fclose(fp);
    close(s_udp);
}

int main(int argc, char *argv[])
{
    char *mcast_addr;

    if (argc == 2)
        mcast_addr = argv[1];
    else
        printf("\nInvalid arguments");

    pthread_t id;
    pthread_create(&id, NULL, &threadFunction, mcast_addr);

    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *label;
    GdkColor color;

    gtk_window_set_title(GTK_WINDOW(window), "Control!");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gdk_color_parse("light yellow", &color);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();

    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &color);

    button = gtk_button_new_with_label("Pause");
    gtk_widget_set_size_request(button, 300, 80);
    g_signal_connect(button, "clicked", G_CALLBACK(func1), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 5, 5, 5, 5);

    button = gtk_button_new_with_label("Resume");
    gtk_widget_set_size_request(button, 300, 80);
    g_signal_connect(button, "clicked", G_CALLBACK(func2), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 5, 10, 5, 5);

    button = gtk_button_new_with_label("Change station");
    gtk_widget_set_size_request(button, 300, 80);
    g_signal_connect(button, "clicked", G_CALLBACK(func3), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 5, 15, 5, 5);

    button = gtk_button_new_with_label("Terminate");
    gtk_widget_set_size_request(button, 300, 80);
    g_signal_connect(button, "clicked", G_CALLBACK(func4), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 5, 20, 5, 5);

    gtk_widget_show_all(window);
    gtk_main();
}