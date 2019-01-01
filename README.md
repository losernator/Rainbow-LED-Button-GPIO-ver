# RainbowLED
Application for Arcade LED button control using Raspberry Pi GPIO.
Works with Gpio joystick driver ( https://github.com/recalbox/mk_arcade_joystick_rpi ) and Retropie.

## Video
[![LEDVIDEO](http://img.youtube.com/vi/PyplqCNt8OQ/0.jpg)](https://youtu.be/PyplqCNt8OQ)

## Hardware
* Raspberry Pi GPIO
* 3v/5v LED or LED Button
* cables for LED
* default button oder is
<pre><code> SE ST

   Y X L
  B A R
</code></pre>

![LED](/Manual/rainbowled02.png)
![GPIO](/Manual/rainbowled01.png)
![GPIO](/Manual/rainbowled03.png)
![GPIO](/Manual/rainbowled04.jpg)

## Install
first install rainbowLED
<pre><code>cd /home/pi
sudo apt-get install wiringpi
git clone https://github.com/losernator/rainbowLED.git
cd /home/pi/rainbowLED/
sudo chmod 755 rainbowledbtn
sudo chmod 755 dimmedledbtn
</code></pre>
second, make it run on start with retropie
<pre><code>sudo nano /opt/retropie/configs/all/autostart.sh
</code></pre>
a. and then add this on top
<pre><code>/home/pi/rainbowLED/dimmedledbtn /dev/input/js0 &
</code></pre>

if you want chage button order like this
<pre><code> SE ST

   X Y L
  A B R
</code></pre>
then change like this
<pre><code>/home/pi/rainbowLED/dimmedledbtn /dev/input/js0 ab &
</code></pre>

## Configuration
<pre><code>
Usage: dimmedledbtn <#joystick> [dimming delay] [button mapping type] [chargeshot]
[dimming delay] : lower is faster dimming speed, default:3
[button mapping type] : 'ab' or 'ba', default:ba
[chargeshot] : 'charge' or 'nocharge', default:charge
</code></pre>

