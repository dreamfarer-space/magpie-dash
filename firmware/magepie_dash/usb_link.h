#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
void usb_link_begin();
bool usb_link_connected();
bool usb_link_receive(uint8_t byte);  // UI task only; true means protocol consumed it
void usb_link_tick();               // UI task only; bounded/nonblocking
bool usb_link_get_json(const String &path, JsonDocument &doc); // fetch task only

// UI task only: pop a command string sent inside a CRC-checked frame.
bool usb_link_take_command(char *out, size_t cap);
