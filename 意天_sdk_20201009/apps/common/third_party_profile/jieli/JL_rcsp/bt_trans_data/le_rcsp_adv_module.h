// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _LE_SERVER_MODULE_H
#define _LE_SERVER_MODULE_H

#include <stdint.h>
#include "bt_common.h"
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV_RCSP)
//
// gatt profile include file, generated by jieli gatt_inc_generator.exe
//
static const uint8_t profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | WRITE | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x03, 0x00, 0x00, 0x2a,

    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae00
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x80, 0x00, 0x00, 0x28, 0x00, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0040 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x81, 0x00, 0x03, 0x28, 0x04, 0x82, 0x00, 0x01, 0xae,
    // 0x0041 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x82, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0042 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x83, 0x00, 0x03, 0x28, 0x10, 0x84, 0x00, 0x02, 0xae,
    // 0x0043 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x84, 0x00, 0x02, 0xae,
    // 0x0044 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x85, 0x00, 0x02, 0x29, 0x00, 0x00,
    //////////////////////////////////////////////////////
    //
    // 0x0014 PRIMARY_SERVICE  1812
    //
    //////////////////////////////////////////////////////
    // 0x0a, 0x00, 0x02, 0x00, 0x14, 0x00, 0x00, 0x28, 0x12, 0x18,

#if 0// authentication
    //
    10, 0x00,   0x00, 0x00,    0x1A, 0x00,     0x00, 0x28,     0x0A, 0x18,                                     //primary service declaration
    13, 0x00,   0x02, 0x00,    0x1B, 0x00,     0x03, 0x28,     0x02, 0x1C, 0x00, 0x50, 0x2A,                   //characteristic declaration
    15, 0x00,   0x02, 0x04,    0x1C, 0x00,     0x50, 0x2A,     0x02, 0x8A, 0x24, 0x66, 0x82, 0x34, 0x36,       //PnP ID
#endif

    // END
    0x00, 0x00,
};
//
// characteristics <--> handles
//
#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0082
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0084
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0085

// #define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0006
// #define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0008
// #define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
// #define ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE 0x000b
// #define ATT_CHARACTERISTIC_ae04_01_VALUE_HANDLE 0x000d
// #define ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE 0x000e
// #define ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE 0x0010
// #define ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE 0x0011
// #define ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE 0x0013




#define JL_OPCODE_SET_ADV 							    0xC0
#define	JL_OPCODE_GET_ADV  								0xC1
#define JL_OPCODE_ADV_DEVICE_NOTIFY 					0xC2
#define JL_OPCODE_ADV_NOTIFY_SETTING 					0xC3
#define JL_OPCODE_ADV_DEVICE_REQUEST					0xC4

#define ATTR_TYPE_BAT_VALUE  		(0)
#define ATTR_TYPE_EDR_NAME   		(1)
#define ATTR_TYPE_KEY_SETTING  		(2)
#define ATTR_TYPE_LED_SETTING  		(3)
#define ATTR_TYPE_MIC_SETTING  		(4)
#define ATTR_TYPE_WORK_MODE  		(5)
#define ATTR_TYPE_PRODUCT_MESSAGE  	(6)
#define ATTR_TYPE_TIME_STAMP		(7)
#define ATTR_TYPE_EQ_SETTING		(8)
#define ATTR_TYPE_HIGH_LOW_VOL		(9)

//rcsp功能模块使能
#define RCSP_ADV_NAME_SET_ENABLE        1
#define RCSP_ADV_KEY_SET_ENABLE         1
#define RCSP_ADV_LED_SET_ENABLE         1
#define RCSP_ADV_MIC_SET_ENABLE         1
#define RCSP_ADV_WORK_SET_ENABLE        1
#if (JL_EARPHONE_APP_EN)
#define RCSP_ADV_EQ_SET_ENABLE          1
#define RCSP_ADV_MUSIC_INFO_ENABLE      1
#define RCSP_ADV_HIGH_LOW_SET			1
#else
#define RCSP_ADV_EQ_SET_ENABLE          0
#define RCSP_ADV_MUSIC_INFO_ENABLE      0
#define RCSP_ADV_HIGH_LOW_SET			0
#endif
#define RCSP_ADV_PRODUCT_MSG_ENABLE     1

enum {
    BT_ADV_ENABLE,
    BT_ADV_DISABLE,
    BT_ADV_SET_EDR_CON_FLAG,
    BT_ADV_SET_BAT_CHARGE_L,
    BT_ADV_SET_BAT_CHARGE_R,
    BT_ADV_SET_BAT_CHARGE_C,
    BT_ADV_SET_BAT_PERCENT_L,
    BT_ADV_SET_BAT_PERCENT_R,
    BT_ADV_SET_BAT_PERCENT_C,
    BT_ADV_SET_NOTIFY_EN,
};

// BT_ADV_SET_EDR_CON_FLAG
#define SECNE_DISMISS			(0x00)
#define SECNE_UNCONNECTED		(0x01)
#define SECNE_CONNECTED			(0x02)
#define SECNE_CONNECTING		(0x03)
#define SECNE_CONNECTINLESS		(0x04)

#define TWS_FUNC_ID_SEQ_RAND_SYNC	(('S' << (3 * 8)) | ('E' << (2 * 8)) | ('Q' << (1 * 8)) | ('\0'))

enum {
    TWS_ADV_SEQ_CHANGE = 0,
    TWS_VERSON_INFO,
    TWS_UPDATE_INFO,
};

extern int  bt_ble_adv_ioctl(u32 cmd, u32 priv, u8 mode);
extern void bt_adv_seq_change(void);
void ble_app_disconnect(void);
void bt_ble_rcsp_adv_enable(void);
void bt_ble_rcsp_adv_disable(void);
u8 get_ble_adv_notify(void);
void set_ble_adv_notify(u8 en);
u8 get_connect_flag(void);
void set_connect_flag(u8 value);
void ble_module_enable(u8 en);

#endif
#endif
