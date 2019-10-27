/*
 *
 * Outpost Client Module
 * =====================
 * 
 * Created by Gideon Tong
 * 
 */

// Includes
#include "Arduino.h"
#include "WiFi.h"
#include "heltec.h"

// Definitions
#define     BAND        915E6           // Set to 915MHz IHS
#define     MAX_CLIENTS 4               // Maximum number of WiFi clients
const char* ssid        = "OutpostDev"; // WiFi name
const char* psk         = "test1234";   // WiFi password
int         progress    = 0;            // Progress bar indicator
String      header;                     // Stores WiFi header
WiFiServer  server(80);                 // Set webserver to port 80

// Functions
// Draws the progress bar
void progressDraw() {
    Heltec.display -> clear();
    Heltec.display -> drawProgressBar(0, 32, 120, 10, progress);
    Heltec.display -> setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display -> drawString(64, 15, "Booting Outpost: " + String(progress) + "%");
    Heltec.display -> display();
}

// Draws the status screen
void viewStatus() {
    int clients = WiFi.softAPgetStationNum();
    Heltec.display -> clear();
    Heltec.display -> drawProgressBar(0, 32, 120, 10, (0.0 + clients) / MAX_CLIENTS * 100);
    Heltec.display -> setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display -> drawString(64, 15, "Connected: " + String(clients) + "/" + String(MAX_CLIENTS));
    Heltec.display -> display();
}

// Main Program
void setup() {
    Heltec.begin(true /* Display */, false /* LoRa */, true /* Serial */);
    Heltec.display -> setContrast(255);
    Heltec.display -> clear();
    progressDraw();

    // Setting up the WiFi
    Serial.print("Setting up AP...");
    WiFi.softAP(ssid, psk);
    Serial.println(" Success!");
    progress = 33;
    progressDraw();
    
    // Make the IP address available
    Serial.print("IP address is: ");
    IPAddress IP = WiFi.softAPIP();
    Serial.println(IP);
    progress = 66;
    progressDraw();

    // Start the web server
    server.begin();
    progress = 100;
    progressDraw();
}

void loop() {
    WiFiClient client = server.available();
    viewStatus();

    if(client) {
        Serial.println("New device connected!");
        String currentLine = "";
        while(client.connected()) {
            if(client.available()) {
                char c = client.read();
                Serial.write(c);
                header += c;
                if(c == '\n') {
                    if(currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><title>Outpost</title></head>");
                        client.println("<body><h1>Outpost</h1></body>");
                        client.println("</html>");
                        client.println();
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if(c != '\r') {
                    currentLine += c;
                }
            }
        }
        header = "";
        client.stop();
        Serial.println("Client disconnected.");
    }
}
