#ifndef __RF_IO_H__
#define __RF_IO_H__

#include "nRF24L01.h"
#include "RF24.h"

typedef enum {stand_alone=0, first_packet=1, continued_packet=2, last_packet=3} packet_type_e;

bool handle_read(RF24 *radio, uint8_t* buf, unsigned int len, uint8_t* data, unsigned int* data_len);
bool handle_write(RF24 *radio, void* buf, unsigned int len, int repeat);

#endif
