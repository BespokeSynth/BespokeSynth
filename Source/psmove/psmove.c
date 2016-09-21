
 /**
 * PS Move API - An interface for the PS Move Motion Controller
 * Copyright (c) 2011 Thomas Perl <m@thp.io>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 **/

#include "psmove.h"

#include "hidapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include <assert.h>

/* OS-specific includes, for getting the Bluetooth address */
#ifdef __APPLE__
#  include <IOBluetooth/Bluetooth.h>
#endif

#ifdef __linux
#  include <bluetooth/bluetooth.h>
#  include <bluetooth/hci.h>
#  include <sys/ioctl.h>
#endif


/* Begin private definitions */

/* Vendor ID and Product ID of PS Move Controller */
#define PSMOVE_VID 0x054c
#define PSMOVE_PID 0x03d5

/* Buffer size for writing LEDs and reading sensor data */
#define PSMOVE_BUFFER_SIZE 49

/* Buffer size for calibration data */
#define PSMOVE_CALIBRATION_SIZE 49

/* Buffer size for the Bluetooth address get request */
#define PSMOVE_BTADDR_GET_SIZE 16

/* Buffer size for the Bluetooth address set request */
#define PSMOVE_BTADDR_SET_SIZE 23

enum PSMove_Request_Type {
    PSMove_Req_GetInput = 0x01,
    PSMove_Req_SetLEDs = 0x02,
    PSMove_Req_GetBTAddr = 0x04,
    PSMove_Req_SetBTAddr = 0x05,
    PSMove_Req_GetCalibration = 0x10,
};

typedef struct {
    unsigned char type; /* message type, must be PSMove_Req_SetLEDs */
    unsigned char _zero; /* must be zero */
    unsigned char r; /* red value, 0x00..0xff */
    unsigned char g; /* green value, 0x00..0xff */
    unsigned char b; /* blue value, 0x00..0xff */
    unsigned char rumble2; /* unknown, should be 0x00 for now */
    unsigned char rumble; /* rumble value, 0x00..0xff */
    unsigned char _padding[PSMOVE_BUFFER_SIZE-7]; /* must be zero */
} PSMove_Data_LEDs;

typedef struct {
    unsigned char type; /* message type, must be PSMove_Req_GetInput */
    unsigned char buttons1;
    unsigned char buttons2;
    unsigned char buttons3;
    unsigned char buttons4;
    unsigned char _unk5;
    unsigned char trigger; /* trigger value; 0..255 */
    unsigned char _unk7;
    unsigned char _unk8;
    unsigned char _unk9;
    unsigned char _unk10;
    unsigned char _unk11;
    unsigned char battery; /* battery level; 0x05 = max, 0xEE = USB charging */
    unsigned char aXlow; /* low byte of accelerometer X value */
    unsigned char aXhigh; /* high byte of accelerometer X value */
    unsigned char aYlow;
    unsigned char aYhigh;
    unsigned char aZlow;
    unsigned char aZhigh;
    unsigned char aXlow2; /* low byte of accelerometer X value, 2nd frame */
    unsigned char aXhigh2; /* high byte of accelerometer X value, 2nd frame */
    unsigned char aYlow2;
    unsigned char aYhigh2;
    unsigned char aZlow2;
    unsigned char aZhigh2;
    unsigned char gXlow; /* low byte of gyro X value */
    unsigned char gXhigh; /* high byte of gyro X value */
    unsigned char gYlow;
    unsigned char gYhigh;
    unsigned char gZlow;
    unsigned char gZhigh;
    unsigned char gXlow2; /* low byte of gyro X value, 2nd frame */
    unsigned char gXhigh2; /* high byte of gyro X value, 2nd frame */
    unsigned char gYlow2;
    unsigned char gYhigh2;
    unsigned char gZlow2;
    unsigned char gZhigh2;
    unsigned char templow; /* temperature */
    unsigned char temphigh;
    unsigned char mag40; /* magnetometer data ??? */
    unsigned char mag41;
    unsigned char mag42;
    unsigned char _padding[PSMOVE_BUFFER_SIZE-42]; /* unknown */
} PSMove_Data_Input;

struct _PSMove {
    /* The handle to the HIDAPI device */
    hid_device *handle;

    /* Index (at connection time) - not exposed yet */
    int id;

    /* Various buffers for PS Move-related data */
    PSMove_Data_LEDs leds;
    PSMove_Data_Input input;

    /* Save location for the controller BTAddr */
    PSMove_Data_BTAddr btaddr;
};

/* Macro: Print a critical message if an assertion fails */
#define psmove_CRITICAL(x) \
        {fprintf(stderr, "[PSMOVE] Assertion fail in %s: %s\n", __func__, x);}

/* Macros: Return immediately if an assertion fails + log */
#define psmove_return_if_fail(expr) \
        {if(!(expr)){psmove_CRITICAL(#expr);return;}}
#define psmove_return_val_if_fail(expr, val) \
        {if(!(expr)){psmove_CRITICAL(#expr);return(val);}}

/* End private definitions */

/* Private functionality needed by the Linux version */
#if defined(__linux)

int _psmove_linux_bt_dev_info(int s, int dev_id, long arg)
{
    struct hci_dev_info di = { dev_id: dev_id };
    unsigned char *btaddr = (void*)arg;
    int i;

    if (ioctl(s, HCIGETDEVINFO, (void *) &di) == 0) {
        for (i=0; i<6; i++) {
            btaddr[i] = di.bdaddr.b[5-i];
        }
    }

    return 0;
}

#endif /* defined(__linux) */


/* Start implementation of the API */

int
psmove_count_connected()
{
    struct hid_device_info *devs, *cur_dev;
    int count = 0;

    devs = hid_enumerate(PSMOVE_VID, PSMOVE_PID);
    cur_dev = devs;
    while (cur_dev) {
        count++;
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    return count;
}

PSMove *
psmove_connect_internal(wchar_t *serial, char *path, int id)
{
    PSMove *move = (PSMove*)calloc(1, sizeof(PSMove));

    if (serial == NULL && path != NULL) {
        move->handle = hid_open_path(path);
    } else {
        move->handle = hid_open(PSMOVE_VID, PSMOVE_PID, serial);
    }

    if (!move->handle) {
        free(move);
        return NULL;
    }

    /* Use Non-Blocking I/O */
    hid_set_nonblocking(move->handle, 1);

    /* Message type for LED set requests */
    move->leds.type = PSMove_Req_SetLEDs;

    /* Remember the ID/index */
    move->id = id;

    return move;
}

PSMove *
psmove_connect_by_id(int id)
{
    struct hid_device_info *devs, *cur_dev;
    int count = 0;
    PSMove *move = NULL;

    devs = hid_enumerate(PSMOVE_VID, PSMOVE_PID);
    cur_dev = devs;
    while (cur_dev) {
        if (count == id) {
            move = psmove_connect_internal(cur_dev->serial_number,
                    cur_dev->path, id);
            break;
        }
        count++;
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    return move;
}


PSMove *
psmove_connect()
{
    return psmove_connect_internal(NULL, NULL, 0);
}

int
psmove_get_btaddr(PSMove *move, PSMove_Data_BTAddr *addr)
{
    unsigned char cal[PSMOVE_CALIBRATION_SIZE];
    unsigned char btg[PSMOVE_BTADDR_GET_SIZE];
    int res;
    int i;

    psmove_return_val_if_fail(move != NULL, 0);

    /* Get calibration data */
    memset(cal, 0, sizeof(cal));
    cal[0] = PSMove_Req_GetCalibration;
    res = hid_get_feature_report(move->handle, cal, sizeof(cal));
    if (res < 0) {
        return 0;
    }

    /* Get Bluetooth address */
    memset(btg, 0, sizeof(btg));
    btg[0] = PSMove_Req_GetBTAddr;
    res = hid_get_feature_report(move->handle, btg, sizeof(btg));

    if (res == sizeof(btg)) {
#ifdef PSMOVE_DEBUG
        fprintf(stderr, "[PSMOVE] controller bt mac addr: ");
        for (i=6; i>=1; i--) {
            if (i != 6) putc(':', stderr);
            fprintf(stderr, "%02x", btg[i]);
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "[PSMOVE] current host bt mac addr: ");
        for (i=15; i>=10; i--) {
            if (i != 15) putc(':', stderr);
            fprintf(stderr, "%02x", btg[i]);
        }
        fprintf(stderr, "\n");
#endif
        if (addr != NULL) {
            /* Copy 6 bytes (btg[10]..btg[15]) into addr, reversed */
            for (i=0; i<6; i++) {
                (*addr)[i] = btg[15-i];
            }
            memcpy(addr, btg+10, 6);
        }

        /* Success! */
        return 1;
    }

    return 0;
}

int
psmove_controller_btaddr(PSMove *move, PSMove_Data_BTAddr *addr)
{
    psmove_return_val_if_fail(move != NULL, 0);
    psmove_return_val_if_fail(addr != NULL, 0);

    memcpy(addr, move->btaddr, sizeof(PSMove_Data_BTAddr));

    return 1;
}

int
psmove_set_btaddr(PSMove *move, PSMove_Data_BTAddr *addr)
{
    unsigned char bts[PSMOVE_BTADDR_SET_SIZE];
    int res;
    int i;

    psmove_return_val_if_fail(move != NULL, 0);
    psmove_return_val_if_fail(addr != NULL, 0);

    /* Get calibration data */
    memset(bts, 0, sizeof(bts));
    bts[0] = PSMove_Req_SetBTAddr;

    /* Copy 6 bytes from addr into bts[1]..bts[6], reversed */
    for (i=0; i<6; i++) {
        bts[1+5-i] = (*addr)[i];
    }

    res = hid_send_feature_report(move->handle, bts, sizeof(bts));

    return (res == sizeof(bts));
}

int
psmove_pair(PSMove *move)
{
    psmove_return_val_if_fail(move != NULL, 0);

    PSMove_Data_BTAddr btaddr;
    int i;

    if (!psmove_get_btaddr(move, &btaddr)) {
        return 0;
    }

#if defined(__APPLE__)
    BluetoothDeviceAddress address;
    IOReturn result = kIOReturnError;
   
   assert(false); //this API is removed, but also we seem to never call psmove_pair()
    //result = IOBluetoothLocalDeviceReadAddress(&address, NULL, NULL, NULL);

    if (result != kIOReturnSuccess) {
        return 0;
    }

    for (i=0; i<6; i++) {
        btaddr[i] = address.data[i];
    }
   
   printf("******ADDRESS %x %x %x %x %x %x",address.data[0],address.data[1],address.data[2],address.data[3],address.data[4],address.data[5]);
   

#elif defined(__linux)
    hci_for_each_dev(HCI_UP, _psmove_linux_bt_dev_info, (long)btaddr);
#else
    /* TODO: Implement for Windows and other OSes */
    return 0;
#endif

    if (!psmove_set_btaddr(move, &btaddr)) {
        return 0;
    }

    return 1;
}

enum PSMove_Connection_Type
psmove_connection_type(PSMove *move)
{
    wchar_t wstr[255];
    int res;

    psmove_return_val_if_fail(move != NULL, Conn_Unknown);

    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(move->handle,
            wstr, sizeof(wstr)/sizeof(wstr[0]));

    /**
     * As it turns out, we don't get a serial number when connected via USB,
     * so assume that when the serial number length is zero, then we have the
     * USB connection type, and if we have a greater-than-zero length, then it
     * is a Bluetooth connection.
     **/
    if (res == 0) {
        return Conn_USB;
    } else if (res > 0) {
        return Conn_Bluetooth;
    }

    return Conn_Unknown;
}

void
psmove_set_leds(PSMove *move, unsigned char r, unsigned char g,
        unsigned char b)
{
    psmove_return_if_fail(move != NULL);
    move->leds.r = r;
    move->leds.g = g;
    move->leds.b = b;
}

void
psmove_set_rumble(PSMove *move, unsigned char rumble)
{
    psmove_return_if_fail(move != NULL);
    move->leds.rumble2 = 0x00;
    move->leds.rumble = rumble;
}

int
psmove_update_leds(PSMove *move)
{
    int res;

    psmove_return_val_if_fail(move != NULL, 0);

    res = hid_write(move->handle, (unsigned char*)(&(move->leds)),
            sizeof(move->leds));
    return (res == sizeof(move->leds));
}

int
psmove_poll(PSMove *move)
{
    int res;

    psmove_return_val_if_fail(move != NULL, 0);

#ifdef PSMOVE_DEBUG
    /* store old sequence number before reading */
    int oldseq = (move->input.buttons4 & 0x0F);
#endif

    res = hid_read(move->handle, (unsigned char*)(&(move->input)),
        sizeof(move->input));

    if (res == sizeof(move->input)) {
        /* Sanity check: The first byte should be PSMove_Req_GetInput */
        psmove_return_val_if_fail(move->input.type == PSMove_Req_GetInput, 0);

        /**
         * buttons4's 4 least significant bits contain the sequence number,
         * so we add 1 to signal "success" and add the sequence number for
         * consumers to utilize the data
         **/
#ifdef PSMOVE_DEBUG
        int seq = (move->input.buttons4 & 0x0F);
        if (seq != ((oldseq + 1) % 16)) {
            fprintf(stderr, "[PSMOVE] Warning: Dropped frames (seq %d -> %d)\n",
                    oldseq, seq);
        }
#endif
        return 1 + (move->input.buttons4 & 0x0F);
    }

    return 0;
}

unsigned int
psmove_get_buttons(PSMove *move)
{
    psmove_return_val_if_fail(move != NULL, 0);

    return ((move->input.buttons2) |
            (move->input.buttons1 << 8) |
            ((move->input.buttons3 & 0x01) << 16) |
            ((move->input.buttons4 & 0xF0) << 13));
}

unsigned char
psmove_get_battery(PSMove *move)
{
    psmove_return_val_if_fail(move != NULL, 0);

    return move->input.battery;
}

int
psmove_get_temperature(PSMove *move)
{
    psmove_return_val_if_fail(move != NULL, 0);

    return ((move->input.templow << 4) |
            ((move->input.temphigh & 0xF0) >> 4));
}

unsigned char
psmove_get_trigger(PSMove *move)
{
    psmove_return_val_if_fail(move != NULL, 0);

    return move->input.trigger;
}

void
psmove_get_accelerometer(PSMove *move, int *ax, int *ay, int *az)
{
    psmove_return_if_fail(move != NULL);

    if (ax != NULL) {
        *ax = ((move->input.aXlow + move->input.aXlow2) +
               ((move->input.aXhigh + move->input.aXhigh2) << 8)) / 2 - 0x8000;
    }

    if (ay != NULL) {
        *ay = ((move->input.aYlow + move->input.aYlow2) +
               ((move->input.aYhigh + move->input.aYhigh2) << 8)) / 2 - 0x8000;
    }

    if (az != NULL) {
        *az = ((move->input.aZlow + move->input.aZlow2) +
               ((move->input.aZhigh + move->input.aZhigh2) << 8)) / 2 - 0x8000;
    }
}

void
psmove_get_gyroscope(PSMove *move, int *gx, int *gy, int *gz)
{
    psmove_return_if_fail(move != NULL);

    if (gx != NULL) {
        *gx = ((move->input.gXlow + move->input.gXlow2) +
               ((move->input.gXhigh + move->input.gXhigh2) << 8)) / 2 - 0x8000;
    }

    if (gy != NULL) {
        *gy = ((move->input.gYlow + move->input.gYlow2) +
               ((move->input.gYhigh + move->input.gYhigh2) << 8)) / 2 - 0x8000;
    }

    if (gz != NULL) {
        *gz = ((move->input.gZlow + move->input.gZlow2) +
               ((move->input.gZhigh + move->input.gZhigh2) << 8)) / 2 - 0x8000;
    }
}

void
psmove_get_magnetometer(PSMove *move, int *mx, int *my, int *mz)
{
    psmove_return_if_fail(move != NULL);

    if (mx != NULL) {
        *mx = move->input.templow << 0x0C | move->input.temphigh << 0x04;
    }

    if (my != NULL) {
        *my = move->input.mag40 << 0x08 | (move->input.mag41 & 0xF0);
    }

    if (mz != NULL) {
        *mz = move->input.mag41 << 0x0C | move->input.mag42 << 0x0f;
    }
}

void
psmove_disconnect(PSMove *move)
{
    psmove_return_if_fail(move != NULL);
    free(move);
}

