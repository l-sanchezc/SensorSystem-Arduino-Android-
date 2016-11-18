/* Code for project "Intelligent sensor system to control and manage the growth of plants for biofuel production"
   Created by Lazaro Sanchez Campos and Alejandro Nicolas Llor - July 2016
*/

// This code will be installed in the Intel Galileo 2 boards to act as individual servers to transmit the sensor parameters
// Define the pin to which the Grove temperature, light, moisture sensors and relay for water pump are connected.
 int pinRelay = A3;

// B-value of the thermistor.
const int B = 3975;

// Libraries needed
#include <SPI.h>
#include <Ethernet.h>

// MAC address and IP address for controllers below.
byte mac[] = {
  0x98, 0x4F, 0xEE, 0x01, 0xBD, 0x80 };
IPAddress ip(192,168,0,99);

// Initialize the Ethernet server library
// with the IP address in port 80
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(pinRelay, OUTPUT);   
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {

  // listen for incoming clients
  EthernetClient client = server.available();
 
  if (client) {
 
  Serial.println("new client");
  // an http request ends with a blank line
  boolean currentLineIsBlank = true;

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      // if gotten to the end of the line and the line is blank, the http request has ended,
      // a reply is sent   
      if (c == '\n' && currentLineIsBlank) {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        // add a meta refresh tag (refresh every 5 seconds)
        client.println("<meta http-equiv=\"refresh\" content=\"5\">");
       
        // output the value of each analog input pin
        for (int analogChannel = 0; analogChannel < 3; analogChannel++) {
          int sensorReading = analogRead(analogChannel);
          int previousSensorReading = sensorReading;
          // if the variable belongs to the temperature sensor....
          if (analogChannel==0)  {
            
              // Determine the current resistance of the thermistor based on the sensor value.
              float resistance = (float)(1023-sensorReading)*10000/sensorReading;
              // Calculate the temperature based on the resistance value.
              float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;
              
              if (temperature!=0 && temperature>0 && temperature<40){
                  client.print("The temperature is: ");        
                  client.print(temperature);
                  client.print(" C");
                  client.println("<br />");
              }

              // Error prevention loop, checks if the values are between right range
              else if (temperature<0||temperature>40){
                client.println("Error 12: The temperature value is not within normal values, please check conditions");
                client.println("<br />");
                delay (1000);
              }
            }
           
            // if the variable belongs to the light sensor....
            if (analogChannel==1)  {

              if (sensorReading!=0&&sensorReading>0&&sensorReading<1000){
                client.print("The level of light is: ");
                 
                  if(sensorReading<789){
                    client.print("Low ");
                  }

                  else client.print("High ");

                  client.print(sensorReading);
                  client.println("<br />");
              }

              else if (sensorReading<0||sensorReading>1000) {
                client.println("Error 22: No information received from light sensor, please check connection or sensor status");
                client.println("<br />");
                delay (1000);
              }
            }
           
            // if the variable belongs to the moisture sensor....
            if (analogChannel==2) {
              double humidityLevel = sensorReading/10.0;

              if (sensorReading!=0&&humidityLevel>0&&humidityLevel<100){
              client.print("The humidity is: ");
              client.print(humidityLevel);
              client.print(" %");
              client.println("<br />");

                // If soil is not wet enough, the water pump will be activated for 30 seconds
                if (humidityLevel<50.00){
                  client.print("entro");
                  client.print(humidityLevel);
                  client.println("Low humidity detected, initiating watering system..."); 
                  digitalWrite(pinRelay, HIGH);
                  delay(5000);
                  client.println("Stopping watering system...");
                  digitalWrite(pinRelay, LOW);
                  delay(1000);
                }  
              }

              else if (sensorReading<0||sensorReading>1000) {
                client.println("Error 32: No information received from moisture sensor, please check connection or sensor status");
                client.println("<br />");
                delay (1000);
              }
            }
             
        } // End of for loop  

        client.println("</html>");
        break;
      }

      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      }

      else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }
  }
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  Serial.println("client disonnected");
  }
}
