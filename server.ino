#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

const int buttonUp = 32;
const int buttonDown = 33;

int16_t txt_x1, txt_y1;
uint16_t txt_w, txt_h;

// Scrolling
int scrollPos = 0;
int textHeight;
int textSpeed = 4; // pixels per scroll

String message = "";
String final = "";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "SUPERONLINE_WiFi_DAAC";
const char* password = "V37U7JFW3P7W";
const char* serverUrl = "https://eurythmical-eloisa-unrotating.ngrok-free.dev";

void setup() {
    pinMode(buttonUp, INPUT);
    pinMode(buttonDown, INPUT);
    Serial.begin(9600);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;) {}
    }

    display.display();
    delay(1000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Hello, World!"));
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
}

void sendMessageToServer(String msg) {
    String serverReply = "";

    if (WiFi.status() != WL_CONNECTED) {
        serverReply = "WiFi not connected!";
        Serial.println(serverReply);
        final = msg + "\n" + serverReply + "\n\n" + final;
        return;
    }

    HTTPClient http;

    // POST request
    http.begin(String(serverUrl) + "/");
    http.addHeader("Content-Type", "application/json");
    String postData = "{\"input\": \"" + msg + "\"}";
    int httpResponseCode = http.POST(postData);
    Serial.print("POST response: ");
    Serial.println(httpResponseCode);
    http.end();

    delay(500);

    // GET request with retry loop
    bool gotResponse = false;
    while (!gotResponse) {
        http.begin(String(serverUrl) + "/data");
        int httpCode = http.GET();

        if (httpCode > 0) {
            serverReply = http.getString();
            serverReply.replace("\\n", "\n");
            serverReply = cleanString(serverReply);
            gotResponse = true;  // successful GET
        } else {
            serverReply = "GET error: " + String(httpCode);  // show errors like -11
        }

        http.end();

        // Prepend own message with current serverReply so far, keep history
        final = msg + "\n" + serverReply + "\n\n" + final;

        // Allow display and scrolling to update
        display.getTextBounds(final.c_str(), 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
        textHeight = txt_h;
        display.clearDisplay();
        display.setCursor(0, -scrollPos);
        display.print(final);
        display.display();

        if (!gotResponse) delay(300); // wait before retrying
    }
}

// Converts literal \uXXXX sequences into proper UTF-8 and maps common symbols to ASCII
String cleanString(String s) {
    String out;
    out.reserve(s.length());
    for (size_t i = 0; i < s.length();) {
        if (s[i] == '\\' && i + 5 < s.length() && s[i+1] == 'u') {
            // Extract hex digits
            String hex = s.substring(i+2, i+6);
            char *endptr;
            long code = strtol(hex.c_str(), &endptr, 16);
            if (*endptr == 0) {
                // Map common Unicode codepoints to ASCII
                switch (code) {
                    case 0x2019: out += '\''; break; // ’
                    case 0x201C:
                    case 0x201D: out += '"'; break; // “ ”
                    case 0x2026: out += "..."; break; // …
                    case 0x2013: out += '-'; break; // –
                    case 0x2014: out += "--"; break; // —
                    default:
                        if (code < 128) out += (char)code;
                        else out += '?';
                }
                i += 6; // skip \uXXXX
                continue;
            }
        }
        // Regular ASCII
        out += s[i];
        i++;
    }
    return out;
}

void loop() {
    // Only print prompt once
    static bool promptPrinted = false;
    if (!promptPrinted) {
        Serial.println("Enter message to send: ");
        promptPrinted = true;
    }

    // Check if the user typed something
    if (Serial.available() > 0) {
        message = Serial.readStringUntil('\n');
        message.trim();
        if (message.length() > 0) {
            final += message + "\n\n";
            sendMessageToServer(message);
        }
        // After sending, print the prompt again
        Serial.println("Enter message to send: ");
    }

    display.getTextBounds(final.c_str(), 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
    textHeight = txt_h;

    // Scrolling controls
    if (digitalRead(buttonUp) == LOW) {
        scrollPos -= textSpeed;
        if (scrollPos < 0) scrollPos = 0;
    }
    if (digitalRead(buttonDown) == LOW) {
        scrollPos += textSpeed;
        if (scrollPos > textHeight - SCREEN_HEIGHT)
            scrollPos = textHeight - SCREEN_HEIGHT;
    }

    // Draw scrolling text
    display.clearDisplay();
    display.setCursor(0, -scrollPos); // shift text up or down
    display.print(final);
    display.display();

    delay(20);
}
