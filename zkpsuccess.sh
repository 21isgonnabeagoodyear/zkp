#!/bin/sh
echo 1 > /sys/devices/platform/leds-gpio/leds/gl-connect:green:lan/brightness
cp /www/demo/statusopen.html /www/demo/status.html
sleep 5
cp /www/demo/statusclosed.html /www/demo/status.html
echo 0 > /sys/devices/platform/leds-gpio/leds/gl-connect:green:lan/brightness
