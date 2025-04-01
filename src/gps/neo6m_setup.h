#ifndef NEO6M_SETUP_H
#define NEO6M_SETUP_H

#include <stddef.h> // size_t
#include "esp_err.h" 
#include "ubx.h"

typedef struct
{
  uint16_t MeasRate;
  uint16_t NavRate;
  uint16_t TimeRef;
} RATE_Setup_t;
static const RATE_Setup_t rateConfigs = {1000, 1, 0}; // 1000ms por medida, 1 medida por mensagem, tempo de referencia UTC
//Resposta:
//B5 62 | bytes de sincronismo UBX 
//05 01 | classe = 0x05 (ACK), ID = 0x01 (ACK-ACK) 
//02 00 | length = 2 bytes 
//06 08 | payload indicando ACK para a mensagem classe 0x06, ID 0x08 (CFG-RATE)
//16 3F | checksum
typedef struct
{
  uint8_t msgClass;
  uint8_t msgID;
  uint8_t I2C_rate;   // 0
  uint8_t UART1_rate; // para usar este 
  uint8_t UART2_rate; // 0
  uint8_t USB_rate;   // 0
  uint8_t SPI_rate;   // 0
  uint8_t Reserved;   // 0
} NMEA_Setup_t;
// TODO : juntar estas configuracoes nmeaConfigs[], com as configuracoes em Kconfig.projbuild, pois nmea_parser.c  só dispara o evento GPS_UPDATE quando todas as sentenças configuradas (“all_statements”) são recebidas pelo menos uma vez.
static const NMEA_Setup_t nmeaConfigs[] = // existem mais mostradas na documentacao
    {
      //DTM, GBS, GPQ, GRS, GST, THS, TXT, ZDA
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_GGA, 0, 1, 0, 0, 0, 0}, // GGA
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_GLL, 0, 0, 0, 0, 0, 0}, // GLL
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_GSA, 0, 0, 0, 0, 0, 0}, // GSA
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_GSV, 0, 0, 0, 0, 0, 0}, // GSV
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_RMC, 0, 0, 0, 0, 0, 0}, // RMC
        {MSG_CLASS_NMEA_STANDARD, MSG_ID_VTG, 0, 0, 0, 0, 0, 0}  // VTG
};
//Respostas (6x):
//B5 62 | bytes de sincronismo UBX 
//05 01 | classe = 0x05 (ACK), ID = 0x01 (ACK-ACK) 
//02 00 | length = 2 bytes 
//06 01 | payload indicando ACK para a mensagem classe 0x06, ID 0x01 (CFG-MSG)
//0F 38 | checksum

esp_err_t NEO6M_SetUp();

#endif