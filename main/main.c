#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "stdio.h" //printf
#include "string.h" //memset
#include "stdlib.h" //exit(0);
#include "arpa/inet.h"
#include "lwip/sockets.h"

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void die(char *s)
{
    perror(s);
    exit(1);
}

void app_main(void)
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
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);

    //gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    //int level = 0;
    //while (true) {
    //    gpio_set_level(GPIO_NUM_4, level);
    //    level = !level;
    //    vTaskDelay(300 / portTICK_PERIOD_MS);
    //}


	//lets begin with UDP server

    void udp_server () {
    struct sockaddr_in si_me, si_other;
     
    int s, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = lwip_recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, slen)) == -1)
        {
            die("lwip_recvfrom()");
        }
         
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , buf);

        if ( strcmp( buf, "test") == 0 ) {
            gpio_set_level(GPIO_NUM_4, level);
            level = !level;
        }
         
        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
    }
 
    close(s);
    }





}

