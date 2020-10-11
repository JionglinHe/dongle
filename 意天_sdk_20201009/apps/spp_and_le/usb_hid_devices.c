#include "os/os_api.h"

#include "app_config.h"

#if     TCFG_PC_ENABLE
#include "usb/device/usb_stack.h"
#include "usb/device/hid.h"
#include "usb_config.h"
#include "timer.h"
#include "usb/host/usb_hid_keys.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
#define     HID_ITF_0_EP_OUT   1
#define     HID_ITF_0_EP_IN    1
#define     HID_ITF_1_EP_OUT   2
#define     HID_ITF_1_EP_IN    2
#define     HID_ITF_2_EP_OUT   3
#define     HID_ITF_2_EP_IN    3

static const u8 sHIDDescriptor[] = {
//HID
    //InterfaceDeszcriptor:
    USB_DT_INTERFACE_SIZE,     // Length
    USB_DT_INTERFACE,          // DescriptorType
    /* 0x04,                      // bInterface number */
    0x00,                       // bInterface number
    0x00,                      // AlternateSetting
    0x01,                      // NumEndpoint
    /* 0x02,                        // NumEndpoint */
    USB_CLASS_HID,             // Class = Human Interface Device
    0x00,                      // Subclass, 0 No subclass, 1 Boot Interface subclass
    0x00,                      // Procotol, 0 None, 1 Keyboard, 2 Mouse
    0x00,                      // Interface Name


    //HIDDescriptor:
    0x09,                      // bLength
    USB_HID_DT_HID,            // bDescriptorType, HID Descriptor
    0x01, 0x02,                // bcdHID, HID Class Specification release NO.
    0x00,                      // bCuntryCode, Country localization (=none)
    0x01,                       // bNumDescriptors, Number of descriptors to follow
    0x22,                       // bDescriptorType, Report Desc. 0x22, Physical Desc. 0x23
    0,//LOW(ReportLength)
    0, //HIGH(ReportLength)

    //EndpointDescriptor:
    USB_DT_ENDPOINT_SIZE,       // bLength
    USB_DT_ENDPOINT,            // bDescriptorType, Type
    USB_DIR_IN | HID_EP_IN,     // bEndpointAddress
    USB_ENDPOINT_XFER_INT,      // Interrupt
    LOBYTE(MAXP_SIZE_HIDIN), HIBYTE(MAXP_SIZE_HIDIN),// Maximum packet size
    0x01,     // Poll every 10msec seconds

//@Endpoint Descriptor:
    /* USB_DT_ENDPOINT_SIZE,       // bLength
    USB_DT_ENDPOINT,            // bDescriptorType, Type
    USB_DIR_OUT | HID_EP_OUT,   // bEndpointAddress
    USB_ENDPOINT_XFER_INT,      // Interrupt
    LOBYTE(MAXP_SIZE_HIDOUT), HIBYTE(MAXP_SIZE_HIDOUT),// Maximum packet size
    0x01,                       // bInterval, for high speed 2^(n-1) * 125us, for full/low speed n * 1ms */
};

static const u8 hid_itf_0[]={
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x02,        // bInterfaceNumber 2
    0x00,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x10, 0x01,  // bcdHID 1.10
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x34, 0x00,  // wDescriptorLength[0] 52

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    HID_ITF_0_EP_IN|USB_DIR_IN,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x08, 0x00,  // wMaxPacketSize 8
    0x01,        // bInterval 1 (unit depends on device speed)
};
static const u8 hid_itf_1[]={
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x03,        // bInterfaceNumber 3
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x01,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x10, 0x01,  // bcdHID 1.10
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x1C, 0x00,  // wDescriptorLength[0] 28

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    HID_ITF_1_EP_IN|USB_DIR_IN,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    HID_ITF_1_EP_OUT|USB_DIR_OUT,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};

static const u8 hid_itf_2[]={
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x04,        // bInterfaceNumber 3
    0x00,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x01,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x10, 0x01,  // bcdHID 1.10
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x1C, 0x00,  // wDescriptorLength[0] 28

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    HID_ITF_2_EP_OUT|USB_DIR_OUT,        // bEndpointAddress (OUT/H2D)
    0x02,        // bmAttributes (Bulk)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};

static const u8 *hid_itf_arry[]={
    hid_itf_0,hid_itf_1,hid_itf_2
};
static const u32 hid_itf_size[]={
    sizeof(hid_itf_0),sizeof(hid_itf_1),sizeof(hid_itf_2)
};


static const u8 hid_report_0[]={
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x85, 0x02,        //   report ID(2)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
// 52 bytes
    
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        // report ID(3)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x05,        //   Usage Maximum (Kana)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (-1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0xFF,        //   Usage Maximum (0xFF)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
// 63 bytes

    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x04,        // report ID(4)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x13,        //   Report Count (19)
    0x0A, 0x94, 0x01,  //   Usage (AL Local Machine Browser)
    0x0A, 0x92, 0x01,  //   Usage (AL Calculator)
    0x0A, 0x83, 0x01,  //   Usage (AL Consumer Control Configuration)
    0x0A, 0x23, 0x02,  //   Usage (AC Home)
    0x0A, 0x8A, 0x01,  //   Usage (AL Email Reader)
    0x0A, 0x2A, 0x02,  //   Usage (AC Bookmarks)
    0x0A, 0x21, 0x02,  //   Usage (AC Search)
    0x0A, 0x24, 0x02,  //   Usage (AC Back)
    0x0A, 0x25, 0x02,  //   Usage (AC Forward)
    0x0A, 0x26, 0x02,  //   Usage (AC Stop)
    0x0A, 0x27, 0x02,  //   Usage (AC Refresh)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB7,        //   Usage (Stop)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0x41,        //   Usage (Menu Pick)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x05,        //   Report Size (5)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
// 72 bytes

};
void custom_hid_recv_data(struct usb_device_t *usb_device, u32 ep);
static const u8 hid_report_1[]={
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x00,        // Usage (0x00)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (-1)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x08,        //   Usage Maximum (0x08)
    0x95, 0x40,        //   Report Count (64)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x08,        //   Usage Maximum (0x08)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
// 28 bytes
};
void hid_bulk_recv_data(struct usb_device_t *usb_device, u32 ep);
static const u8 hid_report_2[]={
    0x06, 0x02, 0xFF,  // Usage Page (Vendor Defined 0xFF02)
    0x09, 0x02,        // Usage (0x02)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x03,        //   Usage (0x03)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x04,        //   Usage (0x04)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};
/* static const u8 *hid_report_map; */
/* static int hid_report_map_size = 0; */
static const u8 *hid_report_map[]={
    hid_report_0,hid_report_1,hid_report_2
};
static int hid_report_map_size[]={
    sizeof(hid_report_0),sizeof(hid_report_1),sizeof(hid_report_2)
};
void usb_hid_set_repport_map(const u8 *map, int size)
{
    /* hid_report_map_size = size; */
    /* hid_report_map = map; */
}

static u32 get_hid_report_desc_len(u32 index)
{
    ASSERT(hid_report_map_size != NULL, "%s\n", "report map err\n");
    return hid_report_map_size[index];
}
static void *get_hid_report_desc(u32 index)
{
    return hid_report_map[index];
}


static u32 hid_tx_data(struct usb_device_t *usb_device, const u8 *buffer, u32 len)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    return usb_g_intr_write(usb_id, HID_EP_IN, buffer, len);
}
static void hid_rx_data(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer = usb_get_ep_buffer(usb_id, ep);
    u32 rx_len = usb_g_intr_read(usb_id, ep, NULL, 64, 0);
    /* hid_tx_data(usb_device, ep_buffer, rx_len); */
    log_debug("ep%d rx_data:",ep);
    printf_buf(ep_buffer,rx_len);
}

static void hid_endpoint_init(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ep_rx = 0;
    u32 ep_tx = 0;
    u32 ep_rx_size = 0;
    u32 ep_tx_size = 0;
    switch (itf)
    {
    case 0:
        ep_rx = HID_ITF_0_EP_IN;
        ep_rx_size = 8;
        break;
    case 1:
        ep_rx = HID_ITF_1_EP_IN;
        ep_rx_size = 64;
        ep_tx = HID_ITF_1_EP_OUT;
        ep_tx_size = 64;
        break;
    case 2:
        ep_tx = HID_ITF_2_EP_OUT;
        ep_tx_size = 64;
        if(ep_tx) {
            u8 *ep_buffer = usb_get_ep_buffer(usb_id, ep_tx);
            usb_g_set_intr_hander(usb_id, ep_tx, hid_bulk_recv_data);
            usb_g_ep_config(usb_id, HID_EP_OUT, USB_ENDPOINT_XFER_BULK, 1, ep_buffer, ep_tx_size);
            usb_enable_ep(usb_id, ep_tx);
            return;
        }
        break;
    default:
        break;
    }
    if(ep_rx) {
        u8 *ep_buffer = usb_get_ep_buffer(usb_id, ep_rx | USB_DIR_IN);
        usb_g_ep_config(usb_id, ep_rx | USB_DIR_IN, USB_ENDPOINT_XFER_INT, 0, ep_buffer, ep_rx_size);
    }

    if(ep_tx) {
        u8 *ep_buffer = usb_get_ep_buffer(usb_id, ep_tx);
        /* usb_g_set_intr_hander(usb_id, ep_tx, hid_rx_data); */
        usb_g_set_intr_hander(usb_id, ep_tx, custom_hid_recv_data);
        usb_g_ep_config(usb_id, HID_EP_OUT, USB_ENDPOINT_XFER_INT, 1, ep_buffer, ep_tx_size);
        usb_enable_ep(usb_id, ep_tx);
    }
}

static void hid_reset(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);
#if USB_ROOT2
    usb_disable_ep(usb_id, HID_EP_OUT);
#else
    hid_endpoint_init(usb_device, itf);
#endif
}
static struct usb_device_t *hid_device;
static u32 hid_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *req)
{
    hid_device = usb_device;

    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = req->bRequestType & USB_TYPE_MASK;
    u8 *hid_desc = NULL;
    u32 itf_num = LOBYTE(req->wIndex);

    switch (bRequestType) {
    case USB_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_DESCRIPTOR:
            switch (HIBYTE(req->wValue)) {
            case USB_HID_DT_HID:
                hid_desc = hid_itf_arry[itf_num];
                tx_payload = (u8 *)hid_desc + USB_DT_INTERFACE_SIZE;
                /* tx_payload = (u8 *)sHIDDescriptor + USB_DT_INTERFACE_SIZE; */
                tx_len = 9;
                tx_payload = usb_set_data_payload(usb_device, req, tx_payload, tx_len);
                tx_payload[7] = LOBYTE(get_hid_report_desc_len(req->wIndex));
                tx_payload[8] = HIBYTE(get_hid_report_desc_len(req->wIndex));
                break;
            case USB_HID_DT_REPORT:
                hid_endpoint_init(usb_device, req->wIndex);
                tx_len = get_hid_report_desc_len(req->wIndex);
                tx_payload = get_hid_report_desc(req->wIndex);
                usb_set_data_payload(usb_device, req, tx_payload, tx_len);
                break;
            }// USB_REQ_GET_DESCRIPTOR
            break;
        case USB_REQ_SET_DESCRIPTOR:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_SET_INTERFACE:
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                //只有一个interface 没有Alternate
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
            break;
        case USB_REQ_GET_INTERFACE:
            if (req->wLength) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                tx_payload[0] = 0x00;
                usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            }
            break;
        case USB_REQ_GET_STATUS:
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
            break;
        }//bRequest @ USB_TYPE_STANDARD
        break;

    case USB_TYPE_CLASS: {
        switch (req->bRequest) {
        case USB_REQ_SET_IDLE:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_GET_IDLE:
            tx_len = 1;
            tx_payload[0] = 0;
            usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            break;
        }//bRequest @ USB_TYPE_CLASS
    }
    break;
    }
    return 0;
}

static void usb_tx_test(void *p);
u32 hid_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    log_debug("hid interface num:%d\n", *cur_itf_num);
    static u32 hid_timer_tx_id = 0;
    if(hid_timer_tx_id==0){
        hid_timer_tx_id = sys_timer_add(NULL,usb_tx_test,2000);
    }
    u8 *_ptr = ptr;
    /* memcpy(ptr, sHIDDescriptor, sizeof(sHIDDescriptor)); */
    memcpy(ptr, hid_itf_arry[*cur_itf_num], hid_itf_size[*cur_itf_num]);

    ptr[2] = *cur_itf_num;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, hid_itf_hander) != *cur_itf_num) {
        ASSERT(0, "hid set interface_hander fail");
    }

    if (usb_set_reset_hander(usb_id, *cur_itf_num, hid_reset) != *cur_itf_num) {
        ASSERT(0, "hid set interface_reset_hander fail");
    }

    ptr[USB_DT_INTERFACE_SIZE + 7] = LOBYTE(get_hid_report_desc_len(*cur_itf_num));
    ptr[USB_DT_INTERFACE_SIZE + 8] = HIBYTE(get_hid_report_desc_len(*cur_itf_num));
    *cur_itf_num = *cur_itf_num + 1;
    /* return sizeof(sHIDDescriptor) ; */
    return hid_itf_size[*cur_itf_num -1];
}

u32 hid_send_data(const void *p, u32 len)
{
    return hid_tx_data(hid_device, p, len);
}

struct _mouse_point_t
{
    u8 btn;
    s8 x;
    s8 y;
    s8 wheel;
};

u32 hid_send_mouse_data(struct _mouse_point_t *p)
{
    const usb_dev usb_id=0;
    u8 buf[5];
    buf[0] = 0x02; //report ID 2
    memcpy(&buf[1],p,sizeof(struct _mouse_point_t));
    return usb_g_intr_write(usb_id, HID_ITF_0_EP_IN, buf, sizeof(buf));
}
u32 hid_send_keyboard_data(struct keyboard_data_t *p)
{
    const usb_dev usb_id=0;
    u8 buf[1+sizeof(struct keyboard_data_t)];
    buf[0] = 0x03; //report ID 3
    memcpy(&buf[1],p,sizeof(struct keyboard_data_t));
    return usb_g_intr_write(usb_id, HID_ITF_0_EP_IN, buf, sizeof(buf));
}
u32 hid_send_consumer_control(u32 key)
{   
    const usb_dev usb_id=0;
    u8 buf[5];
    buf[0] = 0x04; //report ID 4
    memcpy(&buf[1],&key,sizeof(key));
    return usb_g_intr_write(usb_id, HID_ITF_0_EP_IN, buf, 4);
}

u32 custom_hid_send_data(u8 *buffer)
{
    const usb_dev usb_id = 0;
    return usb_g_intr_write(usb_id, HID_ITF_1_EP_IN, buffer, 64);
}

void custom_hid_recv_data(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer = usb_get_ep_buffer(usb_id, ep);
    u32 rx_len = usb_g_intr_read(usb_id, ep, NULL, 64, 0);
    /* hid_tx_data(usb_device, ep_buffer, rx_len); */
    log_debug("ep%d rx_data:",ep);
    printf_buf(ep_buffer,rx_len);
}
void hid_bulk_recv_data(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 buf[64];
    u32 rx_len = usb_g_bulk_read(usb_id,HID_ITF_2_EP_OUT,buf,sizeof(buf),1);
    if(rx_len){
        printf_buf(buf,rx_len);
    }
}


// For test
//
#define _KEY_QUIT                   BIT(0)
#define _KEY_Media_Select_VCR       BIT(1)
#define _KEY_Recall_Las             BIT(2)
#define _KEY_Media_Select_WWW       BIT(4)
#define _KEY_add_100                BIT(6)
#define _KEY_Scan_Previous_Track 
#define _KEY_Volume_Increment       BIT(15)
#define _KEY_Volume_Decrement       BIT(16)
static void usb_tx_test(void *p)
{
#if 0
    //按下
    hid_send_consumer_control(_KEY_Volume_Increment);
    os_time_dly(2);
    //抬起
    hid_send_consumer_control(0);

    struct _mouse_point_t mouse_data={
        .btn=0,
        .x=1,
        .y=1,
        .wheel=-1,
    };
    
    /* hid_send_mouse_data(&mouse_data); */

    struct keyboard_data_t keyboard_data={0};
    keyboard_data.fun_key=_KEY_MOD_LSHIFT;
    keyboard_data.Keypad[0]=_KEY_A;
    //按下
    /* hid_send_keyboard_data(&keyboard_data); */
    os_time_dly(2);
    keyboard_data.fun_key=0;
    keyboard_data.Keypad[0]=0;
    //抬起
    /* hid_send_keyboard_data(&keyboard_data); */
#endif

    u8 buf[] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
        0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
        0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
        0x1f,0x20,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
        0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,
        0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,
        0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43
    };
    custom_hid_send_data(buf);
}
#endif
