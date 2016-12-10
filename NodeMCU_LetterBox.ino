
/* 
 *  
 *  This program is designed to use an ESP8266 to monitoring a physical letter box !
 *  
 *  There is no loop, because it have to be triggered by a switch you have
 *  to connect to the Reset pin and activated by the lid.
 *  
 *  ESP8266 will remain in deep sleep mode until the postman open the lid.
 *  
 *  !!! IMPORTANT !!!
 *  The power have to be provided by a Li-Ion or lipo single cell trough a diode
 *  to reduce the voltage.
 *  Common silicium diode have a 0.6V drop out.
 *  With fully charged, Li-Ion cell (4.2V), the voltage will be 3.6 volts
 *  The minimal voltage required for ESP8266 is 2.9 volts, it will remain
 *  3.5V in the cell.
 *  During communication with wireless active, the current is beetween 50mA and 100mA
 *  In deep sleep mode, the current is lower than 200µA !
 *  
 *  
 *  The code will begin a serial connection and check voltage.
 *  If the voltage is too low, ESP8266 return to deep sleep mode.
 *  
 *  After the battery check, it will try to reach the wireless network you
 *  have to declare (ssid and password variables)
 *  Each try will blink the internal led.
 *  If no connection before 30 seconds, a reboot occur.
 *  
 *  When connected, the code will trying to confirm than smtpServer:port is alive
 *  if not, reboot
 *  
 *  Then, it will send a mail with the addresses you have set for
 *  senderMail and targetMail variables.
 *  
 *  If voltage is low, a warning is added to the mail.
 *  
 *  Adapted from SurferTim's Ethernet email program by Ty Tower May 2015
 *  See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3349#sthash.ry0vdMKz.dpuf
 *  Register at smtp2go.com --easy.
 *  
 *  Modifications by .AleX. :
 *  Encode user and password for smtp2go.com in base 64 utf8 are now done by base64.h library
 *  In alternate, you can use https://www.base64encode.org/ and directly put those in the right places in code
 */


#include <ESP8266WiFi.h>

#include <WiFiClient.h>

ADC_MODE(ADC_VCC); // Needed to read VCC voltage instead of A0 input

#include <Base64.h>  // Needed to convert plain text format to base64 required for SMTP

//const char* ssid = "My wireless network";
//const char* password = "My password";
const char* ssid = "MagicBox";
const char* password = "SoGoodSignal";



// SMTP Server declaration (For Yahoo service, you have to use numeric IP (no DNS))
//char smtpServer[] = "smtp2go.com";
//char smtpServer[] = "smtpcorp.com";
char smtpServer[] = "mail.smtp2go.com";
int smtpPort = 2525;

// This is the SMTP server account you will use to send mail
// It can be different than the mail adress used
//char* smtpUser = "MyAccount@SMTP_provider.net";
//char* smtpPassword = "MySMTPPassword";
char* smtpUser = "SenderMail@SMTPProvider.net";
char* smtpPassword = "MySMTPPassword";

// This is the From and To addresses for the mail
const char* senderMail = "My Letter Box <LetterBox@Whatever.net>";
const char* targetMail = "MailBox Owner <TargetMail@internet.com>";

// Choose the correct led for your own setup
//int internalLed = BUILTIN_LED ; // Common for lot of ESP8266 modules
int internalLed = 2 ; // For nodeMCU modules
//int internalLed = 4 ;

int lowVoltageLimit = 2900 ; //Minimal voltage in millivolts
int presentVoltage = ESP.getVcc();


// Create an instance of the server
// specify the port to listen on as an argument
// WiFiServer server(80);
WiFiClient client;



void setup(){
  Serial.begin(115200);

 
  pinMode ( internalLed, OUTPUT );
  digitalWrite (internalLed,HIGH);   


  Serial.println("Hello World");
  Serial.println("LetterBox system is starting");
  Serial.println("");

  Serial.print("Voltage present : ");
  Serial.println(ESP.getVcc());
  
  if (presentVoltage < 2500 ){
    Serial.println(F("Going to deep sleep mode because of low voltage"));
    ESP.deepSleep(0,WAKE_RF_DEFAULT);
  }
  
  Serial.println("");
  Serial.println("Wireless connection settings : ");
  
  Serial.print("SSID : ");
  Serial.println(ssid);
  
  Serial.print("Password : ");
  Serial.println(password);
  Serial.println("");

  
  

//  server.begin();
  delay(1000);


  wireless();
  
  sendEmail();
}

void loop()
{
  // nothing to do because this is a single mail just after reboot
}



byte wireless()
{
  Serial.println("");
  Serial.print("ESP8266 MAC address : ");
  Serial.println(WiFi.macAddress());

  Serial.println("");
  Serial.print("Trying to reach \"");
  Serial.print(ssid);
  Serial.print("\" network.");
  
  WiFi.mode(WIFI_STA);        // Optionnal, but prevent module create a new SSID (Default mode is AP/STA)
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    flash( 5, 245, 1 );       // Blink for each try to reach network

    
    if (millis() > 30000) {    // In case of no network found, reboot
      WiFi.disconnect();
      Serial.println("");
      Serial.print("Network ");
      Serial.print(ssid);
      Serial.print(" not found, trying to restart !");
      flash( 10, 40, 25 );
      ESP.wdtDisable();
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");

  
  IPAddress ip= WiFi.localIP();
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());

}




byte sendEmail()
{

  byte thisByte = 0;
  byte respCode;

  delay (500);

  if(client.connect(smtpServer,smtpPort) == 1) {
    Serial.println(F("connected"));
  }
  else {
    Serial.println(F("connection failed, trying to restart"));
    flash( 10, 40, 25 );
    ESP.wdtDisable();
    ESP.restart();
    return 0;
  }
  
  if(!eRcv()) {Serial.println("before ehlo");return 0 ;}

  Serial.println(F("Sending hello with IP Address"));
  client.print("EHLO ");
  client.println(WiFi.localIP());
  if(!eRcv()) {Serial.println("ehlo");return 0 ;}

  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if(!eRcv()) {Serial.println("auth");return 0 ;}

  Serial.println("Sending User");
  client.println(base64Encoding(smtpUser));  // User have to be base64 encoded
  if(!eRcv()) {Serial.println("user");return 0 ;}

  Serial.println(F("Sending Password"));
  client.println(base64Encoding(smtpPassword));  // Password have to be base64 encoded
  if(!eRcv()) {Serial.println("ehlo");return 0;}

// change to your email address (sender)
  Serial.println(F("Sending From"));
  client.print("MAIL From: "); 
  client.println(senderMail);
  if(!eRcv()) {Serial.println("email");return 0 ;}

// change to recipient address
  Serial.println(F("Sending To"));
  client.print("RCPT To: ");
  client.println(targetMail);
  if(!eRcv()) {Serial.println("email");return 0 ;}

  Serial.println(F("Sending DATA"));
  client.println("DATA");
  if(!eRcv()) {Serial.println("email");return 0 ;}

  Serial.println(F("Sending email"));

// change to recipient address
  client.print("To: ");
  client.println(targetMail);

// change to your address
  client.print("From: ");
  client.println(senderMail);

  client.println("Subject: You've got mail !\r\n");

  client.println("The door of the box have been opened...");
  presentVoltage = ESP.getVcc();
  client.print("Actual voltage is ");
  client.print(presentVoltage);
  client.println(" millivolts.");
  
  if (presentVoltage < lowVoltageLimit){
  client.println("Low batt, you have to change it");
  }
 

  client.println(".");  // End of the mail

  if(!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  client.println("QUIT");
 
  if(!eRcv()) return 0;

  client.stop();

  Serial.println(F("disconnected"));
  Serial.println(F(""));
  
  Serial.println(F("Going to deep sleep mode")); // Need hardware reset
  flash(100, 900, 5);
  ESP.deepSleep(0,WAKE_RF_DEFAULT);
  /*    
      flash(10, 990, 600);
      ESP.wdtDisable();
      ESP.restart();
   */
  return 1;

  
}
  
  
void flash(int lenght, int pause, int counter){

  if (counter < 0){
    counter = 1;
  }
 
  while (counter != 0){
    digitalWrite (internalLed,LOW);
    delay(lenght);
    digitalWrite(internalLed,HIGH);
    delay(pause);
    counter--;
    
  }
 
}



String base64Encoding(char* wordToEncode){
  int inputLen = strlen(wordToEncode);  // Note : sizeof can not be used because it will return the wordToEncode object size and note the size of value
  int encodedLen = base64_enc_len(inputLen);
  char base64Encoded[encodedLen];
  base64_encode(base64Encoded, wordToEncode, inputLen); 
  return base64Encoded;
}


byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while(!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      Serial.println(F("10 sec \r\nTimeout"));
      ESP.wdtDisable();
      ESP.restart();
      return 0;
    }
  }

  respCode = client.peek();

  while(client.available())
  { 
    thisByte = client.read();   
    Serial.write(thisByte);
  }

  if(respCode >= '4')
  {
    efail();
    return 0; 
  }

  return 1;
}


void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while(!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      Serial.println(F("efail \r\nTimeout"));
      return;
    }
  }

  while(client.available())
  { 
    thisByte = client.read();   
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("disconnected"));
}


