//
//   weak-wlan-router - ESP8266-based WLAN NAT "Router"
//
//   jens, 20200122
//
//   based on: https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/RangeExtender-NAPT/RangeExtender-NAPT.ino
//   see also work of https://github.com/martin-ger
//


// #if LWIP_FEATURES && !LWIP_IPV6        needed configuration

// include your local secrets ...
#include "credentials.h"

// ... or here
// wifi credentials, upstream (existing WLAN, role: STA) and downstream (new access point, role: AP)
//const char* up_wlan_ssid = "WLAN-NAME";
//const char* up_wlan_pass = "WLAN-PASSWORD";
//const char* down_wlan_ssid = "WLAN-NAME";
//const char* down_wlan_pass = "WLAN-PASSWORD";
//


#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <dhcpserver.h>

/* size of the tables used for NAPT */
#define NAPT 1000
#define NAPT_PORT 10


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nWeak WLAN Router\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  WiFi.begin(up_wlan_ssid, up_wlan_pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nSTA: %s (dns: %s / %s)\n",
                WiFi.localIP().toString().c_str(),
                WiFi.dnsIP(0).toString().c_str(),
                WiFi.dnsIP(1).toString().c_str());

  // give DNS servers to AP side
  dhcps_set_dns(0, WiFi.dnsIP(0));
  dhcps_set_dns(1, WiFi.dnsIP(1));

  WiFi.softAPConfig(  // enable AP, with android-compatible google domain --> TODO
    IPAddress(172, 217, 28, 254),
    IPAddress(172, 217, 28, 254),
    IPAddress(255, 255, 255, 0));
  WiFi.softAP(down_wlan_ssid, down_wlan_pass);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init: ret=%d (OK=%d), table (%d,%d)\n", (int)ERR_OK, NAPT, NAPT_PORT, (int)ret );
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf("WiFi network '%s' with is now available via '%s'\n", down_wlan_ssid, up_wlan_ssid);
    }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) {
    Serial.printf("NAPT initialization failed\n");
  }
}


void loop() {
  Serial.print("Stations connected: ");
  Serial.println(WiFi.softAPgetStationNum());
  delay(5000);
}
