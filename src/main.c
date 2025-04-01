//TODO - Experimentar semaforos para sincronizar gps e ble
//TODO - Experimentar um buffer circular para o gps
//TODO - Limpar todas as bibliotecas de todos os arquivos para ficar os com as bibliotecas necessarios para cada arquivo:
// .h                       	                                | .c
// Propaga os #include para outros arquivos que incluem prog.h  | Restringe os #include ao arquivo prog.c
// Para dependências necessárias às declarações públicas.       | Para dependências usadas apenas na implementação.
// Aumenta o tempo de compilação quantos mais #include prog.h.  | Reduz o impacto na compilação, pois os #include são limitados ao ble.c.
// Para dependências compartilhadas.                            | Para dependências internas.
//TODO - fazer arquivo "project_settings.h" conterá todas as macros, constantes, estruturas e definições globais.
//TODO - fazer programa de programacao da flash do neo6m para nao ser necessario estar sempre a programar a ram no neo6m- provavelmente no neo6m_setup, confirma a configuracao, se nao for a esperada, faz a programacao da ram e depois exporta para a flash
//#include "ble/ble_prh_main.h" // usado neste codigo para iniciar o ble
//#include "ble/gatt_srv.h"	  // usado neste codigo para update_gpgga_data e update_gprmc_data
#ifdef UNIT_TEST
// não implementa a app_main quando está em teste
#else

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps/uart_setup.h" // usado neste codigo para iniciar o gps
#include "ble/ble_prh_main.h" 

static const char *TAG = "MAIN";
void app_main(void)
{
	app_ble_prh_main(); // inicia o ble
	esp_err_t err = GPS_UART_SetUp();
	if (err == ESP_OK)
	{
		ESP_LOGI(TAG, "GPS UART task initialized successfully");
	}
	else
	{
		ESP_LOGE(TAG, "GPS UART task initialization failed: %s", esp_err_to_name(err));
		return;
	}
	/* while (1) {
        ESP_LOGI(TAG, "I'm still breathing!");
        vTaskDelay(pdMS_TO_TICKS(10000));
    } */
}
#endif