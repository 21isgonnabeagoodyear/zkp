#!/bin/sh
cp /www/demo/statusclosed.html /www/demo/status.html
echo 1 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 0 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 1 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 0 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 1 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 0 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 1 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
sleep 0.1
echo 0 > /sys/devices/platform/leds-gpio/leds/gl-connect:red:wlan/brightness
