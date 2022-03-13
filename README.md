# Aegir
ESP8266-based replacement mash controller for Oneconcept Hopfengott vessel

# Google Cloud and Firebase setup

See 
* [Using Cloud IoT Core to connect a microcontroller (ESP32) to the Google Cloud Platform](http://nilhcem.com/iot/cloud-iot-core-with-the-esp32-and-arduino)
* [Using Firestore with IoT Core for device configuration](https://cloud.google.com/community/tutorials/cloud-iot-firestore-config).
* [Getting Started with Firebase (Realtime Database)](https://randomnerdtutorials.com/esp8266-nodemcu-firebase-realtime-database/)
* https://github.com/GoogleCloudPlatform/google-cloud-iot-arduino

Create a Cloud IoT registry following the steps in [the quickstart guide](https://cloud.google.com/iot/docs/quickstart).

Generate an Eliptic Curve (EC) private / public key pair:

```
openssl ecparam -genkey -name prime256v1 -noout -out ec_private.pem
openssl ec -in ec_private.pem -pubout -out ec_public.pem
```
