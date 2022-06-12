/* MDNS-SD Query and advertise Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif_ip_addr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
// #include "mdns.h"
#include "driver/gpio.h"
#include "netdb.h"

#include "tcp_dap_interface.h"

#define EXAMPLE_MDNS_INSTANCE CONFIG_MDNS_INSTANCE

// static const char * TAG = "mdns-test";
// static char * generate_hostname(void);

// #if CONFIG_MDNS_RESOLVE_TEST_SERVICES == 1
// static void  query_mdns_host_with_gethostbyname(char * host);
// static void  query_mdns_host_with_getaddrinfo(char * host);
// #endif

// static void initialise_mdns(void)
// {
//     char * hostname = generate_hostname();

//     //initialize mDNS
//     ESP_ERROR_CHECK( mdns_init() );
//     //set mDNS hostname (required if you want to advertise services)
//     ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
//     ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
//     //set default mDNS instance name
//     ESP_ERROR_CHECK( mdns_instance_name_set(EXAMPLE_MDNS_INSTANCE) );

//     //structure with TXT records
//     mdns_txt_item_t serviceTxtData[3] = {
//         {"board", "esp32"},
//         {"u", "user"},
//         {"p", "password"}
//     };

//     //initialize service
//     ESP_ERROR_CHECK( mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData, 3) );
// // #if CONFIG_MDNS_MULTIPLE_INSTANCE
// //     ESP_ERROR_CHECK( mdns_service_add("ESP32-WebServer1", "_http", "_tcp", 80, NULL, 0) );
// // #endif

// // #if CONFIG_MDNS_PUBLISH_DELEGATE_HOST
// //     char *delegated_hostname;
// //     if (-1 == asprintf(&delegated_hostname, "%s-delegated", hostname)) {
// //         abort();
// //     }

// //     mdns_ip_addr_t addr4, addr6;
// //     esp_netif_str_to_ip4("10.0.0.1", &addr4.addr.u_addr.ip4);
// //     addr4.addr.type = ESP_IPADDR_TYPE_V4;
// //     esp_netif_str_to_ip6("fd11:22::1", &addr6.addr.u_addr.ip6);
// //     addr6.addr.type = ESP_IPADDR_TYPE_V6;
// //     addr4.next = &addr6;
// //     addr6.next = NULL;
// //     ESP_ERROR_CHECK( mdns_delegate_hostname_add(delegated_hostname, &addr4) );
// //     ESP_ERROR_CHECK( mdns_service_add_for_host("test0", "_http", "_tcp", delegated_hostname, 1234, serviceTxtData, 3) );
// //     free(delegated_hostname);
// // #endif // CONFIG_MDNS_PUBLISH_DELEGATE_HOST

//     //add another TXT item
//     ESP_ERROR_CHECK( mdns_service_txt_item_set("_http", "_tcp", "path", "/foobar") );
//     //change TXT item value
//     ESP_ERROR_CHECK( mdns_service_txt_item_set_with_explicit_value_len("_http", "_tcp", "u", "admin", strlen("admin")) );
//     free(hostname);
// }

// /* these strings match tcpip_adapter_if_t enumeration */
// static const char * if_str[] = {"STA", "AP", "ETH", "MAX"};

// /* these strings match mdns_ip_protocol_t enumeration */
// static const char * ip_protocol_str[] = {"V4", "V6", "MAX"};

// static void mdns_print_results(mdns_result_t *results)
// {
//     mdns_result_t *r = results;
//     mdns_ip_addr_t *a = NULL;
//     int i = 1, t;
//     while (r) {
//         printf("%d: Interface: %s, Type: %s, TTL: %u\n", i++, if_str[r->tcpip_if], ip_protocol_str[r->ip_protocol],
//                r->ttl);
//         if (r->instance_name) {
//             printf("  PTR : %s.%s.%s\n", r->instance_name, r->service_type, r->proto);
//         }
//         if (r->hostname) {
//             printf("  SRV : %s.local:%u\n", r->hostname, r->port);
//         }
//         if (r->txt_count) {
//             printf("  TXT : [%zu] ", r->txt_count);
//             for (t = 0; t < r->txt_count; t++) {
//                 printf("%s=%s(%d); ", r->txt[t].key, r->txt[t].value ? r->txt[t].value : "NULL", r->txt_value_len[t]);
//             }
//             printf("\n");
//         }
//         a = r->addr;
//         while (a) {
//             if (a->addr.type == ESP_IPADDR_TYPE_V6) {
//                 printf("  AAAA: " IPV6STR "\n", IPV62STR(a->addr.u_addr.ip6));
//             } else {
//                 printf("  A   : " IPSTR "\n", IP2STR(&(a->addr.u_addr.ip4)));
//             }
//             a = a->next;
//         }
//         r = r->next;
//     }
// }

// static void query_mdns_service(const char * service_name, const char * proto)
// {
//     ESP_LOGI(TAG, "Query PTR: %s.%s.local", service_name, proto);

//     mdns_result_t * results = NULL;
//     esp_err_t err = mdns_query_ptr(service_name, proto, 3000, 20,  &results);
//     if(err){
//         ESP_LOGE(TAG, "Query Failed: %s", esp_err_to_name(err));
//         return;
//     }
//     if(!results){
//         ESP_LOGW(TAG, "No results found!");
//         return;
//     }

//     mdns_print_results(results);
//     mdns_query_results_free(results);
// }

// static bool check_and_print_result(mdns_search_once_t *search)
// {
//     // Check if any result is available
//     mdns_result_t * result = NULL;
//     if (!mdns_query_async_get_results(search, 0, &result)) {
//         return false;
//     }

//     if (!result) {   // search timeout, but no result
//         return true;
//     }

//     // If yes, print the result
//     mdns_ip_addr_t * a = result->addr;
//     while (a) {
//         if(a->addr.type == ESP_IPADDR_TYPE_V6){
//             printf("  AAAA: " IPV6STR "\n", IPV62STR(a->addr.u_addr.ip6));
//         } else {
//             printf("  A   : " IPSTR "\n", IP2STR(&(a->addr.u_addr.ip4)));
//         }
//         a = a->next;
//     }
//     // and free the result
//     mdns_query_results_free(result);
//     return true;
// }

// static void query_mdns_hosts_async(const char * host_name)
// {
//     ESP_LOGI(TAG, "Query both A and AAA: %s.local", host_name);

//     mdns_search_once_t *s_a = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_A, 1000, 1, NULL);
//     mdns_query_async_delete(s_a);
//     mdns_search_once_t *s_aaaa = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_AAAA, 1000, 1, NULL);
//     while (s_a || s_aaaa) {
//         if (s_a && check_and_print_result(s_a)) {
//             ESP_LOGI(TAG, "Query A %s.local finished", host_name);
//             mdns_query_async_delete(s_a);
//             s_a = NULL;
//         }
//         if (s_aaaa && check_and_print_result(s_aaaa)) {
//             ESP_LOGI(TAG, "Query AAAA %s.local finished", host_name);
//             mdns_query_async_delete(s_aaaa);
//             s_aaaa = NULL;
//         }
//     }
// }

// static void query_mdns_host(const char * host_name)
// {
//     ESP_LOGI(TAG, "Query A: %s.local", host_name);

//     struct esp_ip4_addr addr;
//     addr.addr = 0;

//     esp_err_t err = mdns_query_a(host_name, 2000,  &addr);
//     if(err){
//         if(err == ESP_ERR_NOT_FOUND){
//             ESP_LOGW(TAG, "%s: Host was not found!", esp_err_to_name(err));
//             return;
//         }
//         ESP_LOGE(TAG, "Query Failed: %s", esp_err_to_name(err));
//         return;
//     }

//     ESP_LOGI(TAG, "Query A: %s.local resolved to: " IPSTR, host_name, IP2STR(&addr));
// }

void mdns_example_task(void *pvParameters)
{
//     initialise_mdns();
// // #if CONFIG_MDNS_RESOLVE_TEST_SERVICES == 1
// //     /* Send initial queries that are started by CI tester */
// //     query_mdns_host("tinytester");
// //     query_mdns_host_with_gethostbyname("tinytester-lwip.local");
// //     query_mdns_host_with_getaddrinfo("tinytester-lwip.local");
// // #endif

//     while (1) {
//         check_button();
//         vTaskDelay(50 / portTICK_PERIOD_MS);
//     }
}

// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     initialise_mdns();

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());

//     initialise_button();
//     xTaskCreate(&mdns_example_task, "mdns_example_task", 2048, NULL, 5, NULL);
// }

/** Generate host name based on sdkconfig, optionally adding a portion of MAC address to it.
 *  @return host name string allocated from the heap
 */
static char* generate_hostname(void)
{
// #ifndef CONFIG_MDNS_ADD_MAC_TO_HOSTNAME
    return strdup(TCP_HOST_NAME);
// #else
//     uint8_t mac[6];
//     char   *hostname;
//     esp_read_mac(mac, ESP_MAC_WIFI_STA);
//     if (-1 == asprintf(&hostname, "%s-%02X%02X%02X", CONFIG_MDNS_HOSTNAME, mac[3], mac[4], mac[5])) {
//         abort();
//     }
//     return hostname;
// #endif
}

