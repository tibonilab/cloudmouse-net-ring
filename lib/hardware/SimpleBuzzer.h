/**
 * CloudMouse SDK - Simple Buzzer Manager
 *
 * Hardware abstraction for piezo buzzer audio feedback with PWM control.
 * Provides pre-configured sound patterns for common user interactions and system events.
 *
 * Features:
 * - Software PWM generation for precise frequency control
 * - Pre-defined sound patterns for success, error, and notification events
 * - Configurable frequency, duration, and duty cycle parameters
 * - Non-blocking sound generation with microsecond timing precision
 * - Low-level GPIO control for compatibility with various buzzer types
 * - Dedicated FreeRTOS task for smooth alarm playback
 *
 * Hardware Requirements:
 * - Piezo buzzer or passive speaker connected to GPIO pin 14
 * - Optional: Current limiting resistor (100-330Ω) for protection
 *
 * Usage:
 * 1. Call init() during system initialization
 * 2. Call startTask() to launch dedicated buzzer task
 * 3. Use buzz() for positive feedback (button clicks, confirmations)
 * 4. Use error() for negative feedback (failures, warnings)
 * 5. Use alarmStart()/alarmStop() for continuous alarm patterns
 */

#ifndef SIMPLE_BUZZER_H
#define SIMPLE_BUZZER_H

#include <Arduino.h>

// Hardware configuration
#define BUZZER_PIN 14

namespace CloudMouse::Hardware
{
    class SimpleBuzzer
    {
    private:
      inline static bool alarmActive = false;
      inline static unsigned long lastAlarmTime = 0;
      inline static int alarmStep = 0;
      inline static TaskHandle_t buzzerTaskHandle = NULL;

        /**
         * Buzzer task handler (runs on dedicated core)
         * Manages alarm sound generation without blocking main loop
         */
        static void buzzerTask(void* parameter)
        {
            while(true) {
                update();
                vTaskDelay(pdMS_TO_TICKS(10)); // 10ms tick
            }
        }

        /**
         * Update alarm state (internal, called by task)
         * Handles active alarm sound generation (non-blocking)
         */
        static void update()
        {
            if (!alarmActive) {
                alarmStep = 0;
                return;
            }
            
            unsigned long now = millis();
            unsigned long elapsed = now - lastAlarmTime;
            
            switch(alarmStep) {
                case 0: // bi
                    if (elapsed >= 0) {
                        buzzWithPWM(1000, 150, 15);
                        lastAlarmTime = now;
                        alarmStep++;
                    }
                    break;
                case 1: // pausa
                    if (elapsed >= 250) {
                        alarmStep++;
                    }
                    break;
                case 2: // bi
                    if (elapsed >= 250) {
                        buzzWithPWM(1000, 150, 15);
                        lastAlarmTime = now;
                        alarmStep++;
                    }
                    break;
                case 3: // pausa
                    if (elapsed >= 250) {
                        alarmStep++;
                    }
                    break;
                case 4: // bi
                    if (elapsed >= 250) {
                        buzzWithPWM(1000, 150, 15);
                        lastAlarmTime = now;
                        alarmStep++;
                    }
                    break;
                case 5: // pausa
                    if (elapsed >= 250) {
                        alarmStep++;
                    }
                    break;
                case 6: // biiiiip
                    if (elapsed >= 250) {
                        buzzWithPWM(1000, 400, 15);
                        lastAlarmTime = now;
                        alarmStep++;
                    }
                    break;
                case 7: // pausa lunga
                    if (elapsed >= 1200) {
                        alarmStep = 0; // ricomincia
                    }
                    break;
            }
        }

    public:
        /**
         * Initialize buzzer hardware
         * Configures GPIO pin as output for buzzer control
         * Call once during system setup
         */
        static void init()
        {
            pinMode(BUZZER_PIN, OUTPUT);
            digitalWrite(BUZZER_PIN, LOW);
        }

        /**
         * Start dedicated buzzer task
         * Launches FreeRTOS task on Core 1 for alarm management
         * Call after init() during system setup
         */
        static void startTask()
        {
            xTaskCreatePinnedToCore(
                buzzerTask,           // Task function
                "BuzzerTask",         // Task name
                2048,                 // Stack size
                NULL,                 // Parameters
                1,                    // Priority
                &buzzerTaskHandle,    // Task handle
                1                     // Core 1
            );
        }

        /**
         * Start alarm sound pattern
         * Continuous alternating tone loop until stopped
         * Use for: emergency alerts, critical notifications
         * Call alarmStop() to silence
         */
        static void alarmStart()
        {
            alarmActive = true;
        }

        /**
         * Stop alarm sound
         * Silences any active alarm pattern
         */
        static void alarmStop()
        {
            alarmActive = false;
            silence();
        }

        /**
         * Play positive feedback sound pattern
         * Three-tone ascending sequence for success events
         * Use for: button confirmations, successful operations, positive notifications
         *
         * Pattern: High → Low → Medium frequency tones
         * Total duration: ~225ms
         */
        static void buzz()
        {
            buzzWithPWM(740, 75, 20);
            buzzWithPWM(120, 75, 20);
            buzzWithPWM(270, 75, 20);
        }

        /**
         * Play error/warning sound pattern
         * Alternating tone sequence for attention-getting alerts
         * Use for: errors, warnings, failed operations, critical notifications
         *
         * Pattern: Alternating mid-low frequency tones (6 tones total)
         * Total duration: ~450ms
         */
        static void error()
        {
            buzzWithPWM(230, 75, 20);
            buzzWithPWM(120, 75, 20);
            buzzWithPWM(230, 75, 20);
            buzzWithPWM(120, 75, 20);
            buzzWithPWM(230, 75, 20);
            buzzWithPWM(120, 75, 20);
        }

        /**
         * Generate custom PWM tone with precise control
         * Creates square wave signal with specified parameters
         *
         * @param frequency Tone frequency in Hz (50-5000 recommended range)
         * @param duration Sound duration in milliseconds
         * @param dutyCycle PWM duty cycle percentage (1-99, typical: 10-50)
         *
         * Technical notes:
         * - Uses software PWM for precise timing control
         * - Higher duty cycles = louder volume but more power consumption
         * - Lower frequencies = deeper tones, higher frequencies = sharper tones
         * - Function blocks execution for specified duration
         */
        static void buzzWithPWM(int frequency, int duration, int dutyCycle)
        {
            int period = 1000000 / frequency;
            int pulse = (period * dutyCycle) / 100;

            unsigned long startTime = millis();
            while (millis() - startTime < duration)
            {
                digitalWrite(BUZZER_PIN, HIGH);
                delayMicroseconds(pulse);
                digitalWrite(BUZZER_PIN, LOW);
                delayMicroseconds(period - pulse);
            }
        }

        /**
         * Silence buzzer immediately
         * Ensures buzzer output is low (silent state)
         * Use for emergency stop or cleanup
         */
        static void silence()
        {
            digitalWrite(BUZZER_PIN, LOW);
        }

        /**
         * Play single short beep
         * Quick notification sound for minimal feedback
         * Use for: key presses, menu navigation, minor alerts
         *
         * @param frequency Optional frequency in Hz (default: 1000Hz)
         * @param duration Optional duration in ms (default: 100ms)
         */
        static void beep(int frequency = 1000, int duration = 100)
        {
            buzzWithPWM(frequency, duration, 25);
        }

        /**
         * Play double beep pattern
         * Two quick beeps for intermediate feedback
         * Use for: mode changes, setting confirmations, intermediate states
         */
        static void doubleBeep()
        {
            beep(800, 80);
            delay(50);
            beep(800, 80);
        }
    };
};

#endif