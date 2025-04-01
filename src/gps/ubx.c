#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "ubx.h"

/**
 * @brief Verify the checksum of a NMEA message
 *
 * @param nmea  The NMEA message to verify
 *
 * @return true if the checksum is correct, false otherwise
 *
 * @example verify_checksum("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"); // true
 */
bool verify_checksum(uint8_t *data, size_t len)
{
	uint8_t ck_a = 0, ck_b = 0;
	for (int i = 2; i < len - 2; i++)
	{
		ck_a += data[i];
		ck_b += ck_a;
	}
	return (ck_a == data[len - 2] && ck_b == data[len - 1]);
}
bool is_ubx_packet(uint8_t *data, size_t len)
{
	if (len < 8)
		return false; // Tamanho mínimo de um pacote UBX
	if (data[0] != 0xB5 || data[1] != 0x62)
		return false; // Header inválido
	return true;
}
// TODO :  fazer a funcao ou uma funcao para identificar se o payload da ack corresponde a class e ao id da UBX_CFG
ubx_t CFG_acknowledged(uint8_t *data, size_t len)
{
	ubx_t code = INV_ID;
	if (!is_ubx_packet(data,len)){
		code = NOT_UBX_PACKAGE;
		return code;
	}
	if (!verify_checksum(data,len)){
		code = CHECKSUM_F;	
		return code;
	}
	if (data[2] != UBX_CLASS_ACK){
		code = INV_CLASS;
		return code;
	}
	if (data[3] == UBX_ID_ACK_ACK){
		code = ACK;
		return code;
	}
		return ACK;  
	if (data[3] == UBX_ID_ACK_NAK){
		code = NACK;
		return code;
	}
	return (ubx_t) INV_ID;
}
void calculate_checksum(UBXPackege *pckg)
{
	uint8_t ck_a = 0, ck_b = 0;
	// Soma ubxClass
	ck_a += pckg->ubxClass;
	ck_b += ck_a;
	// Soma ubxID
	ck_a += pckg->ubxID;
	ck_b += ck_a;
	// Soma length (LSB e depois MSB)
	ck_a += (uint8_t)(pckg->payload_length & 0xFF);
	ck_b += ck_a;
	ck_a += (uint8_t)((pckg->payload_length >> 8) & 0xFF);
	ck_b += ck_a;
	// Soma todos os bytes do payload
	for (int i = 0; i < pckg->payload_length; i++)
	{
		ck_a += pckg->payload[i];
		ck_b += ck_a;
	}

	pckg->ck_a = ck_a;
	pckg->ck_b = ck_b;
}

uint8_t *create_ubx_cmd(uint8_t ubxClass, uint8_t ubxID, uint8_t *payload, size_t payload_size)
{
	UBXPackege pckg = {
		.sync1 = 0xB5,
		.sync2 = 0x62,
		.ubxClass = ubxClass,
		.ubxID = ubxID,
		.payload_length = payload_size,
		.payload = (uint8_t *)malloc(payload_size)};
	if (pckg.payload == NULL)
	{
		printf("ERROR: Falha ao alocar memória para o payload");
		return NULL;
	}
	memcpy(pckg.payload, payload, payload_size);
	calculate_checksum(&pckg);

	// Aloca sendBuffer dinamicamente - precisa de ser assim porque senao o sendBuffer é destruido quando a função termina
	uint8_t *sendBuffer = (uint8_t *)malloc(8 + payload_size); // 8 = 2 bytes de sincronização + 2 bytes de classe e ID + 2 bytes de payload_length + 2 bytes de checksum
	if (sendBuffer == NULL)
	{
		free(pckg.payload);
		printf("ERROR: Falha ao alocar memória para o buffer de envio");
		return NULL;
	}
	memcpy(sendBuffer, &pckg, 6);	// Copia cabeçalho
	memcpy(sendBuffer + 6, pckg.payload, payload_size); // Copia payload - tem que ser assim porque se fosse  memcpy(sendBuffer, &msg, sizeof(msg)); apenas copiava o valor do ponteiro do payload em vez do payload
	sendBuffer[6 + payload_size] = pckg.ck_a;
	sendBuffer[7 + payload_size] = pckg.ck_b;

	free(pckg.payload);

	return sendBuffer;
}
/**
 * @brief Create a UBX-CFG-MSG message
 *
 * @param msgClass	Classe da mensagem NMEA
 * @param msgID		ID da mensagem NMEA
 * @param rate		Taxa de envio da mensagem, se 0 desativa a mensagem
 *
 * @return uint8_t  The message to be sent
 *
 * @example create_config_nmea_message(MSG_CLASS_NMEA_STANDARD, MSG_ID_GGA, 1); // envia a mensagem GGA a 1 Hz
 */
uint8_t *create_config_nmea_message(uint8_t msgClass, uint8_t msgID, uint8_t I2C_rate, uint8_t UART1_rate, uint8_t UART2_rate,uint8_t USB_rate,uint8_t SPI_rate,uint8_t Reserved, size_t *payload_size)
{
	uint8_t payload[8] = {msgClass, msgID, I2C_rate, UART1_rate, UART2_rate, USB_rate, SPI_rate, Reserved};

	*payload_size  = sizeof(payload) + 8;
	return create_ubx_cmd(UBX_CLASS_CFG, UBX_ID_CFG_MSG, payload, sizeof(payload));
}
/**
 * @brief Create a UBX-CFG-RATE message
 *
 * @param MeasRate 	Intervalo em milissegundos de cada ciclo de navegação. Por exemplo, se quisermos 5 Hz, é 200 ms.
 * @param NavRate 	Quantos ciclos “measRate” devem ocorrer antes de calcular 1 solução de navegação final. Geralmente deixamos em 1 para obter uma solução por ciclo. Se navRate = 2, então a cada 2 ciclos de medição o receptor produz 1 “Navigation Solution” efetiva.
 * @param TimeRef 	Time reference 0 = UTC time, 1 =GPS time
 *
 * @return uint8_t  The message to be sent
 *
 * @example create_config_rate_message(200, 1, 0); // 5 Hz, 1 ciclo, tempo UTC
 */
uint8_t *create_config_rate_message(uint16_t measRateMs, uint16_t navRate, uint16_t timeRef, size_t *payload_size)
{
	uint8_t payload[6];

	payload[0] = measRateMs & 0xFF;
	payload[1] = (measRateMs >> 8) & 0xFF;
	payload[2] = navRate & 0xFF;
	payload[3] = (navRate >> 8) & 0xFF;
	payload[4] = timeRef & 0xFF;
	payload[5] = (timeRef >> 8) & 0xFF;

	*payload_size = sizeof(payload) + 8;
	return create_ubx_cmd(UBX_CLASS_CFG, UBX_ID_CFG_RATE, payload, sizeof(payload));
}