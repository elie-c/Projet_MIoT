#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "contiki.h"
#include "coap-engine.h"
#include "dev/uart.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* Buffer pour réception UART */
#define UART_BUFFER_SIZE 128
static char uart_rx_buffer[UART_BUFFER_SIZE];
static uint8_t uart_rx_index = 0;

/* Données reçues depuis l'UART */
static int uPan = 0;
static int uBat = 0;
static int cPan = 0;
static int cBat = 0;
static int soc = 0;
static int load_activated = 0;

/* Process pour gérer les lignes UART complètes */
PROCESS(uart_reader_process, "UART Reader");
process_event_t uart_line_event;

/* Parse une ligne UART reçue */
static void
parse_uart_line(char *line)
{
  char *colon;
  int value;
  
  /* Cherche le séparateur ':' */
  colon = strchr(line, ':');
  if(colon == NULL) {
    return;
  }
  
  *colon = '\0'; /* Sépare la clé de la valeur */
  value = atoi(colon + 1);
  
  /* Identifie et stocke la valeur */
  if(strcmp(line, "upan") == 0) {
    uPan = value;
    LOG_INFO("UART: uPan=%d\n", uPan);
  } else if(strcmp(line, "ubat") == 0) {
    uBat = value;
    LOG_INFO("UART: uBat=%d\n", uBat);
  } else if(strcmp(line, "cpan") == 0) {
    cPan = value;
    LOG_INFO("UART: cPan=%d\n", cPan);
  } else if(strcmp(line, "cbat") == 0) {
    cBat = value;
    LOG_INFO("UART: cBat=%d\n", cBat);
  } else if(strcmp(line, "soc") == 0) {
    soc = value;
    LOG_INFO("UART: soc=%d\n", soc);
  }
}

/* Callback UART - appelé à chaque caractère reçu */
static int
uart_rx_callback(unsigned char c)
{
  /* Ajoute le caractère au buffer */
  if(c == '\n' || c == '\r') {
    /* Fin de ligne détectée */
    if(uart_rx_index > 0) {
      uart_rx_buffer[uart_rx_index] = '\0'; /* Termine la chaîne */
      
      /* Post un event au process pour traiter la ligne */
      process_post(&uart_reader_process, uart_line_event, uart_rx_buffer);
      
      uart_rx_index = 0; /* Reset buffer */
    }
  } else if(uart_rx_index < UART_BUFFER_SIZE - 1) {
    uart_rx_buffer[uart_rx_index++] = c;
  } else {
    /* Buffer plein, reset */
    uart_rx_index = 0;
    LOG_WARN("UART buffer overflow\n");
  }
  
  return 1;
}

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
    if(strstr((const char *)payload, "\"activate\":1")) {
      load_activated = 1;
      LOG_INFO("POST /load -> Load ACTIVATED\n");
      
      /* Envoie la commande sur l'UART pour contrôler la charge */
      printf("load:1\n");
      
    } else if(strstr((const char *)payload, "\"activate\":0")) {
      load_activated = 0;
      LOG_INFO("POST /load -> Load DEACTIVATED\n");
      
      /* Envoie la commande sur l'UART pour contrôler la charge */
      printf("load:0\n");
      
    } else {
      coap_set_status_code(response, BAD_REQUEST_4_00);
      return;
    }
    
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

/* Process UART Reader */
PROCESS_THREAD(uart_reader_process, ev, data)
{
  PROCESS_BEGIN();
  
  LOG_INFO("UART Reader started\n");
  
  /* Alloue un event personnalisé pour les lignes UART */
  uart_line_event = process_alloc_event();
  
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == uart_line_event);
    
    /* Parse la ligne reçue */
    parse_uart_line((char *)data);
  }
  
  PROCESS_END();
}

/* Process CoAP Server */
PROCESS(coap_server_process, "CoAP Server");
AUTOSTART_PROCESSES(&uart_reader_process, &coap_server_process);

PROCESS_THREAD(coap_server_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  LOG_INFO("Starting CoAP Server with custom resources\n");
  
  /* Configure UART1 pour réception */
  uart_set_input(1, uart_rx_callback);

  
  /* Active les ressources CoAP */
  coap_activate_resource(&res_u, "u");
  coap_activate_resource(&res_c, "c");
  coap_activate_resource(&res_soc, "soc");
  coap_activate_resource(&res_load, "load");

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
