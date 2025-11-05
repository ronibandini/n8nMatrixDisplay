/*
 * n8n Matrix Display 
 * Roni Bandini, 11/2025
 * MIT license  
 * 1 x DFRobot ESP32 C3 Dev Module (CDC on boot, 160mhz 80, Flash 4mb 32, Partition 4mb 1.2)
 * 2 x WS2812B Matrix 16x16
 */

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PIN 4
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 16
#define BRIGHTNESS 20

// 📶 WiFi Configuration
const char* ssid = "";  
const char* password = "";  
const char* webhookURL = "";

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800
);

// 🎨 Colors
const uint16_t BLACK = matrix.Color(0, 0, 0);
const uint16_t PLAYER_COLOR = matrix.Color(0, 255, 0);  // Green  
const uint16_t ENEMY_COLOR = matrix.Color(255, 0, 0);   // Red
const uint16_t LASER_COLOR = matrix.Color(0, 255, 255); // Cyan
const uint16_t SCROLL_TEXT_COLOR = matrix.Color(0, 255, 0);  

const uint16_t colors[] = {
  matrix.Color(255, 0, 0),  
  matrix.Color(0, 255, 0),  
  matrix.Color(0, 0, 255),
  matrix.Color(255, 255, 0),
  matrix.Color(255, 0, 255),
  matrix.Color(0, 255, 255)
};
const int numColors = sizeof(colors) / sizeof(colors[0]);

String webhookMessage = "n8n display, Roni Bandini. Conectando al WiFi...";

// 🎬 Modes
enum Mode { SCROLL_TEXT, PACMAN_ANIM, PONG_ANIM, SPACE_INVADER_ANIM };
Mode currentMode = SCROLL_TEXT;

unsigned long modeTimer = 0;
const unsigned long TEXT_DURATION = 120000;    // 2 min (120,000 ms)
const unsigned long ANIMATION_DURATION = 15000; // 15 sec  
int interactionCounter = 0;                    

// Scrolling text vars
int x = MATRIX_WIDTH;
int colorIndex = 0;

// Pac-Man vars
float pacmanX = -10;
float pacmanY = (MATRIX_HEIGHT / 2.0) - 6;
int mouthPhase = 0;
float mouthAngle = 0;
int dotPositions[] = {8, 16, 24, 30};
bool dotsVisible[] = {true, true, true, true};

// Pong vars
float ballX = MATRIX_WIDTH / 2.0;
float ballY = MATRIX_HEIGHT / 2.0;
float ballVX = 1.2;
float ballVY = 0.8;
int paddleLeftY = MATRIX_HEIGHT / 2 - 2;
int paddleRightY = MATRIX_HEIGHT / 2 - 2;
int paddleHeight = 4;

// 👾 Space Invaders Data & Vars

// Space Invaders Calamar
const PROGMEM bool spaceInvaderEnemy[4][8] = {
  {0, 0, 1, 0, 0, 1, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {1, 0, 1, 1, 1, 1, 0, 1}
};
const int ENEMY_SHIP_WIDTH = 8;
const int ENEMY_SHIP_HEIGHT = 4;

// Player
const PROGMEM bool spaceInvaderPlayer[3][5] = {
  {0, 1, 0, 1, 0},
  {1, 1, 1, 1, 1},
  {1, 0, 1, 0, 1}
};
const int PLAYER_SHIP_WIDTH = 5;
const int PLAYER_SHIP_HEIGHT = 3;

// Space Invaders animation
float invaderX = 0;
float invaderDir = 1; // 1 = right, -1 = left
float bulletY = -1; // -1 means no bullet
int playerX = (MATRIX_WIDTH / 2) - (PLAYER_SHIP_WIDTH / 2); // Centered

// Draw enemy ship
void drawEnemyShip(int x_pos, int y_pos, uint16_t color) {
  for (int r = 0; r < ENEMY_SHIP_HEIGHT; r++) {
    for (int c = 0; c < ENEMY_SHIP_WIDTH; c++) {
      if (spaceInvaderEnemy[r][c]) {
        matrix.drawPixel(x_pos + c, y_pos + r, color);
      }
    }
  }
}

// Draw player ship
void drawPlayerShip(int x_pos, int y_pos, uint16_t color) {
  for (int r = 0; r < PLAYER_SHIP_HEIGHT; r++) {
    for (int c = 0; c < PLAYER_SHIP_WIDTH; c++) {
      if (spaceInvaderPlayer[r][c]) {
        matrix.drawPixel(x_pos + c, y_pos + r, color);
      }
    }
  }
}

// Shooting
void animateSpaceInvader() {
  matrix.fillScreen(BLACK);

  // Enemy ship movement
  invaderX += invaderDir * 0.5;
  if (invaderX >= MATRIX_WIDTH - ENEMY_SHIP_WIDTH || invaderX <= 0) {
    invaderDir *= -1;
  }


  drawEnemyShip((int)invaderX, 1, ENEMY_COLOR);


  int playerY = MATRIX_HEIGHT - PLAYER_SHIP_HEIGHT - 1;
  drawPlayerShip(playerX, playerY, PLAYER_COLOR);


  if (bulletY == -1) {
    if (random(0, 15) == 0) {  
      bulletY = playerY - 1;  
    }
  }

  if (bulletY != -1) {
    int bulletX = playerX + PLAYER_SHIP_WIDTH / 2;
    
    bulletY -= 1.8;

    matrix.drawPixel(bulletX, (int)bulletY, LASER_COLOR); 

    if (bulletY <= ENEMY_SHIP_HEIGHT && bulletX >= (int)invaderX && bulletX < (int)invaderX + ENEMY_SHIP_WIDTH) {
      matrix.drawPixel((int)invaderX + random(0, ENEMY_SHIP_WIDTH), 1 + random(0, ENEMY_SHIP_HEIGHT), matrix.Color(255, 255, 255));
      bulletY = -1;  
    }
    
    if (bulletY < 0) {
      bulletY = -1;
    }
  }
}


void animateSpaceInvaderIntro() {
  int shipY = MATRIX_HEIGHT - ENEMY_SHIP_HEIGHT - 2;  

  for (int i = 0; i < 2; i++) {
    for (int shipX = 0; shipX < MATRIX_WIDTH - ENEMY_SHIP_WIDTH; shipX++) {
      matrix.fillScreen(BLACK);
      drawEnemyShip(shipX, shipY, ENEMY_COLOR);
      matrix.show();
      delay(40);
    }
    for (int shipX = MATRIX_WIDTH - ENEMY_SHIP_WIDTH; shipX >= 0; shipX--) {
      matrix.fillScreen(BLACK);
      drawEnemyShip(shipX, shipY, ENEMY_COLOR);
      matrix.show();
      delay(40);
    }
  }
}


void fetchWebhookData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    webhookMessage = "Sin WiFi";
    return;
  }
  
  WiFiClientSecure client;
  
  client.setInsecure();
  client.setTimeout(30000);
  client.setHandshakeTimeout(30);
  
  HTTPClient http;
  
  http.begin(client, webhookURL);
  http.setTimeout(30000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Matrix/1.0");
  http.addHeader("Accept", "*/*");
  http.addHeader("Connection", "close");
  
  Serial.println("=== Connecting with n8n ===");
  Serial.print("URL: ");
  Serial.println(webhookURL);
  
  int httpCode = http.GET();
  
  Serial.print("HTTP Response code: ");
  Serial.println(httpCode);
  
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      String payload = http.getString();
      Serial.println("Webhook response:");
      Serial.println(payload);
      
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonArray array = doc.as<JsonArray>();
        if (array.size() > 0) {
          JsonObject firstItem = array[0];
          if (firstItem.containsKey("output")) {
            const char* text = firstItem["output"];
            if (text && strlen(text) > 0) {
              webhookMessage = String(text);
              if (webhookMessage.length() > 500) {
                webhookMessage = webhookMessage.substring(0, 500);
              }
              Serial.println("✓ Message extracted from 'output'");
              x = MATRIX_WIDTH;
            } else {
              Serial.println("✗ 'output' field is empty");
              webhookMessage = "Empty msg";
            }
          } else {
            Serial.println("✗ JSON missing 'output' field");
            webhookMessage = "No 'output'";
          }
        } else {
          Serial.println("✗ JSON array is empty");
          webhookMessage = "Empty array";
        }
      } else {
        Serial.print("✗ JSON parse error: ");
        Serial.println(error.c_str());
        webhookMessage = "Error JSON: " + String(error.c_str());
      }
    } else {
      Serial.print("✗ Unexpected HTTP code: ");
      Serial.println(httpCode);
      webhookMessage = "HTTP " + String(httpCode);
    }
  } else {
    Serial.print("✗ HTTP error code: ");
    Serial.println(httpCode);
    
    if (httpCode == -1) {
      webhookMessage = "Error: connection";
      Serial.println("Connection failed");
    } else if (httpCode == -11) {
      webhookMessage = "Error: SSL handshake";
      Serial.println("SSL/TLS handshake failed");
    } else {
      webhookMessage = "Error HTTP: " + String(httpCode);
    }
  }
  
  http.end();
  client.stop();
  
  Serial.println("=== n8n connection complete ===\n");
}

// 🟡 Pac-Man  
void drawPacman(float px, float py, float angle) {
  uint16_t yellow = matrix.Color(255, 255, 0);
  uint16_t black = matrix.Color(0, 0, 0);

  float r = 6.0;
  float cx = px + r;
  float cy = py + r;

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrt(dx * dx + dy * dy);

      if (dist <= r) {
        float a = atan2(dy, dx);
        if (a > -angle && a < angle) {
          matrix.drawPixel(x, y, black);
        } else {
          matrix.drawPixel(x, y, yellow);
        }
      }
    }
  }
  matrix.drawPixel((int)(cx + 2), (int)(cy - 4), black);
}

void drawDot(int dx, int dy) {
  uint16_t white = matrix.Color(255, 255, 255);
  matrix.drawPixel(dx, dy, white);
  matrix.drawPixel(dx + 1, dy, white);
  matrix.drawPixel(dx, dy + 1, white);
  matrix.drawPixel(dx + 1, dy + 1, white);
}

void animatePacman() {
  matrix.fillScreen(0);

  for (int i = 0; i < 4; i++)
    if (dotsVisible[i]) drawDot(dotPositions[i], MATRIX_HEIGHT / 2 - 1);

  for (int i = 0; i < 4; i++)
    if (dotsVisible[i] && pacmanX >= dotPositions[i] - 3 && pacmanX <= dotPositions[i] + 2)
      dotsVisible[i] = false;

  mouthPhase++;
  mouthAngle = 0.7 * fabs(sin(mouthPhase * 0.3)) + 0.3;

  drawPacman(pacmanX, pacmanY, mouthAngle);

  pacmanX += 0.8;
  if (pacmanX > MATRIX_WIDTH + 6) {
    pacmanX = -10;
    for (int i = 0; i < 4; i++) dotsVisible[i] = true;
  }
}

// 📰 Scrolling text  
void scrollText() {
  matrix.fillScreen(0);
  matrix.setFont(&FreeMonoBold9pt7b);
  matrix.setTextSize(1);
  matrix.setTextColor(SCROLL_TEXT_COLOR); 
  matrix.setCursor(x, 14);
  matrix.print(webhookMessage);

  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(webhookMessage.c_str(), x, 14, &x1, &y1, &w, &h);

  if (--x < -w) {
    x = MATRIX_WIDTH;
  }
}

// 🏓 Pong animation  
void drawPaddle(int x, int y, uint16_t color) {
  for (int i = 0; i < paddleHeight; i++)
    matrix.drawPixel(x, y + i, color);
}

void animatePong() {
  matrix.fillScreen(0);

  ballX += ballVX;
  ballY += ballVY;

  if (ballY <= 0 || ballY >= MATRIX_HEIGHT - 1) ballVY *= -1;

  if (ballY > paddleLeftY + paddleHeight / 2) paddleLeftY++;
  else if (ballY < paddleLeftY + paddleHeight / 2) paddleLeftY--;
  if (ballY > paddleRightY + paddleHeight / 2) paddleRightY++;
  else if (ballY < paddleRightY + paddleHeight / 2) paddleRightY--;

  if (ballX <= 1 && ballY >= paddleLeftY && ballY <= paddleLeftY + paddleHeight)
    ballVX = fabs(ballVX);
  if (ballX >= MATRIX_WIDTH - 2 && ballY >= paddleRightY && ballY <= paddleRightY + paddleHeight)
    ballVX = -fabs(ballVX);

  if (ballX < -1 || ballX > MATRIX_WIDTH + 1) {
    ballX = MATRIX_WIDTH / 2;
    ballY = MATRIX_HEIGHT / 2;
    ballVX = (random(0, 2) == 0 ? 1.2 : -1.2);
    ballVY = (random(0, 2) == 0 ? 0.8 : -0.8);
  }

  drawPaddle(0, paddleLeftY, matrix.Color(0, 255, 0));
  drawPaddle(MATRIX_WIDTH - 1, paddleRightY, matrix.Color(0, 255, 0));
  matrix.drawPixel((int)ballX, (int)ballY, matrix.Color(255, 255, 255));
}

// Selecciona la siguiente animación de forma aleatoria
Mode getNextRandomAnimation() {
  Mode animations[] = {PACMAN_ANIM, PONG_ANIM, SPACE_INVADER_ANIM};
  int randomIndex = random(0, 3);  
  return animations[randomIndex];
}

void displayIntroN8N() {
  const uint16_t GREEN = matrix.Color(0, 255, 0);
  const uint16_t YELLOW = matrix.Color(255, 255, 0);
  
  matrix.fillScreen(BLACK);
  matrix.setFont(&FreeMonoBold9pt7b);  
  matrix.setTextSize(1); 
  
  int16_t x1, y1;
  uint16_t w, h;
  
 
  matrix.getTextBounds("n", 0, 0, &x1, &y1, &w, &h);
  int char_width_n = w;   
  matrix.getTextBounds("8", 0, 0, &x1, &y1, &w, &h);
  int char_width_8 = w; 
  const int space_offset = 3; 
  int total_n8n_width = char_width_n + space_offset + char_width_8 + space_offset + char_width_n; 
  int start_x = (MATRIX_WIDTH - total_n8n_width) / 2; 
  int y_pos = 14; 
  matrix.setTextColor(GREEN);
  matrix.setCursor(start_x, y_pos);
  matrix.print("n");
  int pos_after_first_n = start_x + char_width_n + space_offset;  
  matrix.setTextColor(YELLOW);
  matrix.setCursor(pos_after_first_n, y_pos);
  matrix.print("8");
  int pos_after_eight = pos_after_first_n + char_width_8 + space_offset; 
  matrix.setTextColor(GREEN);
  matrix.setCursor(pos_after_eight, y_pos);
  matrix.print("n");
  matrix.show();
  delay(10000);    
  matrix.fillScreen(BLACK);
  matrix.show();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nn8n Matrix Display");
  
  matrix.begin();
  matrix.setBrightness(BRIGHTNESS);
  matrix.setTextWrap(false);
  displayIntroN8N(); 

  randomSeed(analogRead(0));  

  webhookMessage = "WiFi...";
  x = MATRIX_WIDTH;

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    scrollText();
    matrix.show();
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    webhookMessage = "WiFi OK - IP: " + WiFi.localIP().toString();
  } else {
    Serial.println("\n✗ WiFi failed!");
    webhookMessage = "WiFi Error";
  }
  
  delay(3000);
  Serial.println("Attempting first webhook fetch...");
  fetchWebhookData();
  
  modeTimer = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - modeTimer > (currentMode == SCROLL_TEXT ? TEXT_DURATION : ANIMATION_DURATION)) {

    if (currentMode == SCROLL_TEXT) {
      currentMode = getNextRandomAnimation();
      interactionCounter++;
      
      if (currentMode == SPACE_INVADER_ANIM) {
          invaderX = 0;
          invaderDir = 1;
      }

    } else {
      currentMode = SCROLL_TEXT;
      x = MATRIX_WIDTH;  

      if (interactionCounter >= 5) {
        Serial.println("--- Getting data... ---");
        fetchWebhookData();
        interactionCounter = 0;  
      }
    }
    modeTimer = now;  
  }
  
  if (currentMode == SCROLL_TEXT) {
    scrollText();
    delay(35);  
  } else if (currentMode == PACMAN_ANIM) {
    animatePacman();
    delay(42);  
  } else if (currentMode == PONG_ANIM) {
    animatePong();
    delay(21);  
  } else if (currentMode == SPACE_INVADER_ANIM) {
    animateSpaceInvader();
    delay(35);  
  }

  matrix.show();
}