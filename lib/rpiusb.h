#include <libusb-1.0/libusb.h>

typedef struct libusb_device_descriptor libusb_device_descriptor;

int
usb_id(
    char* string
);

void
usb_init(
    libusb_context **context,
    libusb_device_handle **device,
    int device_vid,
    int device_pid,
    int interface
);

void 
usb_exit(
    libusb_context **context,
    libusb_device_handle **device,
    int interface
);

void 
usb_control(
    libusb_context **context,
    libusb_device_handle **device,
    int interface,
    uint8_t request_type,
    uint8_t request,
    uint16_t value,
    uint16_t index,
    unsigned char* data,
    uint16_t data_length
);

void 
usb_bulk(
    libusb_context **context,
    libusb_device_handle **device,
    int interface,
    unsigned char endpoint,
    unsigned char* data,
    int data_length
);
