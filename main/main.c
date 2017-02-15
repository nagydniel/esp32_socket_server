#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <esp_log.h>
#include <string.h>
#include <errno.h>
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "stdio.h" //printf
#include "stdlib.h" //exit(0);
#include "arpa/inet.h"
#include "nvs_flash.h"
#include "driver/gpio.h"


#define PORT_NUMBER 2016

#define BLINK_GPIO GPIO_NUM_4

static char tag[] = "socket_server";

/**
 * Create a listening socket.  We then wait for a client to connect.
 * Once a client has connected, we then read until there is no more data
 * and log the data read.  We then close the client socket and start
 * waiting for a new connection.
 */
void socket_server(void *ignore) {
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;

    // Create a socket that we will listen upon.
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
        goto END;
    }

    // Bind our server socket to a port.
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT_NUMBER);
    int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (rc < 0) {
        ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
        goto END;
    }

    // Flag the socket as listening for new connections.
    rc = listen(sock, 5); //5 concurrent connections
    if (rc < 0) {
        ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
        goto END;
    }

    while (1) {
        // Listen for a new client connection.
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSock < 0) {
            ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
            printf("accept: %d %s", clientSock, strerror(errno));
            goto END;
        }

        // We now have a new client ...
        int total = 4; //bytes 10*1024 // we send 4 chars for now
        int sizeUsed = 0;
        char *data = malloc(total);
        char buffer[256];
        int n;

        bzero(buffer,256);
        n = read(clientSock,buffer,255);

        if (n < 0) printf("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        // Loop reading data.
     /*   while(1) {
            ssize_t sizeRead = recv(clientSock, data + sizeUsed, total-sizeUsed, 0);
            if (sizeRead < 0) {
                ESP_LOGE(tag, "recv: %d %s", sizeRead, strerror(errno));
                printf("recv: %d %s", sizeRead, strerror(errno));
                goto END;
            }
            if (sizeRead == 0) { // need to send a ctrl+d in terminal (eof)
                //debug with: nc IPADDR PORT
                printf("sizeread=0\n");
                break;
            } else if (sizeUsed == 4) {
                printf("sizeused=4\n");
                break;
            }
            sizeUsed += sizeRead;
        }
        */

        // Finished reading data.
      //  ESP_LOGD(tag, "Data read (size: %d) was: %.*s", sizeUsed, sizeUsed, data);
       // printf("Data read (size: %d) was: %.*s", sizeUsed, sizeUsed, data);

        //send back the data
        int sock = clientSock;
        char msg[] = "";


        if (strncmp(buffer,"ONON",2) == 0) {
            strcpy(msg, "LED is now on");
            gpio_set_level(BLINK_GPIO, 1);
        } else if (strncmp(buffer,"OOFF",3) == 0) {
            strcpy(msg, "LED is now off");
            gpio_set_level(BLINK_GPIO, 0);
        }else {
            strcpy(msg, "Got dummy text");
        }


        //socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        send(sock, msg, strlen(msg), 0);
        //ESP_LOGD(tag, "send: rc: %d", rc);

        //close outgoing socket
        //rc = close(sock);
        //ESP_LOGD(tag, "close: rc: %d", rc);

        //close incoming socket
        free(data);
        close(clientSock);
    }
    END:
    vTaskDelete(NULL);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void die(char *s)
{
    perror(s);
    exit(1);
}

void app_main()
{

    nvs_flash_init();
    tcpip_adapter_init();
    int level = 0;
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "KEFE",
        .ssid_len = 0,
            .password = "jelszo123",
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .beacon_interval = 400,
        .max_connection = 16,
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &ap_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

     /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);


    xTaskCreate(&socket_server, "socket_server", 2048, NULL, 5, NULL);
    //socket_server("test");

}

