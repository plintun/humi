// Paing Lin Tun  lab 6 Spring 2025
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_TIME 85
#define DHT11PIN 7

int dht11_val[5] = {0, 0, 0, 0, 0};

int LCDAddr = 0x27;
int BLEN = 1;
int fd;

void write_word(int data){
    int temp = data;
    if (BLEN == 1)
        temp |= 0x08;
    else
        temp &= 0xF7;
    wiringPiI2CWrite(fd, temp);
}

void send_command(int comm){
    int buf;
    buf = comm & 0xF0;
    buf |= 0x04;
    write_word(buf);
    delay(2);
    buf &= 0xFB;
    write_word(buf);

    buf = (comm & 0x0F) << 4;
    buf |= 0x04;
    write_word(buf);
    delay(2);
    buf &= 0xFB;
    write_word(buf);
}

void send_data(int data){
    int buf;
    buf = data & 0xF0;
    buf |= 0x05;
    write_word(buf);
    delay(2);
    buf &= 0xFB;
    write_word(buf);

    buf = (data & 0x0F) << 4;
    buf |= 0x05;
    write_word(buf);
    delay(2);
    buf &= 0xFB;
    write_word(buf);
}

void init(){
    send_command(0x33);
    delay(5);
    send_command(0x32);
    delay(5);
    send_command(0x28);
    delay(5);
    send_command(0x0C);
    delay(5);
    send_command(0x01);
    wiringPiI2CWrite(fd, 0x08);
}

void clear(){
    send_command(0x01);
}

void write(int x, int y, const char data[]){
    int addr, i;
    if (x < 0) x = 0;
    if (x > 15) x = 15;
    if (y < 0) y = 0;
    if (y > 1) y = 1;

    addr = 0x80 + 0x40 * y + x;
    send_command(addr);

    for (i = 0; i < strlen(data); i++){
        send_data(data[i]);
    }
}

void dht11_read_val(){
    uint8_t lststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float farenheit;
    char buffer[16];

    for(i = 0; i < 5; i++)
        dht11_val[i] = 0;

    pinMode(DHT11PIN, OUTPUT);
    digitalWrite(DHT11PIN, LOW);
    delay(18);
    digitalWrite(DHT11PIN, HIGH);
    delayMicroseconds(40);

    pinMode(DHT11PIN, INPUT);

    for(i = 0; i < MAX_TIME; i++){
        counter = 0;
        while(digitalRead(DHT11PIN) == lststate) {
            counter++;
            delayMicroseconds(1);
            if(counter == 255)
                break;
        }
        lststate = digitalRead(DHT11PIN);
        if(counter == 255)
            break;

        if((i >= 4) && (i % 2 == 0)) {
            dht11_val[j / 8] <<= 1;
            if(counter > 16)
                dht11_val[j / 8] |= 1;
            j++;
        }
    }


    int hour = rand() %24;
    int minute = rand()%60;
    char str[7];

    if((j >= 40) && (dht11_val[4] == ((dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3]) & 0xFF))) {
        farenheit = dht11_val[2] * 9.0 / 5.0 + 32;


    write(0, 0 , "temp");
    write(6, 0 , "hum");
    write(11, 0 , "time");

        snprintf(buffer, sizeof(buffer), "%-6.1fF",  farenheit);
        write(0, 1, str);

        snprintf(buffer, sizeof(buffer), "%d.%d %%", dht11_val[0], dht11_val[1]);
        write(6, 1, str);

        sprintf(str, "%02d:%02d", hour, minute);
        write(11, 1, str);

    } else {
        write(0, 1, "InvalidData");
    }
}

int main(void){

    if (wiringPiSetup() == -1) exit(1);

    fd = wiringPiI2CSetup(LCDAddr);
    init();

    while(1){
        dht11_read_val();
        delay(3000);
    }

    return 0;
}
