#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include <Arduino.h>

// #ifndef ETH_ADDR
//     #define ETH_ADDR        0
//     #define ETH_POWER_PIN   -1
//     #define ETH_MDC_PIN     23
//     #define ETH_MDIO_PIN    18
//     #define ETH_TYPE        ETH_PHY_RTL8201
//     #define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
// #endif

#include <ETH.h>

static volatile bool eth_link_up = false;

static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};

    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        Serial.println("Ethernet Link Up");
        Serial.printf("Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x\n",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        eth_link_up = true;
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        Serial.println("Ethernet Link Down");
        eth_link_up = false;
        break;
    case ETHERNET_EVENT_START:
        Serial.println("Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        Serial.println("Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    switch (event_id) 
    {
        case IP_EVENT_STA_GOT_IP: Serial.println("IP_EVENT_STA_GOT_IP");               /*!< station got IP from connected AP */
        case IP_EVENT_STA_LOST_IP: Serial.println("IP_EVENT_STA_LOST_IP");              /*!< station lost IP and the IP is reset to 0 */
        case IP_EVENT_AP_STAIPASSIGNED: Serial.println("IP_EVENT_AP_STAIPASSIGNED");         /*!< soft-AP assign an IP to a connected station */
        case IP_EVENT_GOT_IP6: Serial.println("IP_EVENT_GOT_IP6");                  /*!< station or ap or ethernet interface v6IP addr is preferred */
        case IP_EVENT_ETH_GOT_IP: Serial.println("IP_EVENT_ETH_GOT_IP");               /*!< ethernet got IP from connected AP */
        case IP_EVENT_ETH_LOST_IP: Serial.println("IP_EVENT_ETH_LOST_IP");              /*!< ethernet lost IP and the IP is reset to 0 */
        case IP_EVENT_PPP_GOT_IP: Serial.println("IP_EVENT_PPP_GOT_IP");               /*!< PPP interface got IP */
        case IP_EVENT_PPP_LOST_IP: Serial.println("IP_EVENT_PPP_LOST_IP");              /*!< PPP interface lost IP */

    }

    if (event_id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        const esp_netif_ip_info_t *ip_info = &event->ip_info;

        Serial.println("Ethernet Got IP Address");
        Serial.printf("ETHIP:" IPSTR "\n", IP2STR(&ip_info->ip));
        Serial.printf("ETHMASK:" IPSTR "\n", IP2STR(&ip_info->netmask));
        Serial.printf("ETHGW:" IPSTR "\n", IP2STR(&ip_info->gw));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Connecting LAN");

    WiFi.mode(WIFI_OFF); 

    //delay(1000);

    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);
    esp_log_level_set("eth", ESP_LOG_VERBOSE);
    esp_log_level_set("dhcpc", ESP_LOG_VERBOSE);   

    //ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

    esp_netif_init();
    esp_event_loop_create_default(); // create a default event loop that running in background
    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL); // register Ethernet event handler
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &got_ip_event_handler, NULL);

    //while (!ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE))
    while (!ETH.begin())
    {
        Serial.print(".");
    }

    ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    ETH.setHostname("esp32");

    //ETH.begin();
    
    //while (!ETH.linkUp())
    while (!eth_link_up)
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("IP address: ");
    Serial.println(ETH.localIP());

    Serial.print("Gateway:    ");
    Serial.println(ETH.gatewayIP());
}

void loop() {

}
