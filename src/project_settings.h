#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/uart.h"


//TODO : definir aqui todas as tags usadas e o nivel de log, estes niveis de log deveram ser alterados por Kconfig.projbuild
//esp_log_level_set(TAG, ESP_LOG_INFO);

//TODO : mudar estes valores para estarem de acordo com nmea_parser.h e alterar nmea_parser.c e nmea_parser.h para usar estes defines
#define GPS_UART_NUM UART_NUM_2 // UART_NUM_0 entra em conflito com o BLE e com o console, UART_NUM_1 entra em conflito com o SPI que é usado para a flash
#define GPS_UART_TX 17
#define GPS_UART_RX 16
#define GPS_UART_BUFFER_SIZE (1024 * 2)    // em principio na e preciso tanto
#define GPS_UART_RX_BUFFER_SIZE (1024 * 2) // em principio na e preciso tanto
#define GPS_UART_TX_BUFFER_SIZE 0          // apesar de ser usado, fica a 0 porque é so usado no inicio (ao ficar a 0 a operacao uart_write fica bloqueante até que todos os dados sejam transmitidos, ate pq o gps so deve funcionar depois de tudo estar inicializado)
#define GPS_TASK_CORE 1
#define GPS_TASK_PRIO (configMAX_PRIORITIES - 1) // configMAX_PRIORITIES = 25

#define GPS_CONFIGURATION()                                                                                                                                                                  \
  {\
      .uart = {\
          .baud_rate = 9600,\
          .data_bits = UART_DATA_8_BITS,\
          .parity = UART_PARITY_DISABLE,\
          .stop_bits = UART_STOP_BITS_1,\
          .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,\
          .source_clk = UART_SCLK_DEFAULT,\
      },\
      .uart_pins = {\
          .uart_port = GPS_UART_NUM,\
          .rx_pin = GPS_UART_RX,\
          .tx_pin = GPS_UART_TX,\
          .rts_pin = UART_PIN_NO_CHANGE,\
          .cts_pin = UART_PIN_NO_CHANGE,\
      },\
      .driver = {\
        .uart_port = GPS_UART_NUM, \
        .rx_buffer_size = GPS_UART_RX_BUFFER_SIZE, \
        .tx_buffer_size = GPS_UART_TX_BUFFER_SIZE, \
        .event_queue_size = 16, .flags = 0,\
        .event_queue_hdl = NULL},/* preenchido depois pela uart driver */\
  }
typedef struct {
    struct {
        uint32_t baud_rate;
        uart_word_length_t data_bits;
        uart_parity_t parity;
        uart_stop_bits_t stop_bits;
        uart_hw_flowcontrol_t flow_ctrl;
        uart_sclk_t source_clk;
    } uart;
    struct {
        uart_port_t uart_port;
        int rx_pin;
        int tx_pin;
        int rts_pin;
        int cts_pin;
    } uart_pins;
    struct {
        uart_port_t uart_port;
        uint32_t rx_buffer_size;
        uint32_t tx_buffer_size;
        uint32_t event_queue_size;
        uint32_t flags;           
        QueueHandle_t event_queue_hdl;/* para ser preenchido pela uart driver */     
    } driver;
} gps_config_t;
/**
 * @brief NMEA Parser runtime buffer size
 *
 */
#define NMEA_PARSER_RUNTIME_BUFFER_SIZE (GPS_UART_RX_BUFFER_SIZE / 2)
#define NMEA_MAX_STATEMENT_ITEM_LENGTH (16)
#define NMEA_EVENT_LOOP_QUEUE_SIZE (16)
/* // Valores: default do NMEA Parser
// CONFIG_NMEA_PARSER_TASK_STACK_SIZE para a task
// CONFIG_NMEA_PARSER_RING_BUFFER_SIZE em vez de GPS_UART_RX_BUFFER_SIZE
// CONFIG_NMEA_PARSER_TASK_PRIORITY em vez de GPS_TASK_PRIO */

#define TIME_ZONE (0)   //Lisbon Time
#define YEAR_BASE (2000) //date in GPS starts from 2000




#endif