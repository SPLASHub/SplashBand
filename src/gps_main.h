#ifndef H_GPS_MAIN_
#define H_GPS_MAIN_

#include <inttypes.h>  // Make sure to include this library for PRIu32
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>  // For formatting macros

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "esp_pm.h"   // to adjust the processor frequency

//UART_NUM_0 entra em conflito com o BLE e com o console
//UART_NUM_1 entra em conflito com o SPI que é usado para a flash
#define UART_NUM UART_NUM_2
//pinos conectados a uart por hardware, sem usar IO_MUX para melhor otimizacao
//caso precise usar outros pinos, configurar via uart_set_pin(), mas usará o GPIO matrix (menos eficiente).
#define CONFIG_UART_TX_GPS (gpio_num_t) GPIO_NUM_17 //GPIO_NUM_43
#define CONFIG_UART_RX_GPS (gpio_num_t) GPIO_NUM_16	//GPIO_NUM_44
#define UART_BUFFER_SIZE (1024 * 2)					// em principio na e preciso tanto
#define UART_RX_BUFFER_SIZE (1024 * 2) 				// em principio na e preciso tanto 
#define UART_TX_BUFFER_SIZE 0	 // apesar de ser usado, fica a 0 porque é so usado no inicio (ao ficar a 0 a operacao uart_write fica bloqueante até que todos os dados sejam transmitidos, ate pq o gps so deve funcionar depois de tudo estar inicializado)
extern QueueHandle_t uart_queue; // Declare a fila como externa
#define GPS_TAG "GPS_Module"

// Error codes for the GPS module
#define ESP_ERR_GPS_UART_PARAM_CONFIG_FAILED 0x3001
#define ESP_ERR_GPS_UART_SET_PIN_FAILED 0x3002
#define ESP_ERR_GPS_UART_DRIVER_INSTALL_FAILED 0x3003
// Structure for storing GPS information
typedef struct {
  float latitude;
  float longitude;
  float altitude;
  float speed;
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
  bool valid;
} GPSData_t;

void app_gps_main(void);

#endif