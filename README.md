# LICK
LICK is a free program to install Linux from Windows without burning a CD or
using a USB. It is as simple as installing and running LICK, selecting a Linux
ISO image, and clicking install. A few seconds later, you can reboot into
Linux. Currently only Puppy Linux-based distributions are supported.

LICK runs on any Windows version, from Windows 95 to Windows 10. Check below
for any special notes on your Windows version type.

# Windows Version Notes
## Windows 8, 8.1 and 10
Windows 8 and up have a feature called 'Fast Startup'. This **cannot** be
enabled if LICK is installed. LICK disables Fast Startup upon installation.

## UEFI Systems with Secure Boot
LICK supports secure boot, but requires a manual step during the first
reboot. On subsequent reboots, these steps should not need to be taken.

1. On the first reboot, you will see a menu, `Shim UEFI key management`.
   Use the down arrow to select `Enroll key from disk`, then press enter.
2. Select your drive (probably has a long name), then press enter.
   If you see multiple options, try each, one at a time.
3. Use the down arrow to select `lick.cer`
4. Enter the key number `0`, then enter..
5. Enter `y`, then enter.
6. Finally, select `Return to filesystem list`, `Exit` then `Continue boot`.

On subsequent reboots, these steps will not need to be taken.

## Windows ME
By default, Windows ME does not have all dependencies LICK requires. To fix
this, install [Me2Dos](http://www.rkgage.net/bobby/download/Me2Dos.exe).
You can also read the [README](http://www.rkgage.net/bobby/download/readme.txt).
