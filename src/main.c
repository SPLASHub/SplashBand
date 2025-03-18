//ToDo: Experimentar gps e ble usar uarts diferentes- por gps em uart1
//ToDo: Experimentar semaforos para sincronizar gps e ble
//ToDo: Usar um buffer circular para o gps

#include "ble_prh_main.h" // usado neste codigo para iniciar o ble
#include "gatt_srv.h"	  // usado neste codigo para update_gpgga_data e update_gprmc_data
#include "gps_main.h"	  // usado neste codigo para iniciar o gps

void app_main(void)
{
	app_ble_prh_main(); // inicia o ble
	app_gps_main();
}