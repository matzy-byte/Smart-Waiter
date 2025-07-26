#include <Arduino.h>
#include <Microphone.h>
#include <Speaker.h>
#include <ModelInference.h>

#define SAMPLE_RATE 16000
#define SAMPLE_COUNT (SAMPLE_RATE * 1)

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupMicrophone();
    setupSpeaker();
    setupModel();

    delay(1000);

    Serial.println("System ready. Listening for \"juan\"...");
}

void loop() {
    readMicrophoneSamples(rawSamples, SAMPLE_COUNT);
    downsample(rawSamples, processesSamples);
    
    if (runInference(processesSamples)) {
        playMelody();
        delay(5000);
    }
    delay(100);
}