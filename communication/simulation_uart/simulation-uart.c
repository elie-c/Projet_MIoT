#include "contiki.h"
#include "dev/uart.h"
#include <stdio.h>
#include <string.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/*---------------------------------------------------------------------------*/
PROCESS(uart_sender_process, "UART sender process");
AUTOSTART_PROCESSES(&uart_sender_process);

/*---------------------------------------------------------------------------*/
/* Fonction helper pour envoyer une chaîne sur UART1 */
static void
uart1_send_string(const char *str)
{
  while(*str != '\0') {
    uart_write_byte(1, *str);  /* 1 = UART1 */
    str++;
  }
}

/*---------------------------------------------------------------------------*/
/* Fonction helper pour envoyer un nombre formaté sur UART1 */
static void
uart1_send_data(const char *label, uint16_t value, uint8_t digits)
{
  char buffer[16];
  
  /* Formate la chaîne : "label:XXXX\n" */
  if(digits == 2) {
    snprintf(buffer, sizeof(buffer), "%s:%02u\n", label, value);
  } else {
    snprintf(buffer, sizeof(buffer), "%s:%04u\n", label, value);
  }
  
  uart1_send_string(buffer);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(uart_sender_process, ev, data)
{
  static struct etimer timer;
  
  /* Variables pour stocker les valeurs à envoyer */
  static uint16_t upan = 0;
  static uint16_t ubat = 0;
  static uint16_t cpan = 0;
  static uint16_t cbat = 0;
  static uint8_t soc = 0;

  PROCESS_BEGIN();

  /* Configuration UART1 - Baud rate déjà configuré par défaut à 115200 */
  printf("UART1 sender started - sending to JP3\n");

  /* Setup a periodic timer that expires after 1 second. */
  etimer_set(&timer, CLOCK_SECOND * 3);

  while(1) {
    /* Ici tu peux mettre à jour tes valeurs depuis des capteurs/ADC */
    /* Exemple avec des valeurs fictives qui s'incrémentent : */
    upan = (upan + 10) % 10000;
    ubat = (ubat + 15) % 10000;
    cpan = (cpan + 5) % 10000;
    cbat = (cbat + 8) % 10000;
    soc = (soc + 1) % 100;

    /* Envoi des données sur UART1 (JP3) */
    uart1_send_data("upan", upan, 4);
    LOG_INFO("Sending upan: %u\n", upan);
    uart1_send_data("ubat", ubat, 4);
    LOG_INFO("Sending ubat: %u\n", ubat);
    uart1_send_data("cpan", cpan, 4);
    LOG_INFO("Sending cpan: %u\n", cpan);
    uart1_send_data("cbat", cbat, 4);
    LOG_INFO("Sending cbat: %u\n", cbat);
    uart1_send_data("soc", soc, 2);
    LOG_INFO("Sending soc: %u\n", soc);

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
