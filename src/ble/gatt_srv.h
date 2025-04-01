#ifndef H_GATT_SRV_
#define H_GATT_SRV_

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"
#include "gps/nmea_parser.h"
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
#define BLE_APPEARANCE_GENERIC_WATCH  0x00C0


#define BLE_UUID_LOCATION_NAV_SVC     0x1819 // LN Service
#define BLE_UUID_LOC_SPEED_CHR        0x2A67 // Characteristic "Location and Speed
#define CUSTOM_JSON_SERVICE_UUID  0xABF0
#define CUSTOM_JSON_CHR_UUID      0xABF1

/* Flags bit definitions (simplificado):
   - bit0 = Instant Speed Present
   - bit1 = Total Distance Present
   - bit2 = Location Present
   - bit3 = Elevation Present
   - bit4 = Heading Present
   - bit5 = Rolling Time Present
   - bit6 = UTC Time Present
   - bit7 = Remaining bits
*/
#define LN_FLAG_LOCATION_PRESENT    (1 << 2)
#define LN_FLAG_ELEVATION_PRESENT   (1 << 3)
#define LN_FLAG_TIME_PRESENT        (1 << 6)
#define LN_FLAG_SPEED_PRESENT       (1 << 0)

	// Handles para conexão e características
	static uint16_t conn_handle;
   static uint16_t attr_handle_location_speed;
   static uint16_t attr_handle_json;

	void update_gpgga_data(const char *data);
	void update_gprmc_data(const char *data);
   void update_location_speed_data(const gps_t gps);
   void update_location_speed_json(const gps_t gps);
	void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
	int gatt_svr_init(void);

#endif