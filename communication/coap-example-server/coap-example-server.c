#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "contiki.h"
#include "coap-engine.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* Données factices simulées */
static int uPan = 1234;
static int uBat = 4321;
static int cPan = 5678;
static int cBat = 8765;
static int soc = 42;
static int load_activated = 0;

/* Ressource GET /u */
static void res_get_u_handler(coap_message_t *request, coap_message_t *response,
                               uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_u,
         "title=\"Upan and Ubat\";rt=\"JSON\"",
         res_get_u_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_u_handler(coap_message_t *request, coap_message_t *response,
                  uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  unsigned int accept = 0;
  
  /* Check Accept header for JSON */
  if(coap_get_header_accept(request, &accept) == 0 ||
     accept == APPLICATION_JSON) {
    
    len = snprintf((char *)buffer, preferred_size,
                   "{\"uPan\":%d,\"uBat\":%d}",
                   uPan, uBat);
    
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_payload(response, buffer, len);
    
    LOG_INFO("GET /u -> uPan=%d, uBat=%d\n", uPan, uBat);
  } else {
    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
  }
}

/* Ressource GET /c */
static void res_get_c_handler(coap_message_t *request, coap_message_t *response,
                               uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_c,
         "title=\"Cpan and Cbat\";rt=\"JSON\"",
         res_get_c_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_c_handler(coap_message_t *request, coap_message_t *response,
                  uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  unsigned int accept = 0;
  
  if(coap_get_header_accept(request, &accept) == 0 ||
     accept == APPLICATION_JSON) {
    
    len = snprintf((char *)buffer, preferred_size,
                   "{\"cPan\":%d,\"cBat\":%d}",
                   cPan, cBat);
    
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_payload(response, buffer, len);
    
    LOG_INFO("GET /c -> cPan=%d, cBat=%d\n", cPan, cBat);
  } else {
    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
  }
}

/* Ressource GET /soc */
static void res_get_soc_handler(coap_message_t *request, coap_message_t *response,
                                uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_soc,
         "title=\"State of Charge\";rt=\"JSON\"",
         res_get_soc_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_soc_handler(coap_message_t *request, coap_message_t *response,
                    uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  unsigned int accept = 0;
  
  if(coap_get_header_accept(request, &accept) == 0 ||
     accept == APPLICATION_JSON) {
    
    len = snprintf((char *)buffer, preferred_size,
                   "{\"soc\":%d}",
                   soc);
    
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_payload(response, buffer, len);
    
    LOG_INFO("GET /soc -> soc=%d\n", soc);
  } else {
    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
  }
}

/* Ressource POST /load */
static void res_post_load_handler(coap_message_t *request, coap_message_t *response,
                                   uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_load,
         "title=\"Load Control\";rt=\"JSON\"",
         NULL,
         res_post_load_handler,
         NULL,
         NULL);

static void
res_post_load_handler(coap_message_t *request, coap_message_t *response,
                      uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *payload = NULL;
  size_t len = 0;
  
  len = coap_get_payload(request, &payload);
  
  if(len > 0) {
    /* Parse simple JSON: {"activate":1} ou {"activate":0} */
    if(strstr((const char *)payload, "\"activate\":1")) {
      load_activated = 1;
      LOG_INFO("POST /load -> Load ACTIVATED\n");
    } else if(strstr((const char *)payload, "\"activate\":0")) {
      load_activated = 0;
      LOG_INFO("POST /load -> Load DEACTIVATED\n");
    } else {
      coap_set_status_code(response, BAD_REQUEST_4_00);
      return;
    }
    
    /* Réponse avec état actuel */
    len = snprintf((char *)buffer, preferred_size,
                   "{\"activate\":%d}",
                   load_activated);
    
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_status_code(response, CHANGED_2_04);
    coap_set_payload(response, buffer, len);
    
  } else {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}

PROCESS(coap_server_process, "CoAP Server");
AUTOSTART_PROCESSES(&coap_server_process);

PROCESS_THREAD(coap_server_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  LOG_INFO("Starting CoAP Server with custom resources\n");

  /* Activer les ressources */
  coap_activate_resource(&res_u, "u");
  coap_activate_resource(&res_c, "c");
  coap_activate_resource(&res_soc, "soc");
  coap_activate_resource(&res_load, "load");

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
