python spiffsgen.py 0xF0000 spiffs spiffs.bin

python %IDF_PATH%\components\esptool_py\esptool\esptool.py --port COM3 write_flash -z 0x110000 C:\Users\Jakob\ESP32\apps\EMT_DSP_Webserver\spiffs.bin