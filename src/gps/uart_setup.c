#include "esp_log.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "project_settings.h"
#include "nmea_parser.h" // para usar a estrutura esp_gps_t e a funcao nmea_parser_init
#include "neo6m_setup.h" // para usar a funcao NEO6M_SetUp
#include "uart_setup.h"
#include "ble/gatt_srv.h"

static const char *TAG = "GPS_UART_SETUP";

// TODO : Alterar esta funcao 
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    //ESP_LOGI(TAG, "Evento GPS disparado: ID=%" PRId32, event_id);
    //update_gpgga_data((const char *)data);
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                 "\t\t\t\t\t\tspeed      = %fm/s",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
                 gps->latitude, gps->longitude, gps->altitude, gps->speed);
        gps_t local_gps = *(gps_t *)event_data; // assim envia uma copia para nao haver risco dos dados serem alterados durante a execucao
        update_location_speed_data(local_gps);
        update_location_speed_json(local_gps);
        break;
    case GPS_UNKNOWN:
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}


esp_err_t GPS_UART_SetUp(void) {
    esp_err_t err;
    gps_config_t gps_config = GPS_CONFIGURATION();

	const uart_config_t uart_config = {
		.baud_rate = gps_config.uart.baud_rate,				  	/*!< UART baud rate*/
		.data_bits = gps_config.uart.data_bits,		  	/*!< UART byte size*/
		.parity = gps_config.uart.parity,		  	/*!< UART parity mode*/
		.stop_bits = gps_config.uart.stop_bits,		  	/*!< UART stop bits*/
		.flow_ctrl = gps_config.uart.flow_ctrl, 	/*!< UART HW flow control mode (cts/rts)*/
        .source_clk = gps_config.uart.source_clk,
		//.rx_flow_ctrl_thresh = 					/*!< UART HW RTS threshold*/
	};
	err = uart_driver_install(gps_config.driver.uart_port,
		gps_config.driver.rx_buffer_size, //
		gps_config.driver.tx_buffer_size, //
		gps_config.driver.event_queue_size,				   // Tamanho da fila de eventos
		&gps_config.driver.event_queue_hdl,		   // Ponteiro para a fila
		gps_config.driver.flags);				   // Flags
    if (err != ESP_OK){
        ESP_LOGE(TAG, "UART driver installation failed: %s", esp_err_to_name(err));
        goto err_uart_config;
    }
	err = uart_param_config(GPS_UART_NUM, &uart_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "UART parameter configuration failed: %s", esp_err_to_name(err));
        goto err_uart_config;
	}
	err = uart_set_pin(gps_config.uart_pins.uart_port, 
        gps_config.uart_pins.tx_pin, 			// TX
        gps_config.uart_pins.rx_pin, 			// RX
        gps_config.uart_pins.rts_pin, 	        // RTS
        gps_config.uart_pins.cts_pin);          // CTS
    if (err != ESP_OK){
        ESP_LOGE(TAG, "UART set pin failed: %s", esp_err_to_name(err));
        goto err_uart_config;
    }

    err = NEO6M_SetUp();
    if (err != ESP_OK){
        ESP_LOGE(TAG, "NEO6M_SetUp failed: %s", esp_err_to_name(err));
        goto err_uart_config;
    }
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&gps_config);
    if (nmea_hdl == NULL) {
        ESP_LOGE(TAG, "NMEA parser initialization failed");
        // em caso de falha a funcao ja elimina o driver
        return ESP_FAIL;
        
    }
   err =  nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
   if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add event handler: %s", esp_err_to_name(err));
        nmea_parser_remove_handler(nmea_hdl, gps_event_handler);
        nmea_parser_deinit(nmea_hdl); // ja elimina driver
        return err;
    }
    ESP_LOGI(TAG, "GPS UART setup completed successfully");
    return ESP_OK;

err_uart_config:
    uart_driver_delete(gps_config.driver.uart_port);
    return err;
}