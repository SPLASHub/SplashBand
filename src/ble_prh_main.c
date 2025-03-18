/*CENAS
em esp-nimble:

	//+:nimble/host/services/gap/include/services/gap/ble_svc_gap.h 	//#include (.c)"sysinit/sysinit.h", "host/ble_hs.h","os/endian.h",(.h)"syscfg/syscfg.h","host/ble_ead.h"
	//	Serviços GAP	Configurar nome, aparência, advertising básico
	// 	serviços GAP pré-configurados e funções de alto nível para configurar parâmetros básicos do dispositivo BLE, como nome, aparência e modo de advertising.
	//	Funcoes: 	ble_svc_gap_device_name_set(),ble_svc_gap_device_appearance_set(),ble_svc_gap_adv_start()
	//	Nno misturar funções de ble_svc_gap.h e ble_gap.h para a mesma operação

	//+:nimble/host/include/host/ble_esp_gap.h 							//#include ""
	//	Hardware (ESP32)	Inicializar controlador BLE do ESP32
	// 	arquivo específico da Espressif que estende/adapta a implementação padrão do NimBLE para funcionar no hardware ESP32.
	// 	contém funções e definições específicas do ESP-IDF, como inicialização do controlador BLE e integração com recursos do SoC.
	// 	Funcoes: 	esp_nimble_hci_init()

	//+:nimble/host/include/host/ble_gap.h 								//#include "host/ble_hs.h","host/ble_hs_adv.h","syscfg/syscfg.h","host/ble_esp_gap.h"
	//	Core NimBLE	Controle avançado de conexões e segurança
	// 	cabeçalho principal da NimBLE para operações GAP de baixo nível.
	//	define estruturas, callbacks e funções para controle completo do GAP, incluindo advertising, conexões, e segurança.
	// 	Funcoes: 	ble_gap_adv_set_fields(),ble_gap_connect()
	//	(ble_svc_gap.h, ble_esp_gap.h, ble_gap.h) dependem de ble_hs.h para funcionar, pois ele é o núcleo da stack.

	//+:nimble/host/services/gatt/include/services/gatt/ble_svc_gatt.h	//#include (.c)"sysinit/sysinit.h", "host/ble_hs.h",(.h)"syscfg/syscfg.h"

	//+:nimble/host/include/host/ble_esp_gatt.h							//#include ""

	//+:nimble/host/include/host/ble_gatt.h								//#include "nimble/ble.h","host/ble_att.h","host/ble_uuid.h","host/ble_esp_gatt.h","syscfg/syscfg.h"

	//+:nimble/host/include/host/ble_hs.h
	//	Core NimBLE	Configuração e operação do host BLE - cabeçalho principal da NimBLE para configuração e operação do host BLE.
	// 	Fornece:	APIs de baixo nível para operações do host BLE (ex.: conexões, segurança, advertising).Gerenciamento de estados (ex.: conexões ativas, serviços registrados).Callbacks para eventos BLE (ex.: conexão, desconexão, notificações).Integração com o controlador BLE (via HCI).


	//+:nimble/include/nimble/nimble_npl.h								//
	// 	NPL - NimBLE Porting Layer

	//+:nimble/include/nimble/nimble_port.h 							// #include "nimble/nimble_npl.h"
	// 	Porting Layer

	//+:porting/npl/freertos/include/nimble/npl_freertos.h				// #include ""
	//

	//+:porting/npl/freertos/include/nimble/nimble_port_freertos.h		// #include "freertos/FreeRTOS.h","freertos/queue.h","freertos/semphr.h","freertos/task.h","freertos/timers.h","esp_timer.h"
	//

	//+:porting/npl/freertos/include/nimble/nimble_npl_os.h 			// #include "nimble/nimble_npl.h" ,"esp_err.h"
	//

em esp-idf:

	//+:components/bt/host/nimble/esp-hci/include/esp_nimble_hci.h  	// #include "nimble/transport.h"
	//+:components/bt/host/nimble/esp-hci/src/esp_nimble_hci.c 			// #include "sysinit/sysinit.h","nimble/hci_common.h","host/ble_hs.h","nimble/nimble_port.h","nimble/nimble_port_freertos.h","esp_nimble_hci.h","esp_nimble_mem.h","bt_osi_mem.h","esp_bt.h","freertos/semphr.h","esp_compiler.h","soc/soc_caps.h","bt_common.h","hci_log/bt_hci_log.h"
	//

	//+:components/bt/host/nimble/port/include/esp_nimble_cfg.h 		// #include "sdkconfig.h"
	//

	//+:components/bt/host/nimble/port/include/esp_nimble_mem.h   		// #include "bt_osi_mem.h"
	//

	//+:components/bt/include/esp32/include/esp_bt.h					//#include "esp_err.h","sdkconfig.h","esp_task.h","esp_assert.h"

	//+:components/bt/host/nimble/Kconfig.in
	//


	//+:components/nvs_flash/include/nvs_flash.h
*/

#include <stdio.h>
#include <assert.h>
#include "nvs_flash.h"					 // Para inicializar a NVS (armazenamento não volátil)
/*
- Gerencia a comunicação HCI (Host Controller Interface) entre a stack NimBLE (rodando como host no software) e o controlador Bluetooth interno do SoC da Espressif.
- Inicializa e configura a interface HCI e os eventos de interrupção/ISR necessários.
- Encaminha pacotes e eventos recebidos do controlador para a stack NimBLE, e faz o caminho inverso para enviar pacotes do host ao controlador.
- components/bt/host/nimble/esp-hci/src/esp_nimble_hci.c
*/
#include "esp_nimble_hci.h"
/*
porting/nimble/include/nimble/nimble_port.h
*/
#include "nimble/nimble_port.h"			 // Inicialização do stack NimBLE (camada host)
/*
- nimble_port_freertos_init()
porting/npl/freertos/include/nimble/nimble_port_freertos.h
*/
#include "nimble/nimble_port_freertos.h" // Integração com FreeRTOS: cria tarefa para o stack NimBLE
#include "host/ble_hs.h"				 // Configuração e operação do host BLE (NimBLE)
#include "services/gap/ble_svc_gap.h"	 // Serviços GAP (ex.: nome do dispositivo)
#include "services/gatt/ble_svc_gatt.h"	 // Serviços GATT (para registro de serviços)
#include "esp_log.h"					 // Sistema de logging do ESP-IDF

#include "host/util/util.h"				 // Funções utilitárias do NimBLE
#include "os/os_mbuf.h"					 // Manipulação de buffers mbuf
#include "host/ble_att.h"				 // Definições de atributos GATT
#include "console/console.h"			 // Funções de console

#include "gatt_srv.h"
#include "ble_prh_main.h"

#define NIMBLE_HS_STACK_SIZE CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE 	// default - valor definido em sdkconfig
#define NIMBLE_CORE 0												// default
#define NIMBLE_HOST_TASK_PRIO (configMAX_PRIORITIES - 4) 			// default

static const char *TAG = "NIMBLE_STACK";
static uint8_t own_addr_type;
static int ble_gap_event(struct ble_gap_event *event, void *arg);
void ble_store_config_init(void);

static void
ble_advertise(void)
{
	int rc;
	/**
	 *  GAP - device name definition
	 *  Set the advertisement data included in our advertisements:
	 *     o Flags (indicates advertisement type and other general info).
	 *     o Advertising tx power.
	 *     o Device name.
	 *     o 16-bit service UUIDs (alert notifications).
	 */
	struct ble_hs_adv_fields fields;
	const char *device_name;
	memset(&fields, 0, sizeof fields);

	/* Advertise two flags:
	 *     o Discoverability in forthcoming advertisement (general)
	 *     o BLE-only (BR/EDR unsupported).
	 */
	fields.flags = BLE_HS_ADV_F_DISC_GEN |
				   BLE_HS_ADV_F_BREDR_UNSUP;

	/* Indicate that the TX power level field should be included; have the
	 * stack fill this value automatically.  This is done by assigning the
	 * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
	 */
	fields.tx_pwr_lvl_is_present = 1;
	fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

	device_name = ble_svc_gap_device_name();
	fields.name = (uint8_t *)device_name;
	fields.name_len = strlen(device_name);
	fields.name_is_complete = 1;

	fields.uuids16 = (ble_uuid16_t[]){
		BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)};
	fields.num_uuids16 = 1;
	fields.uuids16_is_complete = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0)
	{
		MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
		return;
	}

	// GAP - device connectivity definition
	struct ble_gap_adv_params adv_params;
	memset(&adv_params, 0, sizeof adv_params);
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
	rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
						   &adv_params, ble_gap_event, NULL);
	if (rc != 0)
	{
		MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
		return;
	}
}
// Callback para eventos do BLE
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
	switch (event->type)
	{
	case BLE_GAP_EVENT_CONNECT:
		ESP_LOGI(TAG, "BLE_GAP_EVENT_CONNECT: Device connected");
		conn_handle = event->connect.conn_handle; // Armazenar handle da conexão
		break;

	case BLE_GAP_EVENT_DISCONNECT:
		ESP_LOGI(TAG, "BLE_GAP_EVENT_DISCONNECT: Device disconnected; reason=%d ", event->disconnect.reason);
		conn_handle = BLE_HS_CONN_HANDLE_NONE; // Resetar handle
		ble_advertise();
		break;

	case BLE_GAP_EVENT_ADV_COMPLETE:
		ESP_LOGI(TAG, "BLE_GAP_EVENT_ADV_COMPLETE: Advertising complete");
		break;

	default:
		break;
	}
	return 0;
}

/*-----------------------------------------------------------
 * Callback para reset do BLE.
 *
 * Essa função é chamada quando o stack BLE reinicializa – por exemplo,
 * em caso de erro ou na inicialização. Aqui, pode-se limpar variáveis
 * de estado ou apenas registrar o evento.
 *----------------------------------------------------------*/
static void ble_on_reset(int reason)
{
	ESP_LOGI(TAG, "BLE Host Reset, reason: %d", reason);
}

/*-----------------------------------------------------------
 * Callback de sincronização do BLE.
 *
 * Chamado quando o stack BLE sincroniza com o controlador e está
 * pronto para iniciar as operações (ex.: publicidade). É aqui que
 * normalmente se inicia a publicidade.
 *----------------------------------------------------------*/
static void ble_on_sync(void)
{
	ESP_LOGI(TAG, "BLE Host Sync: Stack pronto");
	// Aqui poderíamos chamar uma função para iniciar a publicidade,
	// por exemplo: start_advertising();
	ble_hs_id_infer_auto(0, &own_addr_type); // Determines the best address type automatically
	ble_advertise();						 // Define the BLE connection
}

/*-----------------------------------------------------------
 * Função para inicializar o servidor GATT.
 *
 * Normalmente, aqui definiríamos os serviços e características a serem
 * expostos via BLE. Neste exemplo, usaremos um stub mínimo.
 *----------------------------------------------------------*/
// Tarefa principal do NimBLE
void ble_host_task(void *param)
{
	ESP_LOGI(TAG, "BLE Host Task Started");
	/* This function will return only when nimble_port_stop() is executed */
	nimble_port_run(); // Executa o loop de eventos do NimBLE
	nimble_port_freertos_deinit(); // Libera recursos da tarefa FreeRTOS
}
/*-----------------------------------------------------------
 * Função principal: ()
 *
 * Essa função é o ponto de entrada do aplicativo ESP-IDF.
 * Ela realiza a inicialização do NVS, configura e inicia o stack BLE
 * (tanto a camada de hardware quanto a camada de software NimBLE) e,
 * por fim, inicia a tarefa que gerenciará o loop de eventos do BLE.
 *----------------------------------------------------------*/
void app_ble_prh_main(void)
{
	esp_err_t ret;
	int rc;

	// Inicializa a NVS, utilizada para armazenar dados de calibração do PHY
	// e informações de emparelhamento (bonding).
	ret = nvs_flash_init();
	ESP_LOGI(TAG, "nvs_flash_init() ret = %d", ret);
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_LOGI(TAG, "Apagando NVS...");
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	/* Inicializa o HCI BLE. (esp_nimble_hci_init() é ser chamada implicitamente pelo nimble_port_init() em versoes mais recentes.)
	 *
	 * A função esp_nimble_hci_init() configura os recursos de hardware necessários
	 * para o funcionamento do BLE, como clocks, buffers e registradores. Essa etapa
	 * prepara o controlador BLE do ESP32 para ser usado pelo stack NimBLE.
	 */
	//ret = esp_nimble_hci_init();
	//ESP_ERROR_CHECK(ret);

	/* Inicializa o stack NimBLE (camada host BLE).
	 *
	 * A função nimble_port_init() configura as estruturas internas do stack NimBLE,
	 * como gerenciamento de conexões, serviços GATT, GAP, L2CAP, etc.
	 * esp_nimble_hci_init() é ser chamada implicitamente pelo nimble_port_init() em versoes mais recentes.
	 */
	ret = nimble_port_init();
	ESP_LOGI(TAG, "nimble_port_init() ret = %d", ret);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Falha na inicialização do NimBLE");
		return;
	}

	/* Configura a estrutura global de configuração do host BLE (ble_hs_cfg).
	 *
	 * Essa estrutura permite definir callbacks e parâmetros que controlam
	 * o comportamento do stack BLE.
	 */
	ble_hs_cfg.reset_cb = ble_on_reset;					 // Callback chamado em caso de reset do BLE.
	ble_hs_cfg.sync_cb = ble_on_sync;					 // Callback chamado quando o BLE está sincronizado e pronto.
	ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb; // Callback para o registro de serviços GATT.
	// Configuração do modo de entrada/saída para emparelhamento:
	ble_hs_cfg.sm_io_cap = 3; 							// Define que o dispositivo não possui entrada/saída para emparelhamento.
	ble_hs_cfg.sm_sc = 0;								// Secure Connections desabilitado (0 = desabilitado).

	/* Inicializa o servidor GATT.
	 *
	 * Aqui, normalmente, você registraria os serviços e características que seu dispositivo
	 */
	rc = gatt_svr_init();
	assert(rc == 0);

	/* Inicializa o sistema de armazenamento BLE.
	 *
	 * Essa função configura o armazenamento interno usado pelo stack para persistir
	 * dados como chaves de emparelhamento.
	 */
	ble_store_config_init();

	/* Inicializa a tarefa do host NimBLE.
	 *
	 * A função nimble_port_freertos_init() cria uma tarefa FreeRTOS que executa
	 * o loop de eventos do stack NimBLE, responsável por tratar eventos BLE.
	 * Passar NULL usa os parâmetros padrão para a tarefa.
	 * XXX Need to have template for store
	 */
	//nimble_port_freertos_init(ble_host_task); // o codigo abaixo faz a mesma coisa
	static TaskHandle_t host_task_h = NULL;
	xTaskCreatePinnedToCore(ble_host_task,
							"nimble_host",
							NIMBLE_HS_STACK_SIZE,
							NULL,
							NIMBLE_HOST_TASK_PRIO,
							&host_task_h,
							NIMBLE_CORE);

	ESP_LOGI(TAG, "NimBLE stack inicializado e em execução");
}
