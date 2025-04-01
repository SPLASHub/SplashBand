#ifndef UBX_H
#define UBX_H

#include <stdint.h>
#include <stddef.h> 
#include <stdbool.h>

//
// UBX header
//
#define UBX_SYNC_CHAR1 0xB5 // Caracter de sincronismo UBX
#define UBX_SYNC_CHAR2 0x62 // Caracter de sincronismo UBX

//
// ACK (0x05) - Acknowledge Messages
//
#define UBX_CLASS_ACK 0x05 // Ack/Nack Messages (respostas às mensagens CFG)
// IDs
#define UBX_ID_ACK_NAK 0x00 // Negative Acknowledge
#define UBX_ID_ACK_ACK 0x01 // Acknowledge
//
// AID (0x0B) - AssistNow Aiding Messages
//
#define UBX_CLASS_AID 0x0B // AssistNow Aiding Messages
// IDs
#define UBX_ID_AID_REQ 0x00    // AID-REQ
#define UBX_ID_AID_INI 0x01    // AID-INI
#define UBX_ID_AID_HUI 0x02    // AID-HUI
#define UBX_ID_AID_DATA 0x10   // AID-DATA
#define UBX_ID_AID_ALM 0x30    // AID-ALM
#define UBX_ID_AID_EPH 0x31    // AID-EPH
#define UBX_ID_AID_ALPSRV 0x32 // AID-ALPSRV
#define UBX_ID_AID_AOP 0x33    // AID-AOP
#define UBX_ID_AID_ALP 0x50    // AID-ALP
//
// CFG (0x06) - Configuration Input Messages
//
#define UBX_CLASS_CFG 0x06 // Configuration Messages
// IDs
#define UBX_ID_CFG_PRT 0x00    // CFG-PRT  (Port config)
#define UBX_ID_CFG_MSG 0x01    // CFG-MSG  (Enable/disable messages)
#define UBX_ID_CFG_INF 0x02    // CFG-INF  (Information messages configuration)
#define UBX_ID_CFG_RST 0x04    // CFG-RST  (Reset receiver / Clear backup data)
#define UBX_ID_CFG_DAT 0x06    // CFG-DAT  (Datum setting)
#define UBX_ID_CFG_RATE 0x08   // CFG-RATE (Nav/Measurement Rate Settings)
#define UBX_ID_CFG_CFG 0x09    // CFG-CFG  (Save/Load/Restore config)
#define UBX_ID_CFG_FXN 0x0E    // CFG-FXN  (FixNow config)
#define UBX_ID_CFG_EKF 0x12    // CFG-EKF  (EKF settings, ex. LEA-6R)
#define UBX_ID_CFG_ANT 0x13    // CFG-ANT  (Antenna control settings)
#define UBX_ID_CFG_SBAS 0x16   // CFG-SBAS (SBAS settings)
#define UBX_ID_CFG_NMEA 0x17   // CFG-NMEA (NMEA protocol config)
#define UBX_ID_CFG_USB 0x1B    // CFG-USB  (USB config)
#define UBX_ID_CFG_TMODE 0x1D  // CFG-TMODE (Time Mode settings)
#define UBX_ID_CFG_NVS 0x22    // CFG-NVS  (Clear, Save & Load data to/from NVS)
#define UBX_ID_CFG_NAVX5 0x23  // CFG-NAVX5 (Navigation engine expert settings)
#define UBX_ID_CFG_NAV5 0x24   // CFG-NAV5  (Navigation engine settings)
#define UBX_ID_CFG_ESFGWT 0x29 // CFG-ESFGWT (Gyro/WheelTick config - LEA-6R)
#define UBX_ID_CFG_ITFM 0x39   // CFG-ITFM (Jamming/Interference Monitor)
#define UBX_ID_CFG_PM 0x32     // CFG-PM   (Power management config)
#define UBX_ID_CFG_RINV 0x34   // CFG-RINV (Remote inventory config)
#define UBX_ID_CFG_TP 0x07     // CFG-TP   (TimePulse config)
#define UBX_ID_CFG_TP5 0x31    // CFG-TP5  (TimePulse parameters)
#define UBX_ID_CFG_PM2 0x3B    // CFG-PM2  (Extended Power management config)
#define UBX_ID_CFG_RXM 0x11    // CFG-RXM  (RXM config)
#define UBX_ID_CFG_TMODE2 0x3D // CFG-TMODE2 (Time Mode Settings 2)
//
// ESF (0x10) - External Sensor Fusion
//
#define UBX_CLASS_ESF 0x10 // External Sensor Fusion (ex. LEA-6R)
// IDs
#define UBX_ID_ESF_MEAS 0x02   // ESF-MEAS (Sensor fusion measurements)
#define UBX_ID_ESF_STATUS 0x10 // ESF-STATUS (Sensor fusion status)
//
// INF (0x04) - Information Messages
//
#define UBX_CLASS_INF 0x04 // Informational ASCII messages
// IDs
#define UBX_ID_INF_ERROR 0x00   // INF-ERROR
#define UBX_ID_INF_WARNING 0x01 // INF-WARNING
#define UBX_ID_INF_NOTICE 0x02  // INF-NOTICE
#define UBX_ID_INF_TEST 0x03    // INF-TEST
#define UBX_ID_INF_DEBUG 0x04   // INF-DEBUG
//
// MON (0x0A) - Monitoring Messages
//
#define UBX_CLASS_MON 0x0A // Monitoring
// IDs
#define UBX_ID_MON_IO 0x02    // MON-IO
#define UBX_ID_MON_VER 0x04   // MON-VER
#define UBX_ID_MON_MSGPP 0x06 // MON-MSGPP
#define UBX_ID_MON_RXBUF 0x07 // MON-RXBUF
#define UBX_ID_MON_TXBUF 0x08 // MON-TXBUF
#define UBX_ID_MON_HW 0x09    // MON-HW
#define UBX_ID_MON_HW2 0x0B   // MON-HW2
#define UBX_ID_MON_RXR 0x21   // MON-RXR (Receiver status)
//
// NAV (0x01) - Navigation Results
//
#define UBX_CLASS_NAV 0x01 // Navigation results: Position, Speed, Time, etc.
// IDs
#define UBX_ID_NAV_POSECEF 0x01   // NAV-POSECEF (Position in ECEF)
#define UBX_ID_NAV_POSLLH 0x02    // NAV-POSLLH  (Geodetic position)
#define UBX_ID_NAV_STATUS 0x03    // NAV-STATUS  (Receiver Nav Status)
#define UBX_ID_NAV_DOP 0x04       // NAV-DOP     (Dilution of precision)
#define UBX_ID_NAV_SOL 0x06       // NAV-SOL     (Nav solution info)
#define UBX_ID_NAV_VELECEF 0x11   // NAV-VELECEF (Velocidade em ECEF)
#define UBX_ID_NAV_VELNED 0x12    // NAV-VELNED  (Velocidade em NED)
#define UBX_ID_NAV_TIMEGPS 0x20   // NAV-TIMEGPS (GPS time solution)
#define UBX_ID_NAV_TIMEUTC 0x21   // NAV-TIMEUTC (UTC time solution)
#define UBX_ID_NAV_CLOCK 0x22     // NAV-CLOCK   (Clock solution)
#define UBX_ID_NAV_SVINFO 0x30    // NAV-SVINFO  (Space Vehicle info)
#define UBX_ID_NAV_DGPS 0x31      // NAV-DGPS    (DGPS Data)
#define UBX_ID_NAV_SBAS 0x32      // NAV-SBAS    (SBAS status data)
#define UBX_ID_NAV_EKFSTATUS 0x40 // NAV-EKFSTATUS (Dead Reckoning status)
#define UBX_ID_NAV_AOPSTATUS 0x60 // NAV-AOPSTATUS (AssistNow Autonomous status)
//
// RXM (0x02) - Receiver Manager Messages
//
#define UBX_CLASS_RXM 0x02 // Receiver Manager
// IDs
#define UBX_ID_RXM_RAW 0x10   // RXM-RAW   (Raw measurement data)
#define UBX_ID_RXM_SFRB 0x11  // RXM-SFRB  (Subframe buffer)
#define UBX_ID_RXM_SVSI 0x20  // RXM-SVSI  (SV status info)
#define UBX_ID_RXM_ALM 0x30   // RXM-ALM   (GPS Constellation Almanac Data)
#define UBX_ID_RXM_EPH 0x31   // RXM-EPH   (GPS Constellation Ephemeris Data)
#define UBX_ID_RXM_PMREQ 0x41 // RXM-PMREQ (Request Power Management task)
//
// TIM (0x0D) - Timing Messages
//
#define UBX_CLASS_TIM 0x0D // Timing messages
// IDs
#define UBX_ID_TIM_TP 0x01   // TIM-TP   (Timepulse timedata)
#define UBX_ID_TIM_TM2 0x03  // TIM-TM2  (Time mark data)
#define UBX_ID_TIM_SVIN 0x04 // TIM-SVIN (Survey-in data)
#define UBX_ID_TIM_VRFY 0x06 // TIM-VRFY (Sourced time verification)
//
// NEO-6M Message Class and ids
//
#define MSG_CLASS_NMEA_PROPRIETARY 0xF1
// Message IDs
#define MSG_ID_UBX00 0x00               // Poll a PUBX,00 message
#define MSG_ID_UBX00 0x00               // Lat/Long Position Data
#define MSG_ID_UBX03 0x03               // Poll a PUBX,03 message
#define MSG_ID_UBX03 0x03               // Satellite Status
#define MSG_ID_UBX04 0x04               // Poll a PUBX,04 message
#define MSG_ID_UBX04 0x04               // Time of Day and Clock Information
#define MSG_ID_UBX05 0x05               // Poll a PUBX,05 message
#define MSG_ID_UBX05 0x05               // Lat/Long Position Data
#define MSG_ID_UBX06 0x06               // Poll a PUBX,06 message
#define MSG_ID_UBX06 0x06               // Lat/Long Position Data
#define MSG_ID_UBX40 0x40               // Set NMEA message output rate
#define MSG_ID_UBX41 0x41               // Set Protocols and Baudrate
//
// NMEA Message Class and ids
//
#define MSG_CLASS_NMEA_STANDARD 0xF0
// Message IDs
#define MSG_ID_DTM      0x0A        //Datum Reference
#define MSG_ID_GBS      0x09        //GNSS Satellite Fault Detection
#define MSG_ID_GGA      0x00        //Global Positioning System Fix Data
#define MSG_ID_GLL      0x01        //Latitude/Longitude Data
#define MSG_ID_GPQ      0x40        //Poll Message
#define MSG_ID_GRS      0x06        //GNSS Range Residuals
#define MSG_ID_GSA      0x02        //GNSS DOP and Active Satellites
#define MSG_ID_GST      0x07        //GNSS Pseudorange Error Statistics
#define MSG_ID_GSV      0x03        //GNSS Satellites in View
#define MSG_ID_RMC      0x04        //Recommended Minimum Specific GNSS Data
#define MSG_ID_THS      0x0E        //True Heading and Status
#define MSG_ID_TXT      0x41        //Text Transmission
#define MSG_ID_VTG      0x05        //Course Over Ground and Ground Speed
#define MSG_ID_ZDA      0x08        //Time and Date
//
// Notificacoes personalizadas
//
typedef struct {
    int code;
    const char *msg;
} ubx_t;

#define CHECKSUM_V          (ubx_t){0xFFF0100, "Checksum valid"}
#define CHECKSUM_F          (ubx_t){0xFFF0101, "Checksum failed"}
#define UBX_PACKAGE         (ubx_t){0xFFF0200, "UBX package received"}
#define NOT_UBX_PACKAGE     (ubx_t){0xFFF0201, "Not a UBX package"}
#define ACK                 (ubx_t){0xFFF0300, "Acknowledgment received"}
#define NACK                (ubx_t){0xFFF0301, "Negative acknowledgment"}
#define INV_CLASS           (ubx_t){0xFFF0400, "Inválid Class"}
#define INV_ID              (ubx_t){0xFFF0500, "Inválid ID"}



typedef struct {
    uint8_t sync1;      // 0xB5
    uint8_t sync2;      // 0x62
    uint8_t ubxClass;   // Classe da mensagem   1 byte
    uint8_t ubxID;      // ID da mensagem   1 byte
    uint16_t payload_length;    // Tamanho do payload   2 bytes
    uint8_t *payload;	// Payload
    uint8_t ck_a;       // Checksum A   1 byte
    uint8_t ck_b;       // Checksum B   1 byte
} UBXPackege;

bool verify_checksum(uint8_t *data, size_t len);
bool is_ubx_packet(uint8_t *data, size_t len);
ubx_t CFG_acknowledged(uint8_t *data, size_t len);
uint8_t *create_ubx_cmd(uint8_t ubxClass, uint8_t ubxID, uint8_t *payload, size_t payload_size);
uint8_t *create_config_nmea_message(uint8_t msgClass, uint8_t msgID, uint8_t I2C_rate, uint8_t UART1_rate, uint8_t UART2_rate, uint8_t USB_rate, uint8_t SPI_rate, uint8_t Reserved, size_t *payload_size);
uint8_t *create_config_rate_message(uint16_t measRateMs, uint16_t navRate, uint16_t timeRef, size_t *payload_size);

#endif