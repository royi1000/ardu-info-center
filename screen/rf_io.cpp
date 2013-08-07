#include "rf_io.h"

unsigned int  current_ptr = 0;

bool handle_read(RF24 *radio, uint8_t* buf, unsigned int len, uint8_t* data, unsigned int* data_len){
    uint8_t msg_type = buf[0] >> 6;
    uint8_t msg_len = buf[0] & 63;
    if(stand_alone==msg_type){
        memcpy(data, buf+1, msg_len-1);
        *data_len = msg_len-1;
        current_ptr = 0;
        return true;
    }
    else if(first_packet==msg_type) {
        *data_len = *((uint16_t*)(buf+1));
        memcpy(data, buf+3, msg_len-3);
        current_ptr = msg_len-3;
        //printf("got first message, total len: %d\n", data_len);
        return false;
    }
    else if(continued_packet==msg_type) {
        if(!current_ptr) {
            //printf("got invalid continued message\n");
        }
        memcpy(data+current_ptr, buf+1, msg_len-1);
        current_ptr += msg_len-1;
        if(current_ptr > *data_len) {
            // printf("invalid continued message, ptr overflow\n");
        }
        return false;
    }
    else if(last_packet==msg_type) {
        if(!current_ptr) {
            //printf("got invalid last message\n");
            return false;
        }
        memcpy(data+current_ptr, buf+1, msg_len-1);
        current_ptr += msg_len-1;
        if(current_ptr != *data_len) {
            //printf("invalid ptr: %d, data len: %d\n", current_ptr, data_len);
            return false;
        }
        current_ptr = 0;
        return true;
    }
    return false;
}

bool handle_write(RF24 *radio, void* buf, unsigned int len, int repeat){
    uint8_t tx_buf[32];
    if(len<radio->getPayloadSize()) {
        tx_buf[0] = (stand_alone << 6) | (len+1);
        memcpy(tx_buf+1, buf, len);
        //printf("sending message, length: %d content:\n", len);
        for(int i=0;i<len+1;i++) {
            //printf("0x%X ", tx_buf[i]);
        }
        //printf("\n");
        radio->stopListening();
		int i = 0;
		bool success = false;
		while(!success && (i++ < repeat)) {
			success = radio->write(tx_buf, len+1);
		}
        radio->startListening();
        if(success) {
            Serial.println("send single packet succeded");
            return true;
        }
		Serial.println("send single packet failed");
        return false;
    }
	Serial.println("packet too big");
    return false;
}


