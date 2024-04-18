## Point One Polaris Client for MAVLink
This application uses the [polaris](https://github.com/PointOneNav/polaris) library to interface with Point One's GNSS corrections service and send RTCM corrections via MAVLink.

You must first create a PointOne account and copy your API key to the config file. <br>
https://app.pointonenav.com/

The **config.toml** file is used to configure the program settings. <br>
https://github.com/ARK-Electronics/polaris-rtk-client/blob/d24f21f2907bd1246c60027bf1c2f1a8338ab5f9/config.toml#L1-L2

### Behavior
The application waits for a MAVSDK connection. Once connected the Polaris client is created and runs asynchronously. The received GPS_RAW_INT mavlink messages from the flight controller are converted to ECEF and sent to the Polaris server in order to begin receiving corrections. The corrections are binary RTCM3 and are published as GPS_RTCM_DATA MAVLink messages.

### Build
Pre-requisites
```
sudo apt install libssl-dev libgflags-dev libgoogle-glog-dev libboost-all-dev
```
Install MAVSDK if you haven't already, the latest releases can be found at https://github.com/mavlink/MAVSDK/releases
```
sudo dpkg -i libmavsdk-dev_2.4.1_debian12_arm64.deb
```
Build
```
make
```
Run
```
./build/polaris-rtk-client
```
