#include "unity.h"
#include "gps/neo6m_setup.h"
#include "project_settings.h"

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/uart.h"

static const char *TAG = "TEST_NEO6M_SETUP";

extern QueueHandle_t uart_queue;
// handle para a task, para apagar a task depois
static TaskHandle_t neo6mTaskHandle = NULL;
// variável atualizada pela task, para verificar no teste
static int gps_valid_data = 0;
static int gps_total_data = 0;
// variavel a comparar com os dados recebidos
const uint8_t gpgga_header[] = "$GPGGA";

void neo6m_test_task(void *param)
{
	uart_event_t event;
	while (xQueueReceive(uart_queue, &event, portMAX_DELAY))
	{
		switch (event.type)
		{
		case UART_DATA:
			uint8_t *data = (uint8_t *)malloc(event.size);
			if (data == NULL)
			{
				ESP_LOGE(TAG, "Failed to allocate memory for data");
			}
			int rxBytes = uart_read_bytes(GPS_UART_NUM, data, event.size, pdMS_TO_TICKS(100));
			if (rxBytes > 0)
			{
				ESP_LOG_BUFFER_HEXDUMP(TAG, data, rxBytes, ESP_LOG_INFO);
				ESP_LOGI(TAG, "gps_valid_data: %d", gps_valid_data);
				gps_total_data += 1;
				bool valid_data = true;
				for (int j=0; j<6; j++){
					ESP_LOGI(TAG, "data[%d]: %d, gpgga_header[%d]: %d", j, data[j], j, gpgga_header[j]);
					if (data[j] != gpgga_header[j]){
						valid_data = false;
						break;
					}
				}
				if (valid_data){
					gps_valid_data += 1;
				}
				/* ESP_LOGI(TAG, "UART_DATA-Dados recebidos na UART Queue (%d bytes): %.*s", rxBytes, rxBytes, data);
				if (is_ubx_packet(data, rxBytes)){
					ESP_LOGW(TAG, "UBX packet received");
				}
				for (int i = 0; i < rxBytes; i++)
				{
					printf("%02X ", data[i]); // Imprime cada byte em hexadecimal
					if ((i + 1) % 16 == 0)
						printf("\n"); // Quebra linha a cada 16 bytes
				}
				printf("\n"); */
			}
			else
			{
				ESP_LOGW(TAG, "No valid GPS data received");
			}
			free(data); // Liberar a memória alocada
			break;
		case UART_FIFO_OVF:
			// If fifo overflow happened, you should consider adding flow control for your application.
			// The ISR has already reset the rx FIFO, directly flush the rx buffer here in order to read more data
			ESP_LOGE(TAG, "FIFO Overflow! Flushing buffer");
			uart_flush_input(GPS_UART_NUM);
			xQueueReset(uart_queue);
			break;
		case UART_BUFFER_FULL:
			// If buffer full happened, you should consider increasing your buffer size
			// directly flush the rx buffer here in order to read more data
			ESP_LOGE(TAG, "Buffer RX Overflow! Flushing buffer");
			uart_flush_input(GPS_UART_NUM);
			xQueueReset(uart_queue);
			break;
		case UART_BREAK:
			ESP_LOGI(TAG, "uart rx break");
			break;
		case UART_PARITY_ERR:
			ESP_LOGI(TAG, "uart parity error");
			break;
		case UART_FRAME_ERR:
			ESP_LOGI(TAG, "uart frame error");
			break;
		case UART_PATTERN_DET:
			break;
#if SOC_UART_SUPPORT_WAKEUP_INT
		case UART_WAKEUP: /*!< Triggered when a wakeup signal is detected*/
			ESP_LOGI(TAG, "uart wakeup");
			break;
#endif
		case UART_EVENT_MAX: /*!< Maximum index for UART events*/
			ESP_LOGI(TAG, "uart event max");
			break;
		default:
			ESP_LOGI(TAG, "uart event type: %d", event.type);
			break;
		}
	}
}

void setUp(void) {
    esp_err_t err;

	const uart_config_t uart_config = {
		.baud_rate = 9600,				  	/*!< UART baud rate*/
		.data_bits = UART_DATA_8_BITS,		  	/*!< UART byte size*/
		.parity = UART_PARITY_DISABLE,		  	/*!< UART parity mode*/
		.stop_bits = UART_STOP_BITS_1,		  	/*!< UART stop bits*/
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE 	/*!< UART HW flow control mode (cts/rts)*/
		//.rx_flow_ctrl_thresh = 					/*!< UART HW RTS threshold*/
	};
	uart_driver_install(GPS_UART_NUM,
		GPS_UART_RX_BUFFER_SIZE, //
		GPS_UART_TX_BUFFER_SIZE, //
		10,				   // Tamanho da fila de eventos
		&uart_queue,		   // Ponteiro para a fila
		0);				   // Flags
	err = uart_param_config(GPS_UART_NUM, &uart_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "UART parameter configuration failed: %s", esp_err_to_name(err));
	}
	uart_set_pin(GPS_UART_NUM, 
		GPS_UART_TX, 				// TX
		GPS_UART_RX, 				// RX
			UART_PIN_NO_CHANGE, 	// RTS
			UART_PIN_NO_CHANGE); // CTS
    ESP_LOGI(TAG, "UART configured");
	gps_total_data = 0;
	gps_valid_data = 0;
}
  
void tearDown(void) {
	if (neo6mTaskHandle != NULL)
    {
        vTaskDelete(neo6mTaskHandle);
        neo6mTaskHandle = NULL;
    }
	uart_driver_delete(GPS_UART_NUM);
}
  
void test_function_NEO6M_SetUp(void) {
    TEST_ASSERT_EQUAL(ESP_OK, NEO6M_SetUp());
}
  
void test_gps_data_received(void) {
	TEST_ASSERT_EQUAL(ESP_OK, NEO6M_SetUp());
    xTaskCreatePinnedToCore(
		neo6m_test_task,			 // Função da tarefa
		"neo6m_test_task",			 // Nome da tarefa
		3072, // Tamanho do stack
		NULL,				 // Parâmetros
		GPS_TASK_PRIO,		 // Prioridade
		&neo6mTaskHandle,				 // Handle da tarefa
		GPS_TASK_CORE		 // Núcleo (CPU1)
		);
	// wait 3 seconds for the task to run
	vTaskDelay(pdMS_TO_TICKS(3000));
	TEST_ASSERT_EQUAL(gps_total_data, gps_valid_data);
}
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_NEO6M_SetUp);
	//RUN_TEST(test_gps_data_received);
    return UNITY_END();
}

void app_main() {
    runUnityTests();
}