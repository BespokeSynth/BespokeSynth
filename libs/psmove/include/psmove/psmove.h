
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

#ifndef __PSMOVE_H
#define __PSMOVE_H

#ifdef __cplusplus
extern "C" {
#endif

enum PSMove_Connection_Type {
    Conn_Bluetooth,
    Conn_USB,
    Conn_Unknown,
};

enum PSMove_Button {
    Btn_L2 = 1 << 0x00,
    Btn_R2 = 1 << 0x01,
    Btn_L1 = 1 << 0x02,
    Btn_R1 = 1 << 0x03,
    Btn_TRIANGLE = 1 << 0x04,
    Btn_CIRCLE = 1 << 0x05,
    Btn_CROSS = 1 << 0x06,
    Btn_SQUARE = 1 << 0x07,
    Btn_SELECT = 1 << 0x08,
    Btn_L3 = 1 << 0x09,
    Btn_R3 = 1 << 0x0A,
    Btn_START = 1 << 0x0B,
    Btn_UP = 1 << 0x0C,
    Btn_RIGHT = 1 << 0x0D,
    Btn_DOWN = 1 << 0x0E,
    Btn_LEFT = 1 << 0x0F,
    Btn_PS = 1 << 0x10,

    Btn_MOVE = 1 << 0x13,
    Btn_T = 1 << 0x14,
};

enum PSMove_Battery_Level {
    Batt_MIN = 0x00,
    Batt_MAX = 0x05,
    Batt_CHARGING = 0xEE,
};

/* A Bluetooth address. */
typedef unsigned char PSMove_Data_BTAddr[6];

/* Opaque data type for the PS Move internal data */
struct _PSMove;
typedef struct _PSMove PSMove;

/**
 * Get the number of currently-connected PS Move controllers
 **/
int
psmove_count_connected(void);

/**
 * Connect to the PS Move controller id (zero-based index)
 * Returns: A newly-allocated PSMove structure or NULL on error
 **/
PSMove *
psmove_connect_by_id(int id);

/**
 * Connect to the default PS Move controller
 * Returns: A newly-allocated PSMove structure or NULL on error
 **/
PSMove *
psmove_connect(void);

/**
 * Determine the connection type of the controllerj
 * Returns: An enum PSMove_Connection_Type value
 **/
enum PSMove_Connection_Type
psmove_connection_type(PSMove *move);

/**
 * Get the currently-set Host Bluetooth address that is used
 * to connect via Bluetooth when the PS button is pressed.
 *
 * addr might be NULL in which case the address and calibration
 * data are retrieved, but the Bluetooth address is discarded.
 **/
int
psmove_get_btaddr(PSMove *move, PSMove_Data_BTAddr *addr);

/**
 * Get the Bluetooth Mac address of the connected controller.
 *
 * XXX This is not implemented in the backend at the moment.
 **/
int
psmove_controller_btaddr(PSMove *move, PSMove_Data_BTAddr *addr);

/**
 * Set the Host Bluetooth address that is used to connect via
 * Bluetooth. You should set this to the local computer's
 * Bluetooth address when connected via USB, then disconnect
 * and press the PS button to connect the controller via BT.
 **/
int
psmove_set_btaddr(PSMove *move, PSMove_Data_BTAddr *addr);

/**
 * Set the Host Bluetooth address of the PS Move to this
 * computer's Bluetooth address. Only works via USB.
 *
 * Only implemented for OS X for now.
 *
 * Will return nonzero on success, zero on error.
 **/
int
psmove_pair(PSMove *move);

/**
 * Set the LEDs of the PS Move controller. You need to
 * call PSMove_update_leds() to send the update to the
 * controller.
 **/
void
psmove_set_leds(PSMove *move, unsigned char r, unsigned char g,
        unsigned char b);

/**
 * Set the rumble value of the PS Move controller. You
 * need to call PSMove_update_leds() to send the update
 * to the controller.
 **/
void
psmove_set_rumble(PSMove *move, unsigned char rumble);

/**
 * Re-send the LED and Rumble status bits. This needs to
 * be done regularly to keep the LEDs and rumble turned on.
 *
 * Returns: Nonzero on success, zero on error
 **/
int
psmove_update_leds(PSMove *move);

/**
 * Polls the PS Move for new sensor/button data.
 * Returns a positive number (sequence number + 1) if new data is
 * available or zero if no data is available.
 **/
int
psmove_poll(PSMove *move);

/**
 * Get the current status of the PS Move buttons. You need to call
 * PSMove_poll() to read new data from the controller first.
 **/
unsigned int
psmove_get_buttons(PSMove *move);

/**
 * Get the battery level of the PS Move. You need to call
 * PSMove_poll() to read new data from the controller first.
 *
 * Return value range: Batt_MIN..Batt_MAX
 * Charging (via USB): Batt_CHARGING
 **/
unsigned char
psmove_get_battery(PSMove *move);

/**
 * Get the current temperature of the PS Move. You need to
 * call PSMove_poll() to read new data from the controller first.
 *
 * Return value range: FIXME
 **/
int
psmove_get_temperature(PSMove *move);

/**
 * Get the current value of the PS Move analog trigger. You need to
 * call PSMove_poll() to read new data from the controller first.
 **/
unsigned char
psmove_get_trigger(PSMove *move);

/**
 * Get the current accelerometer readings from the PS Move. You need
 * to call PSMove_poll() to read new data from the controller first.
 *
 * ax, ay and az should be pointers to integer locations that you want
 * to have filled with values. If you don't care about one of these
 * values, simply pass NULL and the field will be ignored..
 **/
void
psmove_get_accelerometer(PSMove *move, int *ax, int *ay, int *az);

/**
 * Same as PSMove_get_accelerometer(), but for the gyroscope.
 **/
void
psmove_get_gyroscope(PSMove *move, int *gx, int *gy, int *gz);

/**
 * Same as PSMove_get_accelerometer(), but for the magnetometer.
 * See http://youtu.be/ltOQB_q1UTg for calibration instructions.
 * This is not really tested. YMMV.
 **/
void
psmove_get_magnetometer(PSMove *move, int *mx, int *my, int *mz);

/**
 * Disconnect from the PS Move and free resources
 **/
void
psmove_disconnect(PSMove *move);

#ifdef __cplusplus
}
#endif

#endif
