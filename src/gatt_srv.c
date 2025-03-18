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

static const char *TAG = "NIMBLE_STACK";
// Buffers para armazenar os dados GPGGA e GPRMC
static char gpgga_data[100] = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
static char gprmc_data[100] = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";

// Serviço GNSS_Server (UUID 0x1136)
static const ble_uuid16_t gatt_svc_gnss_uuid = BLE_UUID16_INIT(GNSS_SERVER_SVC_UUID);
//static const ble_uuid128_t gatt_svc_gnss_uuid = BLE_UUID128_INIT(0x19, 0x18, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
//																 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

static const ble_uuid128_t gatt_chr_gpgga_uuid = BLE_UUID128_INIT(0x67, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
																  0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

static const ble_uuid128_t gatt_chr_gprmc_uuid = BLE_UUID128_INIT(0x68, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
																  0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

/**/
void update_gpgga_data(const char *data)
{
	strncpy(gpgga_data, data, sizeof(gpgga_data) - 1);
	gpgga_data[sizeof(gpgga_data) - 1] = '\0'; // Garante terminação nula
											   // Atualiza o valor da característica GPGGA
	ble_gatts_chr_updated(attr_handle_gpgga);

	// Notifica os clientes conectados
	//notify_gpgga_data();
}

void update_gprmc_data(const char *data)
{
	strncpy(gprmc_data, data, sizeof(gprmc_data) - 1);
	gprmc_data[sizeof(gprmc_data) - 1] = '\0'; // Garante terminação nula
											   // Atualiza o valor da característica GPRMC
	ble_gatts_chr_updated(attr_handle_gprmc);

	// Notifica os clientes conectados
	//notify_gprmc_data();
}
static int gatt_chr_gpgga_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	ESP_LOGI(TAG, "GPGGA Read: conn_handle=%d, attr_handle=%d", conn_handle, attr_handle);
	// Retorna o valor da característica GPGGA
	int rc = os_mbuf_append(ctxt->om, gpgga_data, strlen(gpgga_data));
	if (rc != 0)
	{
		ESP_LOGE(TAG, "Failed to append GPGGA data to mbuf");
		return BLE_ATT_ERR_INSUFFICIENT_RES;
	}
	return 0; // Sucesso
}
// Callback para leitura da característica GPRMC
static int gatt_chr_gprmc_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	ESP_LOGI(TAG, "GPRMC Read: conn_handle=%d, attr_handle=%d", conn_handle, attr_handle);
	// Retorna o valor da característica GPRMC
	int rc = os_mbuf_append(ctxt->om, gprmc_data, strlen(gprmc_data));
	if (rc != 0)
	{
		ESP_LOGE(TAG, "Failed to append GPRMC data to mbuf");
		return BLE_ATT_ERR_INSUFFICIENT_RES;
	}
	return 0; // Sucesso
}

// Array de serviços GATT
static const struct ble_gatt_svc_def gatt_gnss_svcs[] = {
	/* GNSS service */
	{.type = BLE_GATT_SVC_TYPE_PRIMARY,
	 .uuid = &gatt_svc_gnss_uuid.u,
	 .characteristics = (struct ble_gatt_chr_def[]){
		 {
			 /* GPGGA characteristic*/
			 .uuid = &gatt_chr_gpgga_uuid.u,
			 .access_cb = gatt_chr_gpgga_read_cb,
			 .flags = BLE_GATT_CHR_F_READ,
		 },
		 {
			 /* GPRMC characteristic*/
			 .uuid = &gatt_chr_gprmc_uuid.u,
			 .access_cb = gatt_chr_gprmc_read_cb,
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

	return 0;
}