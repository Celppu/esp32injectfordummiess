
#include "freertos/FreeRTOS.h"

#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"


#include "nvs_flash.h"
#include "string.h"



#include <WiFi.h>

uint8_t channell = 13;

// Access point MAC
uint8_t ap[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Client MAC 
//uint8_t clientt[6] = {0x06,0x07,0x08,0x09,0x0A,0x0B};
uint8_t clientt[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[1000] = {0};


esp_err_t event_handler(void *ctx, system_event_t *event) {
  return ESP_OK;
}
//https://www.esp32.com/viewtopic.php?f=19&t=3017
//rates:
/*
0 - B 1Mb CCK
1 - B 2Mb CCK
2 - B 5.5Mb CCK
3 - B 11Mb CCK
4 - XXX Not working. Should be B 1Mb CCK SP
5 - B 2Mb CCK SP
6 - B 5.5Mb CCK SP
7 - B 11Mb CCK SP

8 - G 48Mb ODFM
9 - G 24Mb ODFM
10 - G 12Mb ODFM
11 - G 6Mb ODFM
12 - G 54Mb ODFM
13 - G 36Mb ODFM
14 - G 18Mb ODFM
15 - G 9Mb ODFM

16 - N 6.5Mb MCS0
17 - N 13Mb MCS1
18 - N 19.5Mb MCS2
19 - N 26Mb MCS3
20 - N 39Mb MCS4
21 - N 52Mb MCS5
22 - N 58Mb MCS6
23 - N 65Mb MCS7

24 - N 7.2Mb MCS0 SP
25 - N 14.4Mb MCS1 SP
26 - N 21.7Mb MCS2 SP
27 - N 28.9Mb MCS3 SP
28 - N 43.3Mb MCS4 SP
29 - N 57.8Mb MCS5 SP
30 - N 65Mb MCS6 SP
31 - N 72Mb MCS7 SP
*/
typedef union {
     uint8_t fix_rate;
     uint8_t b5;
     uint8_t b4;
 
     struct {
         uint8_t b3;
         uint8_t b2;
     } b1;
 
     struct {
         uint32_t a1;
         uint8_t  a2;
         uint8_t  a3;
         uint8_t  a4;
         uint8_t  a5;
         struct {
             uint8_t a6;
             uint8_t a7;
         } a8[4];
         uint8_t a9;
         uint8_t a10;
         uint8_t a11;
         uint8_t a12;
     } a13;
 
 } wifi_internal_rate_t;

extern "C" {
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
  esp_err_t esp_wifi_internal_set_rate(int a, int b, int c, wifi_internal_rate_t *d);
}


uint16_t empty_data_packet(uint8_t *buf, uint8_t *clientt, uint8_t *ap, uint16_t seq)
{
    int i=0;
    
    buf[0] = 0b00001000; //oikea järjestys
    buf[1] = 0b00000001; //flipattu

    // Duration 0 msec, will be re-written by ESP
    buf[2] = 0x00;
    buf[3] = 0x00;
    // Destination
    for (i=0; i<6; i++) buf[i+4] = clientt[i];
    // Sender
    for (i=0; i<6; i++) buf[i+10] = ap[i];
    for (i=0; i<6; i++) buf[i+16] = ap[i];
    // Seq_n
    buf[22] = seq % 0xFF;
    buf[23] = seq / 0xFF;

    for (i=24; i<1000; i++) buf[i] = 0xFF;

    return 1000;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  tcpip_adapter_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  wifi_config_t sta_config;
  
  sta_config.sta.channel = 13;

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

  ESP_ERROR_CHECK(esp_wifi_start());


  wifi_internal_rate_t rate;  
  rate.fix_rate = 15; //rate number from table
  
  esp_wifi_internal_set_rate(100, 1, 4, &rate);
  esp_wifi_set_channel(13, WIFI_SECOND_CHAN_NONE);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t size = empty_data_packet(packet_buffer, clientt, ap, seq_n+0x10);
  Serial.println(esp_wifi_80211_tx(WIFI_IF_STA, packet_buffer, size, false));
  delay(1);
}
