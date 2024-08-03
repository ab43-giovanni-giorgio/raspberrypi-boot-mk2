#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/rpiusb.h"
#include "fw/bootcode.h"
#include "fw/start.h"

#define INTERFACE   0
#define ENDPOINT    1

void help(char* program_name){
    fprintf(
        stderr,
        "Usage:\n"
        "   %s  USB_VID  USB_PID\n\n"
        "Parameters:\n"
        "   USB_VID     4 digit VID hex number, without the 0x prefix.\n"
        "   USB_PID     4 digit PID hex number, without the 0x prefix.\n\n"
        "All parameters must be supplied in the order specified.\n",
        program_name
    );
}

void stage1(
    libusb_context *context,
    libusb_device_handle *device
){
    int init_len = 24;
    int response_len = 4;
    
    unsigned char init[init_len];
    unsigned char response[response_len];

    memset(init, 0, init_len);
    memset(response, 0, response_len);

    // prepare initial transfer, which contains just the size of bootcode.bin
    init[0] = (BOOTCODE_BIN_LEN & 0x00FF);
    init[1] = (BOOTCODE_BIN_LEN & 0xFF00) >> 8;

    // declare the next transfer will be 24 bytes long.
    usb_control(&context, &device, INTERFACE, 
        0x40, 0, init_len, 0, NULL, 0
    );

    // inside the 24 bytes we're sending the size of bootcode.bin 
    // and nothing else but 0s
    usb_bulk(&context, &device, INTERFACE, ENDPOINT, 
        init, init_len
    );

    // declare the next transfer will be the size of bootcode.bin long
    usb_control(&context, &device, INTERFACE, 
        0x40, 0, BOOTCODE_BIN_LEN, 0, NULL, 0
    );

    // transfer bootcode.bin
    usb_bulk(&context, &device, INTERFACE, ENDPOINT, 
        bootcode_bin, BOOTCODE_BIN_LEN
    );

    // no idea what's going on here.
    // something is probably being checked. we're getting 4 bytes back,
    // probably some status values?
    usb_control(&context, &device, INTERFACE, 
        0xc0, 0, 4, 0, response, response_len
    );
}

void stage2(
    libusb_context *context,
    libusb_device_handle *device
){
    int response_len = 260;
    unsigned char response[response_len];
    memset(response, 0, response_len);

    for(int i = 0; i < 6; i++){

        usb_control(&context, &device, INTERFACE, 
            0xc0, 0, 0x0104, 0, response, response_len
        );

        switch(i){

            // case 0,1,2:
            // autoboot.txt, config.txt, recovery.elf

            // start.elf
            case 3:
            case 4:
                usb_control(&context, &device, INTERFACE, 
                    0x40, 0, 0xfbb8, 0x0007, NULL, 0
                );

                if (i == 4){
                    usb_bulk(&context, &device, INTERFACE, ENDPOINT, 
                        start_elf, START_ELF_LEN
                    );
                }
                break;
            
            default:
                usb_control(&context, &device, INTERFACE, 
                    0x40, 0, 0, 0, NULL, 0
                );
                break;
        }
    }
}

int str_to_hex(char* string){

    for(int i = 0; i < strlen(string); i++){

        // Standardize
        string[i] = tolower(string[i]);

        // Check if hex value
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
                    "Error. Detected non hex value: %c in the string: %s\n", string[i], string
                );
                return -1;
        }
    }
    
    return (int) strtol(string, NULL, 16);
}

int main(int argc, char **argv){

    if(argc != 3 || 4 != strlen(argv[1]) || 4 != strlen(argv[2])){
        help(argv[0]);
        exit(EXIT_FAILURE);
    }

    int vid = str_to_hex(argv[1]);
    int pid = str_to_hex(argv[2]);

    if(-1 == vid || -1 == pid)
        exit(EXIT_FAILURE);

    libusb_context *ctx;
    libusb_device_handle *dev;
    libusb_device_descriptor desc;

    usb_init(&ctx, &dev, vid, pid, INTERFACE);

    libusb_get_device_descriptor(
        libusb_get_device(dev), 
        &desc
    );

    switch(desc.iSerialNumber){

        case 0:
            stage1(ctx, dev);
            break;

        case 1:
            stage2(ctx, dev);
            break;

        default:
            fprintf(
                stderr, 
                "Error. Unknown serial number detected: %d.\n",
                desc.iSerialNumber
            );
            usb_exit(&ctx, &dev, INTERFACE);
            exit(EXIT_FAILURE);
            break;
    }

    usb_exit(&ctx, &dev, INTERFACE);

    return 0;
}
