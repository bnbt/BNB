# BNB
Big N Button

# Change baud to 9600 for no more noise
```
AT+CIOBAUD=9600
```

# PIN Configuration
### ESP WIFI
- ESP_RX_PIN    7   ->  ESP TX
- ESP_TX_PIN    8   ->  ESP RX
- ESP_RESET_PIN 12  -> ESP CH_PD

### RFID reader
- RFID_D0   2
- RFID_D1   3

### BUTTON
- BUTTON_PIN    6

### RGB LED
- RED_PIN       9
- GREEN_PIN     10
- BLUE_PIN      11
