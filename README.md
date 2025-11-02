# CloudMouse Emergency Alert System

A bidirectional emergency alert system built on CloudMouse SDK enabling reliable communication between two ESP32 devices over LAN using HTTP and mDNS.

## Overview

This system allows a person (sender) to trigger an alert on another device (receiver) with visual and audio feedback on both ends, ensuring reliable delivery confirmation. Perfect for elderly care, emergency notifications, or any scenario requiring immediate attention with acknowledgment.

## Features

- **Bidirectional Communication**: Two-way alert system with confirmation
- **Visual Feedback**: RGB LED states (idle, loading, success, error)
- **Audio Feedback**: Buzzer patterns for different states
- **mDNS Discovery**: No need for static IP addresses
- **Event-Driven Architecture**: Clean separation of concerns using EventBus
- **Reliable Delivery**: Confirmation mechanism ensures message delivery

## System Architecture

### Device Roles

The system consists of two CloudMouse devices:

1. **SENDER** (`cloudmouse-sender.local`)
   - Triggers emergency alerts
   - Waits for confirmation
   - Provides feedback when acknowledged

2. **RECEIVER** (`cloudmouse-receiver.local`)
   - Receives emergency alerts
   - Provides continuous alarm until acknowledged
   - Sends confirmation back to sender

## Configuration

### Device Configuration

Edit `DeviceConfig.h` to set device role:
```cpp
// Set ONE device as sender
#define IS_RECEIVER 0
#define IS_SENDER 1

// Set OTHER device as receiver
#define IS_RECEIVER 1
#define IS_SENDER 0

#define MDNS_RECEIVER "cloudmouse-receiver"
#define MDNS_SENDER "cloudmouse-sender"
```

### Network Requirements

- Both devices must be on the same WiFi network
- mDNS must be supported by the network (most home routers support it)
- Standard HTTP port 80 is used

## Communication Flow

### Alert Sequence
```
SENDER                          RECEIVER
  │                                │
  │ 1. Button Press                │
  │ ├─ Buzz + Green Flash          │
  │ └─ Enter Loading State         │
  │                                │
  │ 2. POST /alarm/ring           →│
  │                                │ 3. Receive Alert
  │                                │ ├─ Red LED + Loading
  │                                │ └─ Continuous Alarm
  │                                │
  │                                │ 4. Button Press
  │                                │ ├─ Stop Alarm
  │                                │ └─ Green Flash + Buzz
  │                                │
  │←5. POST /alarm/confirm         │
  │                                │
  │ 6. Confirmation Received       │
  │ ├─ Stop Loading                │
  │ ├─ Green Flash                 │
  │ ├─ Buzz                        │
  │ └─ Return to Idle              │
```

## API Endpoints

### Receiver Endpoints

#### POST `/alarm/ring`
Triggers alarm on receiver device.

**Request:**
```http
POST http://cloudmouse-receiver.local/alarm/ring
Content-Type: application/json
```

**Response:**
```
200 OK
ok
```

### Sender Endpoints

#### POST `/alarm/confirm`
Confirms alarm acknowledgment from receiver.

**Request:**
```http
POST http://cloudmouse-sender.local/alarm/confirm
Content-Type: application/json
```

**Response:**
```
200 OK
ok
```

## Events

The system uses an event-driven architecture with the following events:

| Event | Trigger | Action |
|-------|---------|--------|
| `ALARM_RING` | HTTP request received | Start alarm on receiver |
| `ALARM_STOP` | Button press on receiver | Stop alarm, send confirmation |
| `SEND_ALARM_REQUEST` | Button press on sender | Send alert to receiver |
| `ALARM_REQUEST_RECEIVED` | HTTP confirmation received | Visual/audio feedback on sender |

## LED Feedback States

### Sender States

| State | LED Color | LED Mode | Audio |
|-------|-----------|----------|-------|
| Button Press | Green | Flash | Buzz |
| Waiting for Confirm | Azure | Loading (blinking) | Silent |
| Confirmed | Green | Flash | Buzz |
| Error | Red | Flash | Error tone |
| Idle | Azure | Solid | Silent |

### Receiver States

| State | LED Color | LED Mode | Audio |
|-------|-----------|----------|-------|
| Alert Received | Red | Loading (blinking) | Continuous alarm |
| Acknowledged | Green | Flash | Buzz |
| Idle | Azure | Solid | Silent |

## Hardware Requirements

- 2x CloudMouse devices (ESP32-based)
- WiFi network
- Piezo buzzer on GPIO 14
- RGB LED (integrated in CloudMouse)
- Rotary encoder with button (integrated in CloudMouse)

## Installation

1. Clone the repository
2. Configure each device role in `DeviceConfig.h`
3. Update WiFi credentials
4. Flash one device as SENDER
5. Flash other device as RECEIVER
6. Both devices will auto-discover each other via mDNS

## Usage

### Sending an Alert (Sender Device)

1. Press the encoder button
2. LED turns green briefly with buzz (confirmation)
3. LED enters loading state (blinking azure)
4. Wait for receiver acknowledgment
5. LED flashes green with buzz when confirmed
6. Returns to idle state

### Acknowledging an Alert (Receiver Device)

1. Device receives alert automatically
2. LED turns red and blinks continuously
3. Buzzer plays alarm pattern continuously
4. Press encoder button to acknowledge
5. LED flashes green with buzz
6. Confirmation sent to sender automatically
7. Returns to idle state

## Error Handling

### Network Errors

If the sender cannot reach the receiver:
- Red LED flash (2 seconds)
- Error buzzer tone
- Returns to idle state
- User can retry by pressing button again

### Recovery

- System automatically recovers from network errors
- No manual intervention required
- Devices reconnect automatically when network is restored

## Troubleshooting

### Device Not Found

**Problem:** `cloudmouse-receiver.local` or `cloudmouse-sender.local` not resolving

**Solutions:**
- Ensure both devices are on the same WiFi network
- Check if mDNS is enabled on your router
- Wait 10-20 seconds after boot for mDNS to initialize
- Check Serial output for mDNS confirmation messages

### Alarm Not Stopping

**Problem:** Alarm continues after button press

**Solutions:**
- Check button connection
- Verify encoder is properly initialized
- Check Serial output for event processing
- Restart receiver device

### No Confirmation Received

**Problem:** Sender stays in loading state indefinitely

**Solutions:**
- Check if receiver is powered on and connected
- Verify receiver acknowledged the alarm
- Check network connectivity
- Review Serial output for HTTP errors

## Technical Details

### Components Used

- **WebServerManager**: Handles HTTP endpoints and mDNS
- **EventBus**: Event-driven communication between components
- **SimpleBuzzer**: Audio feedback with dedicated FreeRTOS task
- **LedManager**: RGB LED control for visual feedback
- **HTTPClient**: HTTP POST requests for inter-device communication

### Performance

- Alert delivery: < 100ms (typical LAN latency)
- mDNS resolution: 1-2 seconds on first boot
- Alarm response time: Immediate (event-driven)
- Power consumption: ~80mA per device (typical)

## Future Enhancements

- Battery level monitoring
- Multiple receiver support
- Alarm history logging
- MQTT support for longer range
- Web dashboard for monitoring
- Custom alarm patterns

## License

CloudMouse SDK - Proprietary

## Author

Built with ❤️ by Tibbo using CloudMouse SDK

---

**Note:** This is a critical safety system. Always test thoroughly before deployment in real-world scenarios.