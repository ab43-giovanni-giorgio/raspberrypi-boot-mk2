#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpiusb.h"

#define ID_LEN          4
#define TIMEOUT         100     // 0.1 seconds
#define TIMEOUT_BULK    10000   //  10 seconds

int
usb_id(
    char* string
){
    // Note:
    //  String has to be supplied without the preceeding 0x hex designator.

    int length = 0;

    length = strlen(string);

    if(ID_LEN != length){
        fprintf(
            stderr, 
            "Error. USB VID/PID string should be %d characters long. " 
            "Got %d characters long string instead: \"%s\".\n", 
            ID_LEN, length, string
        );
        return -1;
    }

    for(int i = 0; i < length; i++){

        string[i] = tolower(string[i]);

        switch (string[i]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                break;
            
            default:
                fprintf(
                    stderr, 
                    "Error. Detected non hex value: %c in the string: %s\n", 
                    string[i], string
                );
                return -1;
        }
    }

    return (int) strtol(string, NULL, 16);
}

void 
usb_init(
    libusb_context **context,
    libusb_device_handle **device,
    int device_vid,
    int device_pid,
    int interface
){

    int ret = -1;

    if (0 != libusb_init(context)) {
        fprintf(
            stderr, 
            "Error. Unable to initialize libusb.\n"
        );
        exit(EXIT_FAILURE);
    }

    *device = libusb_open_device_with_vid_pid(
        *context, 
        device_vid, 
        device_pid
    );

    if (NULL == *device){
        fprintf(
            stderr, 
            "Error. Unable to find/open USB device with "
            "VID: %04x, PID: %04x.\n", 
            device_vid, device_pid
        );
        libusb_exit(*context);
        exit(EXIT_FAILURE);
    }

    ret = libusb_claim_interface(*device, interface);

    if (0 != ret){
        fprintf(
            stderr, 
            "Error. Unable to claim interface for USB device with "
            "VID: %04x, PID: %04x. Reason: %s\n", 
            device_vid, device_pid, libusb_error_name(ret)
        );
        libusb_close(*device);
        libusb_exit(*context);
        exit(EXIT_FAILURE);
    }

}

void 
usb_exit(
    libusb_context **context,
    libusb_device_handle **device,
    int interface
){
    libusb_release_interface(*device, interface);
    libusb_close(*device);
    libusb_exit(*context);
}

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
){

    int ret = -1;

    ret = libusb_control_transfer(
        *device, 
        request_type, 
        request, 
        value, 
        index, 
        data, 
        data_length, 
        TIMEOUT
    );

    if (data_length != ret){
        fprintf(
            stderr, 
            "Error. Control transfer failed. Code: %d, Reason: %s\n", 
            ret, libusb_error_name(ret)
        );
        usb_exit(context, device, interface);
        exit(EXIT_FAILURE);
    }

}

void 
usb_bulk(
    libusb_context **context,
    libusb_device_handle **device,
    int interface,
    unsigned char endpoint,
    unsigned char* data,
    int data_length
){

    int ret = -1;
    int transferred = 0;

    ret = libusb_bulk_transfer(
        *device, 
        endpoint,
        data, 
        data_length, 
        &transferred, 
        TIMEOUT_BULK
    );

    if (0 != ret || transferred != data_length){
        fprintf(
            stderr, 
            "Error. Bulk transfer failed. Code: %d, Reason: %s. "
            "Transferred bytes: %d out of total %d.\n", 
            ret, libusb_error_name(ret),
            transferred, data_length
        );
        usb_exit(context, device, interface);
        exit(EXIT_FAILURE);
    }

}
