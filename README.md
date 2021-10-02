# diy-security-camera

Welcome to the DIY Security Camera project. This repository contains documentation, design materials, firmware and software for a custom-made WiFi camera device with built-in motion sensor, for your R&D projects involving cameras and/or sensors, and to use in your next field-security project!

![Camera Device](/static/bodegon01.png)

The device features cloud connectivity with AWS to send telemetry information and captured images, so you can incorporate a camera to any use case you need. The connection with the cloud also allows you to remotely operate and update the device, so you can effectively manage a large fleet of cameras without physically accessing them.

_NOTE: This device is a prototype, meant for experimentation purposes. The contents of this repository are work in progress, and they may change as the device specifications evolve._

## The device

This device features a battery-powered device with built-in camera, PIR sensor - i.e. motion detection - that connects over WiFi to your network and gives you the ability to watch a close-by area remotely and securely. Thanks to the PIR sensor, this camera can be configured to sleep and only wake up when motion is detected, hence prolonging the battery life. 

### Bill of Materials

In order to build this camera, you would need:

* 3D design of the enclosure, available in Thingiverse TODO Link.
* 2x R8mm x 1mm neodymium magnets.
* 1x 18650 Battery.
* 1x ESP32-cam module.
* 1x u.FL to SMA pig tail.
* 1x SMA antenna.
* 1x PIR module.
* 1x TP4056 battery module.
* 2x M3 nuts
* 2x M3x6mm screws.
* 2x 1.4mm screws - used for watches or glasses.

![Disassembled device](/static/disassembled.png)
_Disasembled device - 3D Design._

### Manufacturing

This device's design is split in two pieces - namely _Cover_ and _Back Cover_. 

Printer setup:

* Model: Prusa Research MK3 MMU2s
* Infill: >15%.
* Supports: No.
* Material: ABS.

#### Back cover

![Back cover part](/static/back-cover.png)
_Back cover part - 3D Design._

The back cover hosts the magnets for field attachment, the core electronics - i.e. battery, antenna pig tail and battery module -  and the cabling. Printing the back cover takes about 1h.

Once you have the back cover printed, fix the 2 M3 nuts in the side holes, so the screws can fix the design later. The two magnets should fit in the two holes of the back-side of the part.

Place the SMA connector of the pig tail in the side hole, and attach using the included nuts and components. The SMA connector should be placed outside of the part for connecting an antenna later.

The battery module has also a configured space in the side of the part, where it should fit and leave the micro-USB connection facing outwards. The battery is just placed in the middle of the _Back Cover_ part.

#### Cover

![Cover part](/static/cover.png)
_Cover part - 3D Design._

The cover part hosts the ESP and the PIR. The cover slides in the back cover, creating a closed box. The two screws on the sides keep the box closed.

The PIR module has two holes for screwing the module into the enclosure, and the _Cover_ part has two holes for screwing the module in. Use two small screws - I used 2x 1.5mm x 4mm screws used in glasses or watches, but others may work - to keep the PIR in place. The dome of the PIR should be placed outside of the part.

Attach the camera module to the ESP and fix it on top of the SD-card - usually these camera modules come with 2-sided sticky tape pasted on their back. The module should then just snap in place of the _Cover_ part, with the camera facing outwards from the case.

![Assembled device](/static/assembled.png)
_Assembled device - 3D Design._

#### Cabling

The cabling of the device is simple, but there's very little space on the box for the cables. They must be tidied up in the bottom of the back cover, and carefully placed when closing the box. We may in the future design a PCB to alleviate the cabling task, as the current box is very tight.

The PIR data pin is configured in the firmware as `GPIO6`. If you solder the connection to a different port, then you should adapt the firmware too.

### Firmware

The firmware of the device is based on Arduino, and it has the following features: 

* [x] It connects to your configured WiFi network.
* [x] It allows you to visualize the video feed locally in the same network.
* [x] It connects to the AWS Cloud and transmits telemetry messages.
* [x] It allows you to operate the device remotely.
* [ ] It sends secure video streams to AWS Kinesis Video Streams.
* [ ] It allows you to ship updates to the devices remotely.

The code is based on the Arduino sample for the camera web server - tutorial available [here](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/) - with customizations and enhancements to enable secure cloud communications and remote operations.

#### WiFi configuration

The device is configured to connect to a WiFi network as soon as it is booted up. You need to change a few parameters on `packages/device/config.h` so the device knows your WiFi configuration:

```h
// packages/device/config.h

const char WIFI_SSID[] =          "YOUR_WIFI_SSID_HERE";
const char WIFI_PASSWORD[] =      "YOUR_WIFI_PASSWORD_HERE";
```

After configuring the WiFi details, the device will connect to the network upon every boot. Once it's connected to the network it opens a port for visualizing the camera stream locally.

#### AWS Cloud configuration

By connecting the device to the AWS cloud you would be able to remotely operate the camera, access pictures and sensor information, and define automated processes based on sensor data. The device comes built-in with capabilities for connecting to the cloud.

##### Define the endpoint to the AWS Iot broker

You can find your endpoint address in the `Iot Core` console of AWS, under the _Settings_ option. You can also fetch your endpoint through the AWS CLI, using the following command:

```bash
aws iot describe-endpoint --endpoint-type iot:Data-ATS

# Sample response
# {
#   "endpointAddress": "1234567890abcd-ats.iot.eu-west-1.amazonaws.com"
# }
```

Copy the `endpointAddress` value and paste it in the `packages/device/config.h` file. Also, give your camera a unique name:

```h
// packages/device/config.h

const char AWS_IOT_URL[] =        "xxxxxxxxx.iot.amazonaws.com";
const char AWS_IOT_THING_NAME[] = "Camera1";
```

##### Iot connection material

To connect to the AWS cloud, you need to create authentication materials for the device. You can do this by running `bash scripts/provision-device.sh`. The script will create your authentication materials and store it in the `.provision/` folder. You need to copy the contents of the files in `packages/device/secrets.h`.

```h
// packages/device/secrets.h

// Amazon Root CA 1
static const char AWS_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";
```

### Flashing

Once you've configured the device parameters and credentials, you can flash the device. The simplest way to flash the device is to use the Arduino IDE. Simply open the file `packages/device/device.ino` in the IDE, connect your device's board and click _Upload_.. Remember that you need to add a jumper wire between `GPIO0` and `GND` so the board is in flashing mode.

Once flashing is finished, remove the flashing jumper and press the reset button. The device should start operating, connecting to the WiFi network and trying to connect to the AWS Iot broker.

_NOTE: Until you finish the next step for delivering the cloud resources, the device will not be able to connect to the broker successfully. It will try and fail, and you'd be able to watch the feed locally accessing the device's IP address._
## The cloud

This project includes a cloud footprint for the device, that allows to get data from it and to operate it remotely. This section describes this cloud infrastructure, and the built-in capabilities included in the device.

### Architecture

The cloud architecture for this project is rather simple, and it's designed to:

* Allow the device to connect to the cloud - it establishes a long-lived MQTT connection with the AWS IoT broker.
* Store some of the device capacities and metrics remotely, to access them even when disconnected.
* Provide a secure channel for devices to send periodic pictures and other sensor data.
* Respond to remote requests providing pictures and other sensor data.
* Allow the device to be provisioned and updated remotely.

![Cloud overview](/static/cloud-overview.png)

### Cloud provisioning

This section is focused on the delivery of the project's architecture in the cloud, and the preliminary configuration steps to do to [configure devices to connect to it](#aws-cloud-configuration).

_NOTE: Before continuing this section, make sure you have [generated security credentials](#iot-connection-material) for your device, and modified the file `packages/cloud/bin/cloud.ts` with your certificate ARN. This next step will activate the certificate and link it with the cloud infrastructure._

The infrastructure of this project uses the [AWS CDK](https://docs.aws.amazon.com/cdk/latest/guide/home.html) 2.0, and the source code for the infrastructure can be found at `packages/cloud`. You can use the source to deploy directly using the `cdk deploy` command, or include the delivery of the infrastructure in your CICD pipelines.

Once the infrastructure is delivered - it shouldn't take long to finish - your AWS account should be ready, and hosting all required infrastructure for the device's cloud features to operate correctly. Continue reading for instructions on how to configure the device to operate with the newly created infrastructure.

## Coming up next

We will soon publish new documentation of this device, as well as some sample use cases solved with it. Stay tuned!

# Contributing

If you see something is not right, please feel free to fix it and submit a PR. A good practice is to check out existing issues first, creating one if none existing applies, and give us a shout before starting your development. Thank you for your contributions!

# License

This repository and all its contents are released under [MIT license](/LICENSE.md).
