#ifndef H_GATT_SRV_
#define H_GATT_SRV_

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"
#ifdef __cplusplus
extern "C" {
#endif

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

/** GATT server. */
#define GATT_SVR_SVC_ALERT_UUID               0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID   0x2A47
#define GATT_SVR_CHR_NEW_ALERT                0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID   0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID      0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT        0x2A44

/** GAP appeance macros que nao estao declarados em services/gap/ble_svc_gap.h */
#define BLE_SVC_GAP_APPEARANCE_CATEGORY_GPS   0x044C

/** GNSS Server service UUID. */
#define GNSS_SERVER_SVC_UUID   0x1136
/** N TENHO A CERTEZA */
#define GNSS__SVC_UUID 0x1135
#define LATITUDE_CHR_UUID 0x2A67
#define LONGITUDE_CHR_UUID 0x2A68

	// Handles para conexão e características
	static uint16_t conn_handle;
	static uint16_t attr_handle_gpgga;
	static uint16_t attr_handle_gprmc;

	void update_gpgga_data(const char *data);
	void update_gprmc_data(const char *data);
	void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
	int gatt_svr_init(void);

#endif