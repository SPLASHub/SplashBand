#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"
#include "esp_log.h"

#include "gatt_srv.h"
#include "gps/nmea_parser.h"

/*
ano, mes, dia, hora, minuto, segundo
latitude, longitude, altitude, velocidade

*/
static const char *TAG = "NIMBLE_STACK";
static uint8_t ln_buffer[20]; // Buffer para armazenar os dados LN

// TODO : ter em conta o horario de verao 
void update_location_speed_data(const gps_t gps) // const para garantir que o valor não vai ser alterado
{
	ESP_LOGI(TAG, "update_location_speed_data()");
	int32_t lat = (int32_t)(gps.latitude  * 1e7);
	int32_t lon = (int32_t)(gps.longitude * 1e7);
	int16_t alt = (int16_t)(gps.altitude * 10);
	uint16_t spd = (uint16_t)(gps.speed);
	uint16_t year = gps.date.year + YEAR_BASE; 

    memset(ln_buffer, 0, sizeof(ln_buffer)); // Limpa o buffer

    ln_buffer[0] = LN_FLAG_LOCATION_PRESENT | LN_FLAG_ELEVATION_PRESENT | LN_FLAG_TIME_PRESENT | LN_FLAG_SPEED_PRESENT;
    memcpy(&ln_buffer[1], &lat, 4);
    memcpy(&ln_buffer[5], &lon, 4);
    memcpy(&ln_buffer[9], &alt, 2);
	memcpy(&ln_buffer[11], &spd, 2);
    ln_buffer[13] = (uint8_t)(year & 0xFF);
    ln_buffer[14] = (uint8_t)(year >> 8);
    ln_buffer[15] = gps.date.month;
    ln_buffer[16] = gps.date.day;
    ln_buffer[17] = gps.tim.hour + TIME_ZONE;
    ln_buffer[18] = gps.tim.minute;
    ln_buffer[19] = gps.tim.second;

	ble_gatts_chr_updated(attr_handle_location_speed);

	// Notifica os clientes conectados
	//notify_location_speed_data();
}

static int location_speed_access_cb(uint16_t conn_handle,uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg)
{
/* 	BLE_GATT_ACCESS_OP_READ_CHR
	BLE_GATT_ACCESS_OP_WRITE_CHR
	BLE_GATT_ACCESS_OP_READ_DSC
	BLE_GATT_ACCESS_OP_WRITE_DSC */
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
	
        int rc = os_mbuf_append(ctxt->om, ln_buffer, sizeof(ln_buffer));//Append to the response
        if (rc != 0)
		{
			ESP_LOGE(TAG, "Failed to append LN_buffer data");
			return BLE_ATT_ERR_INSUFFICIENT_RES;
		}
		return 0; // Sucesso
    }
    return BLE_ATT_ERR_UNLIKELY;
}


// Array de serviços GATT
static const struct ble_gatt_svc_def gatt_gnss_svcs[] = {
	/* GNSS service */
	{.type = BLE_GATT_SVC_TYPE_PRIMARY,
	 .uuid = BLE_UUID16_DECLARE(BLE_UUID_LOCATION_NAV_SVC),
	 .characteristics = (struct ble_gatt_chr_def[]){
		 {
			.uuid = BLE_UUID16_DECLARE(BLE_UUID_LOC_SPEED_CHR),
            .access_cb = location_speed_access_cb,
            .flags = BLE_GATT_CHR_F_READ, 
		 },
		 {0}, // Fim da lista
	 }},

	{
		0,
	}, /* No more services. */
};

/*
 * callback de registro de serviços/atributos GATT
 * - Função:
 * 	 - Logging/Depuração: Registrar em logs informações sobre os serviços, características e descritores que foram registrados no servidor GATT.
 * 	 - Monitoramento do Processo de Registro: Garantir que todos os componentes do GATT (serviços, características, descritores) foram registrados corretamente.
 * - Parâmetros:
 * 	 - ctxt: Contexto do registro (contém informações sobre a operação em curso).
 * 	 - arg:  Argumento genérico (não usado).
 * - Como Funciona:
 *   - Durante a Inicialização do BLE: Quando ble_gatts_add_svcs() é chamada para registrar serviços, a stack NimBLE percorre a lista de serviços, características e descritores definidos em gatt_svr_svcs[].
 *	 - Para Cada Componente Registrado: A função gatt_svr_register_cb é invocada com o contexto (ctxt) adequado, permitindo que o aplicativo saiba o que foi registrado e com quais handles.
 *	 - Uso dos Handles: Os handles (ex.: val_handle) são essenciais para operações futuras, como ler/escrever valores de características ou enviar notificações.
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
	char buf[BLE_UUID_STR_LEN];

	switch (ctxt->op)
	{
	case BLE_GATT_REGISTER_OP_SVC:
		MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
					ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
					ctxt->svc.handle);
		break;

	case BLE_GATT_REGISTER_OP_CHR:
		if (ble_uuid_cmp(ctxt->chr.chr_def->uuid, BLE_UUID16_DECLARE(BLE_UUID_LOC_SPEED_CHR)) == 0)
		{
			attr_handle_location_speed = ctxt->chr.val_handle; // atribui o handle da característica
		}
		MODLOG_DFLT(DEBUG, "registering characteristic %s with "
						   "def_handle=%d val_handle=%d\n",
					ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
					ctxt->chr.def_handle,
					ctxt->chr.val_handle);
		break;

	case BLE_GATT_REGISTER_OP_DSC:
		MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
					ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
					ctxt->dsc.handle);
		break;

	default:
		assert(0);
		break;
	}
}

int gatt_svr_init(void)
{
	int rc;
	/*
	 * GAP (Generic Access Profile - 0x1800)
	 * - Gerenciar a visibilidade e conectividade do dispositivo BLE.
	 *	 - Define o nome do dispositivo, aparência, modo de advertising.
	 *	 - Controla como outros dispositivos descobrem e se conectam ao seu ESP32.
	 * - Características:
	 * 	 - Device Name (0x2A00): Nome do dispositivo ("My_NimBLE_Device").
	 *	 - Appearance (0x2A01): Define ícone em apps (ex.: smartphone mostra um "sensor" ou "beacon").
	*/
	ble_svc_gap_init(); // <-- GAP habilitado aqui
	rc = ble_svc_gap_device_name_set("Pulseira"); // Define o nome do dispositivo anunciado (uuid 0x2A00)
	if (rc != 0)
	{
		return rc;
	}
	rc = ble_svc_gap_device_appearance_set(BLE_SVC_GAP_APPEARANCE_CATEGORY_GPS); // Define a aparência do dispositivo (uuid 0x2A01)
	if (rc != 0)
	{
		return rc;
	 }
	/*
	 * GATT (Generic Attribute Profile - 0x1801)
	 * - Estruturar os dados do dispositivo em serviços e características.
	 *	 - Gerencia a tabela de atributos (serviços, características, descritores).
	 *	 - Notifica clientes sobre mudanças nos serviços (ex.: novo serviço adicionado).
	 * - Características:
	 *	 - Service Changed (0x2A05): Indica alterações na tabela de serviços.
	 */
	ble_svc_gatt_init(); // <-- GATT habilitado aqui
	/*
	 * Serviço de Notificação de Alertas (ANS - Alert Notification Service - 0x1811)
	 * - Permitir que o dispositivo envie alertas (ex.: notificações de mensagens, alarmes, eventos de sensor) para um cliente (ex.: smartphone).
	 *	 - Notificar o usuário sobre eventos críticos (ex.: bateria fraca, falha de hardware).
	 *	 - Alertas personalizados (ex.: chegada de uma mensagem, atualização de status).
	 * - Características:
	 *	 - Supported New Alert Category (0x2A47)
	 *	 - New Alert (0x2A46)
	 *	 - Alert Notification Control Point (0x2A44)
	 */
	ble_svc_ans_init(); // <-- ANS (Serviço de Notificação de Alertas) habilitado aqui
	rc = ble_gatts_count_cfg(gatt_gnss_svcs);
	if (rc != 0)
	{
		return rc;
	}
	rc = ble_gatts_add_svcs(gatt_gnss_svcs);
	if (rc != 0)
	{
		return rc;
	}

	return 0;
}