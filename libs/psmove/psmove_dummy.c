#include "psmove/psmove.h"

#include <stddef.h>

int psmove_count_connected(void) { return 0; }

PSMove *psmove_connect_by_id(int id) { return NULL; }

PSMove *psmove_connect(void) { return NULL; }

enum PSMove_Connection_Type psmove_connection_type(PSMove *move) { return Conn_Unknown; }

int psmove_get_btaddr(PSMove *move, PSMove_Data_BTAddr *addr) { return 0; }

int psmove_controller_btaddr(PSMove *move, PSMove_Data_BTAddr *addr) { return 0; }

int psmove_set_btaddr(PSMove *move, PSMove_Data_BTAddr *addr) { return 0; }

int psmove_pair(PSMove *move) { return 0; }

void psmove_set_leds(PSMove *move, unsigned char r, unsigned char g, unsigned char b) {}

void psmove_set_rumble(PSMove *move, unsigned char rumble) {}

int psmove_update_leds(PSMove *move) { return 0; }

int psmove_poll(PSMove *move) { return 0; }

unsigned int psmove_get_buttons(PSMove *move) { return 0; }

unsigned char psmove_get_battery(PSMove *move) { return 0; }

int psmove_get_temperature(PSMove *move) { return 0; }

unsigned char psmove_get_trigger(PSMove *move) { return 0; }

void psmove_get_accelerometer(PSMove *move, int *ax, int *ay, int *az) {}

void psmove_get_gyroscope(PSMove *move, int *gx, int *gy, int *gz) {}

void psmove_get_magnetometer(PSMove *move, int *mx, int *my, int *mz) {}

void psmove_disconnect(PSMove *move) {}
