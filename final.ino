#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Keypad.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

const int buttonUp = 32;
const int buttonDown = 33;
const int buttonSend = 34;

int16_t txt_x1, txt_y1;
uint16_t txt_w, txt_h;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {14,12,13,2};
byte colPins[COLS] = {4,23,17,16};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Scrolling
int scrollPos = 0;
int textHeight;
int textSpeed = 4;

char lastKey = NO_KEY;
int tapCount = 0;
int mode = 0; // 0=letters,1=numbers,2=symbols,3=exotic

String message = "";
String finalMessage = "";
String final = "";

bool uppercaseFlag = false;

bool preview = false;
bool messageSent = false;

int KeypadScrollPos = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Hotspot";
const char* password = "12121212";
const char* serverUrl = "https://eurythmical-eloisa-unrotating.ngrok-free.dev";

void resetMultiTap();
void handleKey(char key);
void outputCharacter(char key, int count);
char getPreviewChar(char key, int count);
void sendMessageToServer(String msg);

String cleanString(String s) {
    String out;
    out.reserve(s.length());
    for (size_t i = 0; i < s.length();) {
        if (s[i] == '\\' && i + 5 < s.length() && s[i+1] == 'u') {
            String hex = s.substring(i+2, i+6);
            char *endptr;
            long code = strtol(hex.c_str(), &endptr, 16);
            if (*endptr == 0) {
                switch (code) {
                    case 0x2019: out += '\''; break;
                    case 0x201C:
                    case 0x201D: out += '"'; break;
                    case 0x2026: out += "..."; break;
                    case 0x2013: out += '-'; break;
                    case 0x2014: out += "--"; break;
                    default:
                        if (code < 128) out += (char)code;
                        else out += '?';
                }
                i += 6;
                continue;
            }
        }
        out += s[i];
        i++;
    }
    return out;
}

void setup() {
    pinMode(buttonUp, INPUT);
    pinMode(buttonDown, INPUT);
    pinMode(buttonSend, INPUT);
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
    display.println(F("Alper tarafindan yapildi, bir yapay zeka chatbot'u."));
    display.display();
    delay(5000);
    display.clearDisplay();
    display.display();

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    display.print("CONNECTED TO WIFI SUCESSFULLY!");
    display.display();
    delay(5000);
    display.clearDisplay();
    display.display();
}

void loop() {
    char key = keypad.getKey();
    if(key) handleKey(key);
    if (key != NO_KEY) preview = true;

    String displayText = preview ? (finalMessage) : finalMessage + final;
    display.getTextBounds(displayText.c_str(), 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
    textHeight = txt_h;

    if(digitalRead(buttonUp) == HIGH){
        if(preview){
            KeypadScrollPos -= textSpeed;
            if(KeypadScrollPos < 0) KeypadScrollPos = 0;
        } else {
            scrollPos -= textSpeed;
            if(scrollPos < 0) scrollPos = 0;
        }
    }
    if(digitalRead(buttonDown) == HIGH){
        if(preview){
            KeypadScrollPos += textSpeed;
            if(KeypadScrollPos > max(0, textHeight - SCREEN_HEIGHT)) 
                KeypadScrollPos = max(0, textHeight - SCREEN_HEIGHT);
        } else {
            scrollPos += textSpeed;
            if(scrollPos > max(0, textHeight - SCREEN_HEIGHT))
                scrollPos = max(0, textHeight - SCREEN_HEIGHT);
        }
    }

    display.clearDisplay();

    if(preview){

        if(digitalRead(buttonSend) == HIGH && message.length() > 0){
            finalMessage = message;
            Serial.print("Message sent: "); Serial.println(message);
            resetMultiTap();
            final = "";
            message = "";
            KeypadScrollPos = 0;
            preview = false;
            messageSent = false;
        }

        display.setTextSize(1);
        display.setCursor(0, -KeypadScrollPos);
        display.print(finalMessage + message);

        if(lastKey != NO_KEY){
            display.setTextSize(2);
            display.setCursor((SCREEN_WIDTH/2)-6, (SCREEN_HEIGHT/2)-8);
            display.print(getPreviewChar(lastKey, tapCount));
            display.display();
        }

    } else {

        if(finalMessage.length() > 0 && !messageSent){
            sendMessageToServer(finalMessage);
            messageSent = true;
        }

        display.setTextSize(1);
        display.setCursor(0, -scrollPos);
        display.print(cleanString(final));
        display.display();
    }

    display.display();
    delay(20);
}

void resetMultiTap() {
    lastKey = NO_KEY;
    tapCount = 0;
}

void handleKey(char key) {
    if(key=='A'){
      static unsigned long lastATime = 0;
      unsigned long now = millis();
    
      if(now - lastATime < 500){
          Serial.println("Mode: Uppercase Letters");
          uppercaseFlag = true;
      } else {
          Serial.println("Mode: Lowercase Letters");
          uppercaseFlag = false;
      }
      
      mode = 0;
      lastATime = now;
      resetMultiTap();
      return;
    }

    if(key=='B'){ mode=1; resetMultiTap(); Serial.println("Mode: Numbers"); return; }
    if(key=='C'){ mode=2; resetMultiTap(); Serial.println("Mode: Symbols"); return; }
    if(key=='D'){ mode=3; resetMultiTap(); Serial.println("Mode: Exotic"); return; }

    if(key == '*') {
        if(lastKey != NO_KEY){
            Serial.println("Cancelled selection");
            resetMultiTap();
        }
        return;
    }

    if(key == '0'){
        if(mode == 1){
            lastKey = '0';
            tapCount = 0;
            Serial.println("Selecting 0...");
        } else {
            message += ' ';
            Serial.println("Added space");
            resetMultiTap();
        }
        return;
    }

    if(key == '#' && lastKey != NO_KEY){
        outputCharacter(lastKey, tapCount);
        Serial.print("Character accepted: "); Serial.println(message[message.length()-1]);
        resetMultiTap();
        return;
    }

    if(key >= '1' && key <= '9'){
        if(lastKey == key) tapCount++;
        else { lastKey = key; tapCount = 0; }
        char previewChar = getPreviewChar(lastKey, tapCount);
        Serial.print("Selecting: "); Serial.println(previewChar);
    }
}

void outputCharacter(char key, int count) {
    char c = getPreviewChar(key, count);
    message += c;
}

char getPreviewChar(char key, int count) {
    if(mode == 0){
        char c = ' ';
        if(key=='1'){ c = (count%3==0) ? '.' : (count%3==1) ? ',' : '\''; }
        if(key=='2'){ c = (count%3==0) ? 'a' : (count%3==1) ? 'b' : 'c'; }
        if(key=='3'){ c = (count%3==0) ? 'd' : (count%3==1) ? 'e' : 'f'; }
        if(key=='4'){ c = (count%3==0) ? 'g' : (count%3==1) ? 'h' : 'i'; }
        if(key=='5'){ c = (count%3==0) ? 'j' : (count%3==1) ? 'k' : 'l'; }
        if(key=='6'){ c = (count%3==0) ? 'm' : (count%3==1) ? 'n' : 'o'; }
        if(key=='7'){ c = (count%4==0) ? 'p' : (count%4==1) ? 'q' : (count%4==2) ? 'r' : 's'; }
        if(key=='8'){ c = (count%3==0) ? 't' : (count%3==1) ? 'u' : 'v'; }
        if(key=='9'){ c = (count%4==0) ? 'w' : (count%4==1) ? 'x' : (count%4==2) ? 'y' : 'z'; }
        
        if(uppercaseFlag){
            if(c >= 'a' && c <= 'z') c = toupper(c);
        }
        return c;
    }
    else if(mode == 1){
        if(key>='0' && key<='9') return key;
    }
    else if(mode == 2){
        if(key=='1'){ if(count%3==0) return '.'; else if(count%3==1) return ','; else return '\''; }
        if(key=='2'){ if(count%3==0) return '/'; else if(count%3==1) return '-'; else return '+'; }
        if(key=='3'){ if(count%3==0) return '!'; else if(count%3==1) return '@'; else return '#'; }
        if(key=='4'){ if(count%3==0) return '$'; else if(count%3==1) return '%'; else return '^'; }
        if(key=='5'){ if(count%3==0) return '&'; else if(count%3==1) return '*'; else return '('; }
        if(key=='6'){ if(count%3==0) return '_'; else if(count%3==1) return '='; else return '['; }
        if(key=='7'){ if(count%3==0) return '{'; else if(count%3==1) return '}'; else return '<'; }
        if(key=='8'){ if(count%3==0) return '>'; else if(count%3==1) return '~'; else return '`'; }
        if(key=='9'){ if(count%3==0) return '|'; else if(count%3==1) return ':'; else return ';'; }
    }
    else if(mode == 3){
        if(key=='1'){ if(count%2==0) return 'α'; else return 'Α'; }
        if(key=='2'){ if(count%2==0) return 'β'; else return 'Β'; }
        if(key=='3'){ if(count%2==0) return 'γ'; else return 'Γ'; }
        if(key=='4'){ if(count%2==0) return 'δ'; else return 'Δ'; }
        if(key=='5'){ if(count%2==0) return 'ε'; else return 'Ε'; }
        if(key=='6'){ if(count%2==0) return 'ζ'; else return 'Ζ'; }
        if(key=='7'){ if(count%2==0) return 'η'; else return 'Η'; }
        if(key=='8'){ if(count%2==0) return 'θ'; else return 'Θ'; }
        if(key=='9'){ if(count%2==0) return 'ι'; else return 'Ι'; }
        if(key=='0'){ if(count%2==0) return 'κ'; else return 'Κ'; }
        if(key=='*'){ if(count%2==0) return 'λ'; else return 'Λ'; }
        if(key=='#'){ if(count%2==0) return 'μ'; else return 'Μ'; }
    }
    return ' ';
}

void sendMessageToServer(String msg) {
    if (WiFi.status() != WL_CONNECTED) {
        final = msg + "\nWiFi not connected!\n\n" + final;
        return;
    }

    HTTPClient http;
    http.begin(String(serverUrl) + "/");
    http.addHeader("Content-Type", "application/json");
    String postData = "{\"input\": \"" + msg + "\"}";
    int postCode = http.POST(postData);
    Serial.print("POST response: ");
    Serial.println(postCode);
    http.end();

    http.begin(String(serverUrl) + "/data");
    int httpCode = http.GET();
    String serverReply = "";

    if (httpCode > 0) {
        serverReply = http.getString();
        serverReply.replace("\\n", "\n");
        serverReply = cleanString(serverReply);
    } else {
        serverReply = "GET error: " + String(httpCode);
    }
    http.end();

    final = msg + "\n" + serverReply + "\n\n" + final;

    finalMessage = "";
}
