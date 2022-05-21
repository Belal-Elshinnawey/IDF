# An ESP-IDF OTA Update Development Component

This is a light weight component that connects to a local network share, downloads a .bin file to your ESP then disconnects and Deinits Wifi.

Change OTA_SSID, OTA_PASSWORD and  OTA_URL_FW_BIN to your settings and call ota_init_loop() at the head of your main.

The component will init 
1- NVS_flash.

2- init WiFi connection.

3- download the firmware.

4- check if file download is complete.

5- Restart the device in the new firmware.

6- Check if the new image is OK.

7- try to reconnect to Wifi.
 - if Wifi reconnect fails, the image is marked as invalid and the last image is reloaded, and the new one is removed.
 
-- In this step the .bin file should be removed from your network share --

8- after reconnect the device will try to redownload the same file.
   - if the file is no longer available, the Wifi and NVS is Deinited and the new app starts running
   
   - if a file is still present, it will download and repeat.

 
