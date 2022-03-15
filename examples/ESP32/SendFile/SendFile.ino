/*******************************************************************
    A telegram bot for your ESP32 that demonstrates sending file
    from SPIFFS.
 *******************************************************************/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

// Wifi network station credentials
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

unsigned long bot_lasttime;          // last time messages' scan has been done
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

File myFile;
char file_name[] = "/test file.txt";
char msg[] = "Hello!";

bool isMoreDataAvailable();
byte getNextByte();

void writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\r\n", path);

   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("failed to open file for writing");
      return;
   }
   if(file.print(message)){
      Serial.println("file written");
   }else {
      Serial.println("frite failed");
   }
   file.close();
}

bool isMoreDataAvailable()
{
  return myFile.available();
}

byte getNextByte()
{
  return myFile.read();
}

void handleNewMessages(int numNewMessages)
{
  String chat_id = bot.messages[0].chat_id;

  myFile = SPIFFS.open(file_name);

  if (myFile)
  {
    Serial.print(file_name);
    Serial.print("....");

    //Content type for PNG image/png
    String sent = bot.sendFileByBinary(chat_id, "text/plain", myFile.size(), file_name,
                                        isMoreDataAvailable,
                                        getNextByte, nullptr, nullptr);

    if (sent)
    {
      Serial.println("was successfully sent");
    }
    else
    {
      Serial.println("was not sent");
    }

    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening photo");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  // make new file to send 
  writeFile(SPIFFS, file_name, msg);
//   writeFile(SPIFFS, file_name, "hi");

}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
