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

bool _send(RF24 *radio, void* buf, unsigned int len, int repeat)
{
		int i = 0;
		bool success = false;
        /*printf("sending message, length: %d content:\n", len);
        for(int i=0;i<len+1;i++) {
            printf("0x%X ", buf);
        }
        printf("\n");*/
		while(!success && (i++ < repeat)) {
			success = radio->write(buf, len);
		}
		return success;
}

bool handle_write(RF24 *radio, void* buf, unsigned int len, int repeat){
    uint8_t tx_buf[32];
	uint8_t* cur_ptr = NULL;
	uint8_t ps = radio->getPayloadSize();
	bool success = false;
    if(len<ps) {
        tx_buf[0] = (stand_alone << 6) | (len+1);
        memcpy(tx_buf+1, buf, len);
        radio->stopListening();
		success = _send(radio, tx_buf, len+1, repeat);
        radio->startListening();
        if(success) {
            Serial.println("send single packet succeded");
            return true;
        }
		Serial.println("send single packet failed");
        return false;
    }
	else {
		uin8_t remain_size = len;
        tx_buf[0] = first_packet << 6;
        tx_buf[0] |= ps;
		*((uint16_t*)(tx_buf+1)) = len;
		memcpy(tx_buf+3, buf, ps-3);
        radio->stopListening();
		success = _send(radio, tx_buf, len+1, repeat);
        if(!success) {
			Serial.println("first packet failed");
			radio->startListening();
			return false;
		}
		remain_size -= ps-3;
		cur_ptr = buf + ps - 3;
		while(remain_size > ps-1 && success) {
			tx_buf[0] = continued_packet << 6;
			tx_buf[0] |= ps;
			memcpy(tx_buf+1, cur_ptr, ps-1);
			cur_ptr += ps-1;
			remain_size -= ps-1;
			success = _send(radio, tx_buf, ps, repeat);
		}	
        if(!success) {
			Serial.println("continued packet failed");
			radio->startListening();
			return false;
		}
        tx_buf[0] = (last_packet << 6) | (remain_size+1);
        memcpy(tx_buf+1, cur_ptr, remain_size);
		success = _send(radio, tx_buf, remain_size+1, repeat);
        if(!success) {
			Serial.println("last packet failed");
		}
		else {
		    Serial.println("send multi packet succeded");
		}
		radio->startListening();
		return success;
	}
    return false;
}
