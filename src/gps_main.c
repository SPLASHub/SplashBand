/*
	//+:components/hal/include/hal/uart_types.h
	//	contem definicoes de uarts
	//+:components/hal/include/hal/gpio_types.h
*/

/* NMEA no neo-6m:
//+:$GPGGA - Global Positioning System Fix Data (Class=0xF0, ID=0x00)(tratada)
	ex: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	campos:
		$GPGGA - Identificador.
		123519 - Hora UTC (hhmmss).
		4807.038 - Latitude (48°07.038').
		N - Hemisfério Norte.
		01131.000 - Longitude (11°31.000').
		E - Hemisfério Leste.
		1 - Qualidade do sinal (0 = sem fix, 1 = fix GPS, 2 = fix DGPS).
		08 - Número de satélites usados.
		0.9 - Precisão Horizontal (HDOP).
		545.4 - Altitude acima do nível do mar (metros).
		M - Unidade da altitude (metros).
		46.9 - Altura do geoide acima do elipsoide WGS84.
		M - Unidade da altura do geoide.
		(vazio) - Tempo de correção diferencial (DGPS).
		(vazio) - ID da estação DGPS.
		*47 - Checksum.
//+:$GPRMC - Recommended Minimum Specific GPS/Transit Data (Class=0xF0, ID=0x04) (tratada)
	ex: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
	campos:
		$GPRMC - Identificador.
		123519 - Hora UTC (hhmmss).
		A - Status (A = válido, V = inválido).
		4807.038 - Latitude.
		N - Hemisfério Norte.
		01131.000 - Longitude.
		E - Hemisfério Leste.
		022.4 - Velocidade sobre o solo (nós).
		084.4 - Rumo sobre o solo (graus).
		230394 - Data (ddmmaa).
		003.1 - Variação magnética (graus).
		W - Direção da variação magnética (Oeste).
		*6A - Checksum.
//+:$GPGSV - Satellites in View (Class=0xF0, ID=0x03)
	ex: $GPGSV,3,1,12,02,45,120,40,04,70,310,38,07,22,245,42,11,50,180,45*76
	campos:
		$GPGSV - Identificador.
		3 - Número total de sentenças GSV.
		1 - Número desta sentença (1 de 3).
		12 - Total de satélites visíveis.
		02 - PRN do satélite.
		45 - Elevação (graus).
		120 - Azimute (graus).
		40 - SNR (dB).
		(Repetido para cada satélite).
		*76 - Checksum.
//+:$GPGLL - Geographic Position - Latitude/Longitude (Class=0xF0, ID=0x01)
	ex: $GPGLL,4916.45,N,12311.12,W,225444,A*1D
	campos:
		$GPGLL - Identificador.
		4916.45 - Latitude.
		N - Hemisfério Norte.
		12311.12 - Longitude.
		W - Hemisfério Oeste.
		225444 - Hora UTC (hhmmss).
		A - Status (A = válido, V = inválido).
		*1D - Checksum.
//+:$GPVTG - Track Made Good and Ground Speed (Class=0xF0, ID=0x05)
	ex: $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
	campos:
		$GPVTG - Identificador.
		054.7 - Rumo em relação ao norte verdadeiro (graus).
		T - Indicador de norte verdadeiro.
		034.4 - Rumo em relação ao norte magnético (graus).
		M - Indicador de norte magnético.
		005.5 - Velocidade em nós.
		N - Unidade (nós).
		010.2 - Velocidade em km/h.
		K - Unidade (km/h).
		*48 - Checksum.
//+:$GPGSA - GNSS DOP and Active Satellites  (Class=0xF0, ID=0x02)
	ex: $GPGSA,A,3,04,05,09,12,24,29,30,,,,,,1.8,1.0,1.5*33
	campos:
		$GPGSA - Identificador.
		A - Modo automático (A = automático, M = manual).
		3 - Tipo de fix (1 = sem fix, 2 = 2D, 3 = 3D).
		04,05,09,... - PRNs dos satélites usados.
		(vazio) - Satélites não utilizados.
		1.8 - PDOP (Dilução da precisão da posição).
		1.0 - HDOP (Dilução horizontal).
		1.5 - VDOP (Dilução vertical).
		*33 - Checksum.
//+:$GPTXT - Text Transmission (Class=0xF0, ID=0x41)
	ex: $GPTXT,01,01,02,u-blox ag - www.u-blox.com*50
	campos:
		$GPTXT - Identificador.
		01 - Número total de sentenças TXT.
		01 - Número desta sentença (1 de 1).
		02 - Número de mensagens TXT.
		u-blox ag - www.u-blox.com - Texto.
		*50 - Checksum.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gps_main.h"
#include "gatt_srv.h"

typedef struct
{
	uint8_t msgClass;
	uint8_t msgID;
	uint8_t rate;
} NMEA_Setup_t;
static const NMEA_Setup_t nmeaConfigs[] = // existem mais mostradas na documentacao
{
		{0xF0, 0x00, 1}, // GGA 
		{0xF0, 0x01, 0}, // GLL
		{0xF0, 0x02, 0}, // GSA
		{0xF0, 0x03, 0}, // GSV
		{0xF0, 0x04, 0}, // RMC
		{0xF0, 0x05, 0}	 // VTG
						 // TXT
};

static const char *TAG = "GPS";

#define GPS_TASK_STACK_SIZE 4096
#define GPS_TASK_CORE 1
#define GPS_TASK_PRIO 5

// fila apenas otimiza a notificação de dados recebidos, não o processamento das mensagens
// queue armazena eventos, não os dados em si.
QueueHandle_t uart_queue;
//	Problema : Pacotes UART podem chegar fragmentados(ex : $GPGGA, ... em um pacote e *CS\r\n em outro).
//	Solução : O gps_buffer acumula dados entre chamadas de GPS_ReadData para montar mensagens completas.static int gps_buffer_pos = 0;
// para optimizar usar um buffer circular em vez de gps_buffer

/**
 * @brief Initialize the UART for GPS communication
 *
 * @param baud_rate: Baud rate for the UART communication
 * @param tx_pin: GPIO pin number for UART TX
 * @param rx_pin: GPIO pin number for UART RX
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_* code on failure
 */
esp_err_t GPS_Init(uint32_t baud_rate, int tx_pin, int rx_pin)
{
	ESP_LOGI(GPS_TAG, "Initializing GPS UART with baud rate %" PRIu32 ", TX pin %d, RX pin %d", baud_rate, tx_pin, rx_pin);

	// Configuracao dos parametros UART
	const uart_config_t uart_config = {
		.baud_rate = baud_rate,				  /*!< UART baud rate*/
		.data_bits = UART_DATA_8_BITS,		  /*!< UART byte size*/
		.parity = UART_PARITY_DISABLE,		  /*!< UART parity mode*/
		.stop_bits = UART_STOP_BITS_1,		  /*!< UART stop bits*/
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE /*!< UART HW flow control mode (cts/rts)*/
											  //.rx_flow_ctrl_thresh = 					/*!< UART HW RTS threshold*/
	};

	esp_err_t ret = uart_param_config(UART_NUM, &uart_config);
	if (ret != ESP_OK)
	{
		ESP_LOGE(GPS_TAG, "UART parameter configuration failed: %s", esp_err_to_name(ret));
		return ESP_ERR_GPS_UART_PARAM_CONFIG_FAILED;
	}

	// Configuracao dos pinos UART
	ret = uart_set_pin(UART_NUM,
					   tx_pin,
					   rx_pin,
					   UART_PIN_NO_CHANGE,
					   UART_PIN_NO_CHANGE);
	if (ret != ESP_OK)
	{
		ESP_LOGE(GPS_TAG, "UART set pin failed: %s", esp_err_to_name(ret));
		return ESP_ERR_GPS_UART_SET_PIN_FAILED;
	}

	// Instalacao do driver UART
	ret = uart_driver_install(UART_NUM,
							  UART_RX_BUFFER_SIZE,
							  UART_TX_BUFFER_SIZE,
							  10,		   // Tamanho da fila de eventos
							  &uart_queue, // Ponteiro para a fila
							  0);		   // Flags
	if (ret != ESP_OK)
	{
		ESP_LOGE(GPS_TAG, "UART driver installation failed: %s", esp_err_to_name(ret));
		return ESP_ERR_GPS_UART_DRIVER_INSTALL_FAILED;
	}

	ESP_LOGI(GPS_TAG, "GPS UART initialized successfully");
	return ESP_OK;
}
void system_initialization_task()
{
	// processor setting to 240 Mhz.
	esp_pm_config_t pm_config = {
		// before esp_pm_config_esp32_t
		.max_freq_mhz = 240,
		.min_freq_mhz = 240,
	};
	esp_pm_configure(&pm_config);
	ESP_LOGI("INIT_TASK", "Processor configured to 240 Mhz");

	esp_err_t ret;

	// GPS Initialization
	ret = GPS_Init(9600, CONFIG_UART_TX_GPS, CONFIG_UART_RX_GPS);
	if (ret == ESP_OK)
	{
		ESP_LOGI("INIT_TASK", "UART GPS initialized successfully");
	}
	else
	{
		ESP_LOGE("INIT_TASK", "GPS UART initialization failed: %s", esp_err_to_name(ret));
	}

	// UART_DATA
	//  Define o limite de bytes (threshold) para acionar o evento UART_DATA
	//  uart_set_rx_full_threshold(UART_NUM_1, 20);
	//    - da jeito se as mensagens tiverem um tamanho fixo
	//  Configura o timeout de recepção (em número de bits) para acionar o evento UART_DATA
	//  	- da jeito pq o gps demora a responder entre mensagens, preciso é calcular esse tempo
	// uart_set_rx_timeout(UART_NUM_1, 2); 			// Timeout de 2 bits (~2 byte times)
	// UART_PATTERN_DET
	/*uart_enable_pattern_det_baud_intr(UART_NUM_1,
									  '\n',	 		// Caractere de padrão.
									  1,	 		// Quantidade de vezes que o caractere precisa aparecer.
									  10000,		 // Tempo limite de espera do padrão (em bits).
									  10,	 		// Tamanho do espaço entre padrões.
									  10);	 		// Tempo máximo entre padrões.
	uart_pattern_queue_reset(UART_NUM_1,
							 20); 					// Max queue length for the detected pattern.
	*/

	// Inicializar buffer circular
	/*gps_circ_buf = circular_buffer_init(UART_BUFFER_SIZE);
	if (!gps_circ_buf)
	{
		ESP_LOGE("INIT_TASK", "Falha ao criar buffer circular");
		return;
	}
	*/
}
/**
 * @brief Internal function to parse $GPGGA NMEA frames and extract data
 *
 * @param nmea: NMEA string containing the $GPGGA frame
 * @param gps_data: Pointer to GPSData_t structure to store extracted data
 *
 * @return
 *      - true if parsing is successful, false otherwise
 */
static bool parse_gpgga(const char *nmea, GPSData_t *gps_data)
{
	ESP_LOGI(GPS_TAG, "Parsing $GPGGA frame");
	float lat, lon;
	char ns, ew;
	int quality, satellites;
	float altitude;

	int parsed = sscanf(nmea, "$GPGGA,%*f,%f,%c,%f,%c,%d,%d,%*f,%f",
						&lat, &ns, &lon, &ew, &quality, &satellites, &altitude);

	if (parsed < 7)
	{
		ESP_LOGE(GPS_TAG, "Failed to parse $GPGGA frame");
		return false;
	}

	// Coordinates conversion
	float lat_deg = (int)(lat / 100);
	float lat_min = lat - (lat_deg * 100);
	gps_data->latitude = lat_deg + lat_min / 60.0f;
	if (ns == 'S')
		gps_data->latitude = -gps_data->latitude;

	float lon_deg = (int)(lon / 100);
	float lon_min = lon - (lon_deg * 100);
	gps_data->longitude = lon_deg + lon_min / 60.0f;
	if (ew == 'W')
		gps_data->longitude = -gps_data->longitude;

	gps_data->altitude = altitude;

	// ESP_LOGI(GPS_TAG, "Parsed $GPGGA: Lat: %.6f, Lon: %.6f, Altitude: %.2f",
	//         gps_data->latitude, gps_data->longitude, gps_data->altitude);

	return true;
}
/**
 * @brief Internal function to parse $GPRMC NMEA frames and extract data
 *
 * @param nmea: NMEA string containing the $GPRMC frame
 * @param gps_data: Pointer to GPSData_t structure to store extracted data
 *
 * @return
 *      - true if parsing is successful, false otherwise
 */
static bool parse_gprmc(const char *nmea, GPSData_t *gps_data)
{
	ESP_LOGI(GPS_TAG, "Parsing $GPRMC frame");

	// Copy NMEA frame for manipulation
	char nmea_copy[100];
	strncpy(nmea_copy, nmea, sizeof(nmea_copy) - 1);
	nmea_copy[sizeof(nmea_copy) - 1] = '\0'; // Make sure the chain is complete

	char *token;
	char *rest = nmea_copy;
	int field_index = 0;

	// Reset GPS data
	gps_data->valid = false;

	while ((token = strtok_r(rest, ",", &rest)))
	{
		field_index++;
		switch (field_index)
		{
		case 2: // Hour (hhmmss.ss)
			if (strlen(token) >= 6)
			{
				char hour_str[3] = {token[0], token[1], '\0'};
				char min_str[3] = {token[2], token[3], '\0'};
				char sec_str[3] = {token[4], token[5], '\0'};
				gps_data->hour = atoi(hour_str);
				gps_data->minute = atoi(min_str);
				gps_data->second = atoi(sec_str);
			}
			else
			{
				ESP_LOGE(GPS_TAG, "Invalid time format in $GPRMC sentence");
				return false;
			}
			break;
		case 3: // Statut (A=Activ, V=Void)
			gps_data->valid = (token[0] == 'A');
			if (!gps_data->valid)
			{
				ESP_LOGE(GPS_TAG, "GPRMC status is invalid");
				return false;
			}
			break;
		case 4: // Latitude
			if (strlen(token) > 0)
			{
				gps_data->latitude = atof(token);
			}
			else
			{
				ESP_LOGE(GPS_TAG, "Invalid latitude data in $GPRMC sentence");
				return false;
			}
			break;
		case 5: // N/S
			if (token[0] == 'S')
				gps_data->latitude = -gps_data->latitude;
			break;
		case 6: // Longitude
			if (strlen(token) > 0)
			{
				gps_data->longitude = atof(token);
			}
			else
			{
				ESP_LOGE(GPS_TAG, "Invalid longitude data in $GPRMC sentence");
				return false;
			}
			break;
		case 7: // E/W
			if (token[0] == 'W')
				gps_data->longitude = -gps_data->longitude;
			break;
		case 8: // Sprint (knots)
			gps_data->speed = atof(token);
			break;
		case 9: // Date (ddmmyy)
			if (strlen(token) == 6)
			{
				char day_str[3] = {token[0], token[1], '\0'};
				char month_str[3] = {token[2], token[3], '\0'};
				char year_str[3] = {token[4], token[5], '\0'};
				gps_data->day = atoi(day_str);
				gps_data->month = atoi(month_str);
				gps_data->year = 2000 + atoi(year_str); // Add 2000 to get the full year
			}
			else
			{
				ESP_LOGE(GPS_TAG, "Invalid date format in $GPRMC sentence");
				return false;
			}
			break;
		default:
			break;
		}
	}

	// Validation et conversion finale
	if (gps_data->valid)
	{
		float lat_deg = (int)(gps_data->latitude / 100);
		float lat_min = gps_data->latitude - (lat_deg * 100);
		gps_data->latitude = lat_deg + lat_min / 60.0f;

		float lon_deg = (int)(gps_data->longitude / 100);
		float lon_min = gps_data->longitude - (lon_deg * 100);
		gps_data->longitude = lon_deg + lon_min / 60.0f;

		// ESP_LOGI(GPS_TAG, "Parsed $GPRMC: Lat: %.6f, Lon: %.6f, Speed: %.2f, Date: %02d/%02d/%04d, Time: %02d:%02d:%02d",
		//          gps_data->latitude, gps_data->longitude, gps_data->speed,
		//          gps_data->day, gps_data->month, gps_data->year,
		//          gps_data->hour, gps_data->minute, gps_data->second);
	}
	else
	{
		ESP_LOGE(GPS_TAG, "Parsed data is invalid");
	}

	return gps_data->valid;
}
/**
 * @brief Verify the checksum of an NMEA string
 *
 * @param nmea: NMEA string to verify
 *
 * @return
 *      - true if checksum matches, false otherwise
 */
static bool verify_nmea_checksum(const char *nmea)
{
	const char *checksum_position = strchr(nmea, '*');
	if (checksum_position == NULL)
	{
		ESP_LOGE(GPS_TAG, "Invalid NMEA frame: No checksum found");
		return false;
	}

	uint8_t checksum = 0;
	for (const char *p = nmea + 1; p < checksum_position; ++p)
	{
		checksum ^= *p;
	}

	uint8_t provided_checksum = (uint8_t)strtol(checksum_position + 1, NULL, 16);
	if (checksum == provided_checksum)
	{
		return true;
	}
	else
	{
		ESP_LOGE(GPS_TAG, "NMEA checksum mismatch: Calculated: 0x%X, Provided: 0x%X", checksum, provided_checksum);
		return false;
	}
}
void print_gps_buffer_escaped(int pos, const char *buffer, int size)
{
	// Exemplo usando ESP_LOGI, mas pode ser printf se preferir
	ESP_LOGI(GPS_TAG, "Buffer (%d/%d bytes):", pos, size);
	int x00count = 0;
	for (int i = 0; i < size; i++)
	{
		if (x00count >= 4)
		{
			printf("...");
			break;
		}
		unsigned char c = (unsigned char)buffer[i];
		if (c >= 32 && c < 127)
		{
			// Caractere ASCII “imprimível”
			printf("%c", c);
			x00count = 0;
		}
		else
		{
			// Caractere especial, imprime como \xNN
			printf("\\x%02X", c);
			if (c == 0x00)
			{
				x00count++;
			}
		}
	}
	printf("\n");
}

/**
 * @brief Read and process NMEA frames from GPS module
 *
 * @param gps_data: Pointer to GPSData_t structure to store parsed data
 *
 * @return
 *      - true if valid GPS data is read, false otherwise
 */
bool GPS_ReadData(GPSData_t *gps_data)
{
	bool data_processed = false;
	gps_data->valid = false;
	uart_event_t event;
	while (xQueueReceive(uart_queue, &event, portMAX_DELAY))
	{
		switch (event.type)
		{
		case UART_DATA:
			ESP_LOGE(GPS_TAG, "UART_DATA");
			uint8_t *data = (uint8_t *)malloc(event.size);
			if (data == NULL)
			{
				ESP_LOGE(GPS_TAG, "Failed to allocate memory for data");
			}
			int len = uart_read_bytes(UART_NUM, data, event.size, pdMS_TO_TICKS(100));
			if (len > 0)
			{
				update_gpgga_data((const char *)data);
				gps_data->valid = true;
				ESP_LOGI(GPS_TAG, "Dados recebidos no fifo  (%d bytes): %.*s", len, len, data);
			}
			else
			{
				ESP_LOGW(GPS_TAG, "No valid GPS data received");
			}
			free(data); // Liberar a memória alocada
			break;
		case UART_FIFO_OVF:
			ESP_LOGE(GPS_TAG, "FIFO Overflow! Flushing buffer");
			uart_flush_input(UART_NUM);
			break;
		case UART_BUFFER_FULL:
			ESP_LOGE(GPS_TAG, "Buffer RX Overflow! Flushing buffer");
			uart_flush_input(UART_NUM);
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
		default:
			ESP_LOGI(TAG, "uart event type: %d", event.type);
			break;
		}
	}
	return data_processed;
}

/**
 * @brief GPS Task for independent GPS processing
 */
void gps_task(void *param)
{
	GPSData_t gps_data;

	while (1)
	{
		if (GPS_ReadData(&gps_data))
		{
			if (gps_data.valid)
			{
				ESP_LOGI(GPS_TAG, "Valid GPS data received.");
			}
			else
			{
				// xEventGroupClearBits(init_event_group, SYSTEM_STATE_GPS_AVAILABLE);
				ESP_LOGI(GPS_TAG, "Invalid GPS data.");
			}
		}
		else
		{
			// xEventGroupClearBits(init_event_group, SYSTEM_STATE_GPS_AVAILABLE);
			ESP_LOGI(GPS_TAG, "No GPS data received.");
		}
		// 1ms delay para garantir que o IDLE task roda
		ESP_LOGI(GPS_TAG, "Delay 1ms");
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

/**
 * @brief Configures the refresh rate of the GPS module
 * @param rate_hz: Desired refresh rate in Hertz (1 to 5 Hz for NEO-M6)
 *
 * @return
 *      - ESP_OK if the configuration is successful
 *      - ESP_ERR_INVALID_ARG if the rate is out of range
 */
esp_err_t GPS_SetUpRate(uint8_t rate_hz)
{
	if (rate_hz < 1 || rate_hz > 5)
	{
		ESP_LOGE(GPS_TAG, "Invalid refresh rate: %d Hz", rate_hz);
		return ESP_ERR_INVALID_ARG;
	}

	// The refresh rate is defined by the interval in milliseconds between each frame
	uint16_t interval_ms = 1000 / rate_hz;

	// UBX-CFG-RATE command to configure the refresh rate
	uint8_t ubx_cmd[] = {
		0xB5, 0x62, // UBX Header - marca comeco de mensagem ubx
		0x06, 0x08, // class and ID (CFG-RATE) - define o tipo de mensagem - Class=0x06 e ID=0x08 significa “configure a taxa de saída do GPS”.
		0x06, 0x00, // Payload size (6 octets) - define o tamanho do conteudo da mensagem (daqui para baixo excluindo checksum)
		0x00, 0x00, // MeasRate (measurement interval in ms)
		0x01, 0x00, // NavRate (navigation rate, fixed at 1)
		0x01, 0x00, // TimeRef (0=UTC, 1=GPS)
		0x00, 0x00	// Checksum (CK_A, CK_B)
	};

	// Insert measurement interval
	ubx_cmd[6] = interval_ms & 0xFF;
	ubx_cmd[7] = (interval_ms >> 8) & 0xFF;

	// Checksum calculation
	uint8_t ck_a = 0, ck_b = 0;
	for (int i = 2; i < 12; i++)
	{
		ck_a += ubx_cmd[i];
		ck_b += ck_a;
	}
	ubx_cmd[12] = ck_a;
	ubx_cmd[13] = ck_b;

	// Send command to GPS module
	int len = uart_write_bytes(UART_NUM, (const char *)ubx_cmd, sizeof(ubx_cmd));
	if (len != sizeof(ubx_cmd))
	{
		ESP_LOGE(GPS_TAG, "Failed to send UBX-CFG-RATE command");
		return ESP_FAIL;
	}

	ESP_LOGI(GPS_TAG, "Refresh rate set to %d Hz", rate_hz);
	return ESP_OK;
}
/**
 * @brief Configures one NMEA message and its refresh rate
 * @param MsgClass: Class of the NMEA message (ex: 0xF0 for GGA)
 * @param MsgID: ID of the NMEA message (ex: 0x00 for GGA)
 * @param rate: Desired refresh rate in Hertz (1 to 5 Hz for NEO-M6)
 *
 * @return
 *      - ESP_OK if the configuration is successful
 *      - ESP_ERR_INVALID_ARG if the rate is out of range
 */
esp_err_t GPS_SetUpNMEA(uint8_t MsgClass, uint8_t MsgID, uint8_t rate)
{
	uint8_t ubx_cmd[] = {
		0xB5, 0x62, // UBX Header - marca comeco de mensagem ubx
		0x06, 0x01, // class and ID (CFG-MSG) - define o tipo de mensagem
		0x03, 0x00, // Payload size (3 octets) - define o tamanho do conteudo da mensagem (daqui para baixo excluindo checksum)
		0x00, 0x00, // MsgClass, MsgID 6
		0x00,		// rate
		0x00, 0x00	// Checksum (CK_A, CK_B)
	};
	// Insert class and ID
	ubx_cmd[6] = MsgClass;
	ubx_cmd[7] = MsgID;
	// Insert rate
	ubx_cmd[8] = rate;
	// Checksum calculation
	uint8_t ck_a = 0, ck_b = 0;
	for (int i = 2; i < 9; i++)
	{
		ck_a += ubx_cmd[i];
		ck_b += ck_a;
	}
	ubx_cmd[9] = ck_a;
	ubx_cmd[10] = ck_b;
	// Send command to GPS module
	int len = uart_write_bytes(UART_NUM, (const char *)ubx_cmd, sizeof(ubx_cmd)); // bloqueia (se necessário) até conseguir copiar todos os bytes para o buffer interno de transmissão.
																				  // uart_write_bytes_async() // tenta enfileirar os bytes de forma não bloqueante, retornando imediatamente quantos bytes efetivamente foram aceitos no buffer.
	if (len != sizeof(ubx_cmd))
	{
		ESP_LOGE(GPS_TAG, "Failed to send UBX-CFG-MSG command");
		return ESP_FAIL;
	}
	return ESP_OK;
}
void GPS_ConfigureNMEA(void)
{
	int count = sizeof(nmeaConfigs) / sizeof(nmeaConfigs[0]);

	for (int i = 0; i < count; i++)
	{
		esp_err_t err = GPS_SetUpNMEA(
			nmeaConfigs[i].msgClass,
			nmeaConfigs[i].msgID,
			nmeaConfigs[i].rate);
		if (err == ESP_OK)
		{
			ESP_LOGI(GPS_TAG, "NMEA %d - configurada com rate=%u",
					 i, nmeaConfigs[i].rate);
		}
		else
		{
			ESP_LOGE(GPS_TAG, "Falha ao configurar NMEA %d (cls=0x%02X, id=0x%02X)",
					 i, nmeaConfigs[i].msgClass, nmeaConfigs[i].msgID);
		}

		// Pequeno delay entre envios (opcional, mas às vezes útil)
		// vTaskDelay(pdMS_TO_TICKS(50));
	}
}
void app_gps_main(void)
{
	system_initialization_task();
	GPS_SetUpRate(1);
	GPS_ConfigureNMEA();

	// xTaskCreate(&gps_task, "gps_task", 4096, NULL, 5, NULL);
	xTaskCreatePinnedToCore(
		gps_task,			 // Função da tarefa
		"gps_task",			 // Nome da tarefa
		GPS_TASK_STACK_SIZE, // Tamanho do stack
		NULL,				 // Parâmetros
		GPS_TASK_PRIO,		 // Prioridade
		NULL,				 // Handle da tarefa
		GPS_TASK_CORE		 // Núcleo (CPU1)
	);
}