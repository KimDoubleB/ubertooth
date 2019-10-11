## specan_ui.py is a basic spectrum analysis tool for the Ubertooth.

Requirements (or "what I developed with"):

  * libusb 1.0.8
  * PyUSB 1.0.0-a1
  * Qt 4.7.3
  * PySide 1.0.2 (0.4 will work, but will not be supported for
    long due to API changes)
  * Python 2.6.6
  * Numpy 1.3

Debian/Ubuntu distributions may also require:

  * libusb-dev ("libusb-*-dev" matching your libusb package)

It's only been tested on:

  * Mac OS X 10.6.7
  * Ubuntu 10.10

...but *should* execute nicely on other platforms.

Execute with:

  $ ubertooth-specan-ui

...and enjoy the show. On Linux, you may need to do this instead:

  $ sudo ubertooth-specan-ui

...or you can set up udev to allow access to the Ubertooth for non-administrative users. See the "Getting Started" section of the
Ubertooth project Web page for more details:

  http://ubertooth.sourceforget.net/

---

## Update (Custom)

### Default mode (Frequency domain graph)
```
python ubertooth-specan-ui

python3.7 ubertooth-specan-ui
```

### Custom mode (Time domain graph)
You must add parameter save and freqlist in this mode.
freq means frequency that you want to scan.
```
python ubertooth-specan-ui -save t -freqlist freq1 freq2 ...

python ubertooth-specan-ui -save true -freqlist freq1 freq2 ...

python3.7 ubertooth-specan-ui -save t -freqlist freq1 freq2 ...

python3.7 ubertooth-specan-ui -save true -freqlist freq1 freq2 ...
```
