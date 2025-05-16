#define ANALOG_CH0 A0
const int numReadings = 10; // Number of readings for moving average
float readings[numReadings]; // Array to store readings
int readIndex = 0; // Current index in the array
float total = 0; // Sum of readings
float movingAverage = 0; // Moving average

void setup() {
    Serial.begin(9600); // Initialize serial communication
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0; // Initialize readings array
    }
}


void loop() {
    // Read the analog input
    int adc_value = analogRead(ANALOG_CH0);
    Serial.print("Analog_value:");
    Serial.print(adc_value);
    Serial.print(",");
    
    float analogValue = (float) adc_value * 5.0 / 1024.0; // Convert to voltage

    // Update the total by subtracting the oldest reading
    total -= readings[readIndex];

    // Store the new reading in the array
    readings[readIndex] = analogValue;

    // Add the new reading to the total
    total += analogValue;

    // Advance to the next index
    readIndex = (readIndex + 1) % numReadings;

    // Calculate the moving average
    movingAverage = total / numReadings;

    // Print the current reading and moving average
    Serial.print("Voltage_value:");
    Serial.print(analogValue);
    Serial.print(",");
    Serial.print("Moving_average:");
    Serial.println(movingAverage);

    delay(100); // Delay for stability
}