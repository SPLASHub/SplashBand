// neo6m_setup.c
#include <string.h> // memcpy, memset, strlen, etc
#include <stdlib.h> // malloc, free, strtol, strtof, etc
#include "esp_log.h"
#include "driver/uart.h"
#include "neo6m_setup.h"
#include "project_settings.h"


static const char *TAG = "NEO6M_SETUP";

static esp_err_t write_ubx_task(const char *data, size_t ubxSize)
{
	const int len = strlen(data);
	const int txBytes = uart_write_bytes(GPS_UART_NUM, data, ubxSize); // bloqueia (se necessário) até conseguir copiar todos os bytes para o buffer interno de transmissão.
																   // uart_write_bytes_async() // tenta enfileirar os bytes de forma não bloqueante, retornando imediatamente quantos bytes efetivamente foram aceitos no buffer.
	if (txBytes != ubxSize)
	{
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "txBytes: %d bytes, ubxSize: %d bytes, len: %d bytes", txBytes, ubxSize, len);
	return ESP_OK;
}
static esp_err_t read_ubx_task(const char *data_sended, size_t data_sended_size)
{
    uint8_t* rxBuffer = (uint8_t*) malloc(GPS_UART_RX_BUFFER_SIZE + 1);// FIXME: definir um UART_RX_BUFFER_SIZE de tamanho mais apropriado para receber apenas dados ubx
	if(!rxBuffer){
		ESP_LOGE(TAG,"Error allocating memory for rxBuffer");
		return ESP_FAIL;
	}
	//uint8_t rx_buffer[UART_RX_BUFFER_SIZE + 1]; //!: será melhor assim ou como na linha de cima
	// Lê até o buffer completo, bloqueando durante 200ms
    //const int rxBytes = uart_read_bytes(UART_NUM, rxBuffer, ACK_RX_BUFFER_SIZE, 1000/portTICK_PERIOD_MS); //!: nao funciona assim pq enquanto espera o ubx package pode ja ter recebido dados gps, mesmo dando fush ao buffer_ring da driver
	const int rxBytes = uart_read_bytes(GPS_UART_NUM, rxBuffer, GPS_UART_RX_BUFFER_SIZE, 200/portTICK_PERIOD_MS);
	//procurar o pacote ubx:
	if (rxBytes <= 0){
		ESP_LOGE(TAG,"rxBytes <= 0");
		free(rxBuffer);
		return ESP_FAIL;
	}
	ESP_LOGI(TAG,"rxBytes: %d",rxBytes);
	int start_ubx_package = -1;
	for (int j=0; j<rxBytes; j++){
		if (rxBuffer[j] == 0xB5){
			start_ubx_package = j;
			break;
		}
	}
	if (start_ubx_package == -1){
		ESP_LOGE(TAG, "Error: no ubx package found");
		free(rxBuffer);
		return ESP_FAIL;
	}
	// Checar se há bytes suficientes no buffer
	if ((start_ubx_package + 10) > rxBytes) {
		ESP_LOGE(TAG, "Pacote UBX incompleto (faltam bytes)");
		free(rxBuffer);
		return ESP_FAIL;
	}
	uint8_t package[10];
	memcpy(package, rxBuffer + start_ubx_package, 10);
    rxBuffer[rxBytes] = 0;
//#ifdef UNIT_TEST
	ESP_LOG_BUFFER_HEXDUMP(TAG, rxBuffer, rxBytes, ESP_LOG_INFO);
	ESP_LOG_BUFFER_HEXDUMP(TAG, package, 10, ESP_LOG_INFO);
//# endif
	ubx_t code = CFG_acknowledged(package, 10);
	if (code.code != ACK.code){
		ESP_LOGE(TAG, "Erro na condfiguracao: %s", code.msg);
		free(rxBuffer);
		return ESP_FAIL;
	}

    free(rxBuffer);
	return ESP_OK;
}
esp_err_t NEO6M_SetUp()
{

	esp_err_t err;
	size_t ubxSize;

	uint8_t *sendBuffer = create_config_rate_message(rateConfigs.MeasRate, rateConfigs.NavRate, rateConfigs.TimeRef, &ubxSize);
	if (sendBuffer == NULL)
	{
		ESP_LOGE(TAG, "Falha ao configurar taxa de atualização");
		return ESP_FAIL;
	}
	else
	{
		uart_flush_input(GPS_UART_NUM); // limpar dados no ring_buffer_rx antes das operacoes
		uart_flush(GPS_UART_NUM); // so para garantir, limpar tb o ring_buffer_tx
		err = write_ubx_task((const char *)sendBuffer, ubxSize);
		if (err == ESP_FAIL){
			ESP_LOGE(TAG, "Failed to send UBX-CFG-RATE command");
			free(sendBuffer);
			return ESP_FAIL;
		}
		// Adiciona à fila de comandos pendentes
		// add_pending_command(UBX_CLASS_CFG, UBX_ID_CFG_RATE, sendBuffer, ubxSize);
		ESP_LOGI(TAG, "Taxa de atualização configurada com MeasRate=%u, NavRate=%u, TimeRef=%u",
				 rateConfigs.MeasRate, rateConfigs.NavRate, rateConfigs.TimeRef);
		err = read_ubx_task((const char *)sendBuffer, ubxSize);
		if (err == ESP_FAIL){
			free(sendBuffer);
			ESP_LOGE(TAG, "Failed to send UBX-CFG-RATE command");
			return ESP_FAIL;
		}
		free(sendBuffer);
	}

	int counter = 0; // apenar para prevencao de ficar preso
	int count_nmeaConfigs = sizeof(nmeaConfigs) / sizeof(nmeaConfigs[0]);
	for (int i = 0; i < count_nmeaConfigs; i++)
	{
		if (counter > count_nmeaConfigs){
			ESP_LOGE(TAG, "Error: counter > count_nmeaConfigs");
			return ESP_FAIL;
		}
		uint8_t *sendBuffer = create_config_nmea_message(nmeaConfigs[i].msgClass, nmeaConfigs[i].msgID, nmeaConfigs[i].I2C_rate, nmeaConfigs[i].UART1_rate, nmeaConfigs[i].UART2_rate, nmeaConfigs[i].USB_rate, nmeaConfigs[i].SPI_rate, nmeaConfigs[i].Reserved, &ubxSize);
		if (sendBuffer == NULL)
		{
			ESP_LOGE(TAG, "Falha ao configurar NMEA %d (cls=0x%02X, id=0x%02X)",
					 i, nmeaConfigs[i].msgClass, nmeaConfigs[i].msgID);
			return ESP_FAIL;
		}
		uart_flush_input(GPS_UART_NUM); // limpar dados no ring_buffer_rx antes das operacoes
		uart_flush(GPS_UART_NUM); // so para garantir, limpar tb o ring_buffer_tx
		err = write_ubx_task((const char *)sendBuffer, ubxSize);
		if (err == ESP_FAIL){
			ESP_LOGE(TAG, "Failed to send UBX-CFG-MSG command");
			free(sendBuffer);
			return ESP_FAIL;
		}
		ESP_LOGI(TAG, "NMEA %d - configurada com rate = %u",
				i, nmeaConfigs[i].UART1_rate);
		err = read_ubx_task((const char *)sendBuffer, ubxSize);
		if (err == ESP_FAIL){
			free(sendBuffer);
			ESP_LOGE(TAG, "Failed to send UBX-CFG-RATE command");
			return ESP_FAIL;
		}
		free(sendBuffer);
		// Pequeno delay entre envios (experientar caso ocorram erros)
		// vTaskDelay(pdMS_TO_TICKS(50));
	}
	return ESP_OK;
}
