# Minecraft: Pi Edition: Reborn
Minecraft: Pi Edition Modding Project

## Getting Started
1. Remove Old Minecraft: Pi Edition Package If Installed (``sudo apt remove minecraft-pi``)
2. Download Appropriate Package From [Here](https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/deb/) (See Table Below)
3. Install With ``sudo apt install ./<Path To File>``
4. Have Fun!

### Packages
| Package | Description |
| --- | --- |
| ``minecraft-pi-reborn-server`` | Minecraft Pi Edition Modded Into A Dedicated Server |
| ``minecraft-pi-reborn-virgl`` | Minecraft Pi Edition Using VirGL For Hardware Acceleration (Recommended For Desktop) |
| ``minecraft-pi-reborn-native`` | Minecraft: Pi Edition Using Docker Device Mounting For GPU Acceleration (Recommended For ARM Devices (ie. Raspberry Pi, PinePhone, etc)) |

### General Notes

#### Docker Versions
While the distribution-provided version of Docker is fine for most systems, in rare cases it can be outdated and cause bugs. Before reporting a bug, try using the official builds of Docker. These can be installed by following your distribution's instructions at https://docs.docker.com/engine/install.

#### Debian/Raspbian Buster And ``libseccomp2``
By default Raspbian Buster ships an older version of the package ``libseccomp2``. This package is used to block certain dangerous system calls from running inside Docker containers. The included version accidentally blocks the system call ``clock_gettime64``, this causes bugs inside Minecraft: Pi Edition. However, the Debian ``buster-backports`` repo includes an updated version. You can enable the ``buster-backports`` repo and update ``libseccomp2`` by running:

```sh
# Install Backports Key
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 04EE7237B7D453EC
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 648ACFD622F3D138
# Install Backports Repository
echo 'deb http://deb.debian.org/debian buster-backports main' | sudo tee -a /etc/apt/sources.list
# Update APT Index
sudo apt update
# Install Updated libseccomp2
sudo apt install -t buster-backports libseccomp2
```

## Troubleshooting Crashes
Game logs are located in ``/tmp/minecraft-pi``.

### ``Error response from daemon: error gathering device information while adding custom device "/dev/dri": no such file or directory``
Make sure you are using the correct GPU drivers for your system. If you are using a Raspberry Pi, make sure you have set your GPU driver to ``Full KMS`` or ``Fake KMS`` in ``raspi-config``.

### ``Segmentation Fault``
1. Attempt To Reproduce Issue And Record Instructions
2. Report On Issue Tracker Including The Instructions To Reproduce

## Dedicated Server
The dedicated server is a version of Minecraft: Pi Edition modified to run in a headless environment. It loads settings from a ``server.properties`` file.

To use, install the ``minecraft-pi-server`` package and run ``minecraft-pi-server``. It will generate the world and ``server.properties`` in the current directory.

This is also compatible with MCPE 0.6.1.

### Limitations
- Player data is not saved because of limitations with MCPE LAN worlds
  - An easy workaround is to place your inventory in a chest before logging off
- Survival mode servers are only compatible with ``minecraft-pi-reborn`` clients

## Modding
[View Modding](MODDING.md)

## Credits
[View Credits](CREDITS.md)
