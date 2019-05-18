# esp32injectfordummiess

Here is example code for esp_wifi_80211_tx or so called freedom output

I also found way to change modulation
//https://www.esp32.com/viewtopic.php?f=19&t=3017
Useful for highbandwidth experimentations



interface depends on which way you configured. Sta or ap

esp_wifi_80211_tx(WIFI_IF_STA, packet_buffer, size, false)
