#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* === Configuration sécurité 802.15.4 (Serveur) === */

/* Activer la sécurité au niveau liaison 802.15.4 */
#define LLSEC802154_CONF_ENABLED 1

/* Clé K1 : Authentification des Enhanced Beacons (128 bits) */
#define TSCH_SECURITY_CONF_K1 { 0x11, 0x11, 0x11, 0x11, \
                                0x11, 0x11, 0x11, 0x11, \
                                0x11, 0x11, 0x11, 0x11, \
                                0x11, 0x11, 0x11, 0x11 }

/* Clé K2 : Chiffrement/Authentification des trames DATA (128 bits) */
#define TSCH_SECURITY_CONF_K2 { 0x22, 0x22, 0x22, 0x22, \
                                0x22, 0x22, 0x22, 0x22, \
                                0x22, 0x22, 0x22, 0x22, \
                                0x22, 0x22, 0x22, 0x22 }

/* === Logs de debug === */

/* Logs application */
#define LOG_LEVEL_APP LOG_LEVEL_DBG

/* Logs CoAP */
#define LOG_CONF_LEVEL_COAP LOG_LEVEL_DBG

/* Active UART1 */
#define UART1_CONF_ENABLE 1

/* Baud rate pour UART1 (par défaut 115200) */
#define UART1_CONF_BAUD_RATE 115200

#endif /* PROJECT_CONF_H_ */
