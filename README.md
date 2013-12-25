opengalax2 - touchscreen daemon utilizing tslib
==============================================

&copy; 2013 Oskari Rauta - oskari.rauta[at]gmail(.)com

Opengalax2 is a Linux userland input device driver for touchscreen panels manufactured by
touchscreen vendors that are supported by tslib. Original OpenGalax touchscreen daemon
supported serial devices manufactured by [EETI](http://home.eeti.com.tw/web20/eGalaxTouchDriver/linuxDriver.htm):
eGalax, eMpia, TouchKit, Touchmon, HanTouch, D-WAV Scientific Co, eTurboTouch, Faytech, PanJit,
3M MicroTouch EX II, ITM, etc.

This driver supports USB versions of same devices through tslib. Calibration is done with tslib,
user only needs to set desired display resolution.

This is fork based on Pau Oliva Fora's work at https://github.com/poliva/opengalax
copyrighted 2012 by Pau Oliva Fora - pof[at]eslack(.)org

**Why?** Because EETI only offers closed source binary drivers for those touch panels, the eGalax Touch driver is outdated
and doesn't work properly on new Xorg servers (the module ABI differs) or wayland/weston, and the newer closed source 
eGTouch daemon driver doesn't work properly with PS2 devices nor USB devices like mine, so I forked Pau Oliva's opengalax
and changed it to use device through tslib to have an alternative Open Source (GPL) driver.


Configuration
-------------

When first launched, opengalax2 will create a configuration file in `/etc/opengalax.conf' with the default configuration values:

    # opengalax configuration file

    #### config data:
    uinput_device=/dev/uinput
    rightclick_enable=0
    rightclick_duration=350
    rightclick_range=10
    # direction: 0 = normal, 1 = invert X, 2 = invert Y, 4 = swap X with Y
    direction=0
    screen_width=800
    screen_height=480


When launched without parameters, opengalax will read the configuration from this
config file, some configuration values can also be overwritten via the command line:

    Usage: opengalax [options]
        -x number            : override display width (default: 800)
        -y number            : override display height (default: 480)
        -v                   : display version and exit
        -d                   : display set configuration (with changes through -x and -y possibly)
    	-f                   : run in foreground (do not daemonize)
    	-u <uinput-device>   : override uinput device (default: /dev/uinput)


Calibration
-----------

Calibration is supposed to be done and set with tslib's provided calibration.

Usage in Xorg
-------------

Opengalax will configure evdev xorg driver to handle right click emulation by default, so you don't need
to enable right click emulation in the configuration file. If for some reason you do not want to use evdev,
opengalax can handle the right click emulation itself by enabling it in the configuration file.
