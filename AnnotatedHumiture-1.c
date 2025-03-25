#include <wiringPi.h>  // Include the WiringPi library for GPIO control
#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Standard library functions
#include <stdint.h>     // Standard integer types

#define MAX_TIME 85    // Maximum time to wait for sensor response
#define DHT11PIN 7     // GPIO pin number where DHT11 is connected

int dht11_val[5] = {0, 0, 0, 0, 0}; // Array to store sensor data

// Function to read values from the DHT11 sensor
void dht11_read_val()
{
    uint8_t lststate = HIGH; // Variable to track the last state of the pin
    uint8_t counter = 0;     // Counter for timing the signal transitions
    uint8_t j = 0, i;        // Index variables
    float farenheit;         // Variable to store temperature in Fahrenheit

    // Initialize the data array
    for(i = 0; i < 5; i++)
        dht11_val[i] = 0;

    // Send start signal to the DHT11 sensor
    pinMode(DHT11PIN, OUTPUT);
    digitalWrite(DHT11PIN, LOW);
    delay(18);  // Keep the pin LOW for at least 18ms to trigger the sensor
    digitalWrite(DHT11PIN, HIGH);
    delayMicroseconds(40); // Wait for 40µs before switching to input mode

    pinMode(DHT11PIN, INPUT); // Set pin as input to read data

    // i keeps track of state changes (HIGH/LOW transitions).
    // Each state transition (LOW ? HIGH or HIGH ? LOW) moves i to the next iteration.
    // This allows the program to track HIGH pulse durations, which determine whether each data bit is a 0 or 1.
    // Read data from the sensor
    // i >= 4 ? Ignores the first 4 transitions (handshake signal).
    // i % 2 == 0 ? Ensures we only process the HIGH pulses.
    // counter > 16 ? If the HIGH pulse lasted longer than ~50µs, it's a 1, otherwise it's a 0.
    // dht11_val[j / 8] <<= 1 ? Shifts previous bits left to make space for the new bit.
    // dht11_val[j / 8] |= 1 ? If it’s a 1, set the least significant bit.
    for(i = 0; i < MAX_TIME; i++)
    {
        counter = 0;
        // Count the duration of the signal state
        while(digitalRead(DHT11PIN) == lststate) {
            counter++;
            delayMicroseconds(1);
            if(counter == 255) // Break if counter overflows
                break;
        }

        lststate = digitalRead(DHT11PIN); // Store the current state

        if(counter == 255) // If we reached max counter, exit the loop
            break;

        // Ignore the first three state transitions (start signal)
        if((i >= 4) && (i % 2 == 0)) {
            dht11_val[j / 8] <<= 1; // Shift left to make room for new bit
            if(counter > 16)        // If signal is longer than 16µs, it's a '1'
                dht11_val[j / 8] |= 1;
            j++;
        }
    }

    // Verify checksum to ensure data integrity
    if((j >= 40) && (dht11_val[4] == ((dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3]) & 0xFF)))
    {
        // Convert temperature to Fahrenheit
        farenheit = dht11_val[2] * 9.0 / 5.0 + 32;
        // Print the humidity and temperature data
        printf("Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n", 
               dht11_val[0], dht11_val[1], dht11_val[2], dht11_val[3], farenheit);
    }
    else
    {
        printf("Invalid Data!!\n"); // Print an error if checksum fails
    }
}

int main(void)
{
    // Print an introductory message
    printf("Interfacing Temperature and Humidity Sensor (DHT11) With Raspberry Pi\n");

    // Initialize WiringPi library for GPIO control
    // If initialization fails, exit the program
    if (wiringPiSetup() == -1)
        exit(1);

    // Continuous loop to read sensor data at intervals
    while (1)
    {
        // Call the function to read values from the DHT11 sensor
        dht11_read_val();

        // Wait for 3 seconds before the next reading to avoid excessive polling
        delay(3000);
    }

    return 0; // Return 0 to indicate normal program termination
}

