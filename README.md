<img width="1536" height="1024" alt="n8nMatrixDisplayFront" src="https://github.com/user-attachments/assets/cd6e8131-fabd-411b-8259-bf3f4c1071f5" />

# 🟩🤖 n8n Matrix Display

**A 32×16 RGB LED information display powered by ESP32, n8n workflows, APIs, and AI.**

n8n Matrix Display combines two **16×16 WS2812B LED matrices**, a compact **DFRobot Beetle ESP32-C3**, and an **n8n AI workflow** to create a physical scrolling information feed.

The ESP32 periodically calls an n8n webhook. Behind that webhook, an AI agent can collect information from multiple tools and services, summarize it, and return a single text response ready to scroll across the LED matrix.

The example workflow can combine:

* 🕒 date and time
* 📧 unread Gmail messages
* 📈 stock-market information
* 📰 technology news
* 🚗 AI-powered parking availability
* 🌦️ weather data
* 🤖 Google Gemini reasoning

Between information cycles, the display runs retro-inspired **Pac-Man, Pong, and Space Invaders animations**.

---

## ✨ Features

* 🟩 32×16 RGB LED display
* 🔌 DFRobot Beetle ESP32-C3
* 🌐 n8n webhook integration
* 🤖 n8n AI Agent
* ✨ Google Gemini support
* 📧 Gmail integration
* 📈 Marketstack integration
* 📰 RSS news feeds
* 🕒 date/time tool
* 🌦️ optional weather integration
* 🚗 Gemini multimodal parking detection
* 📷 optional webcam capture utility
* 👾 Space Invaders animation
* 🟡 Pac-Man animation
* 🏓 Pong animation
* 🖨️ Custom 3D-printable enclosure
* 📜 MIT licensed

---

# 🧠 Concept

Traditional LED tickers usually have their data sources hard-coded directly into the microcontroller.

This project takes a different approach:

```text
APIs / Gmail / RSS / Camera / Services
                 │
                 ▼
          ┌─────────────┐
          │     n8n     │
          │             │
          │  AI Agent   │
          │  + Tools    │
          └──────┬──────┘
                 │
                 │ Webhook response
                 ▼
          ┌─────────────┐
          │ ESP32-C3    │
          │             │
          │ HTTP client │
          └──────┬──────┘
                 │
                 ▼
       ┌──────────────────┐
       │ 32×16 WS2812B    │
       │ LED Matrix       │
       │                  │
       │ scrolling text   │
       └──────────────────┘
```

The ESP32 only needs to know how to:

1. connect to Wi-Fi
2. call an n8n webhook
3. read the returned text
4. display it

All of the higher-level data integration stays inside **n8n**.

---

# ⚙️ Architecture

The physical display is made from two 16×16 WS2812B panels arranged as one:

```text
16×16                16×16
┌────────────────┐   ┌────────────────┐
│                │   │                │
│    Panel 1     │──▶│    Panel 2     │
│                │   │                │
└────────────────┘   └────────────────┘

           ↓

       32 × 16 pixels
```

The firmware defines:

```cpp
#define PIN 4
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 16
#define BRIGHTNESS 20
```

The panels are configured as:

```cpp
NEO_MATRIX_BOTTOM +
NEO_MATRIX_RIGHT +
NEO_MATRIX_COLUMNS +
NEO_MATRIX_ZIGZAG
```

using:

```cpp
NEO_GRB + NEO_KHZ800
```

---

# 🧰 Parts

> ℹ️ **The product URLs below are intentionally preserved from the original project. Some contain referral / affiliate tracking parameters.**

| Qty | Component                    | Link                                                                                                                           |
| --: | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
|   1 | **DFRobot Beetle ESP32-C3**  | [DFRobot](https://www.dfrobot.com/product-2566.html?tracking=hOuIhw4fDaJRTdy4abz04npbQC78dqxBkqVt7XMFYxEXj2s0ukWgm71wbut0ewUP) |
|   2 | **WS2812B 16×16 RGB Matrix** | [Amazon](https://amzn.to/4oR7WSW)                                                                                              |
|   1 | **5 V / 3 A Power Supply**   | [DFRobot](https://www.dfrobot.com/product-1937.html?tracking=hOuIhw4fDaJRTdy4abz04npbQC78dqxBkqVt7XMFYxEXj2s0ukWgm71wbut0ewUP) |
|   6 | 3 mm screws                  | —                                                                                                                              |

You will also need:

* jumper wires
* USB cable
* optional 3D-printed enclosure

---

# 🔌 Wiring

The two WS2812B panels are connected in series using the supplied matrix connector.

Conceptually:

```text
                   5 V / 3 A supply
                    │          │
                    │          └────────── GND
                    ▼
             ┌─────────────┐
             │ Matrix #1   │
             │ 16 × 16     │
             └──────┬──────┘
                    │ Data Out
                    ▼
             ┌─────────────┐
             │ Matrix #2   │
             │ 16 × 16     │
             └─────────────┘

Beetle ESP32-C3
GPIO 4 ─────────────▶ Matrix #1 Data In
GND    ─────────────▶ Common GND
```

The first panel receives power from the external **5 V / 3 A supply**.

The ESP32 data signal is connected to:

```text
GPIO 4
```

> ⚠️ The ESP32 and external LED power supply must share a **common ground**.

Do not attempt to power all 512 RGB LEDs directly from the ESP32 board.

---

# 📡 n8n workflow

The repository includes an importable workflow:

```text
n8nMatrixDisplayUpload.json
```

Import it into n8n and configure the required credentials and endpoints.

The workflow follows this basic structure:

```text
ESP32
  │
  │ HTTP GET
  ▼
Webhook
  │
  ▼
AI Agent
  │
  ├── Gemini
  ├── RSS News
  ├── Parking Image Analysis
  ├── Marketstack
  ├── Gmail
  ├── Date / Time
  └── Weather
  │
  ▼
Respond to Webhook
  │
  ▼
ESP32 Matrix
```

---

# 🤖 AI Agent

The included workflow uses an **n8n AI Agent** with a Google Gemini chat model.

The agent acts as the information editor for the display.

Rather than scrolling large raw API responses, it can combine several tools and generate something concise such as:

```text
Tuesday 4 Nov, 3:18 PM.
No parking spaces available.
1 unread email about Pocket Gone.
AI stocks are up 3%.
```

The exact prompt and language can be changed in n8n.

The sample workflow currently contains Spanish prompts, so adapt them to your preferred language and display format.

---

# 🧩 Included n8n tools

## 🕒 Date and time

The workflow includes an n8n Date & Time tool so the AI agent can include the current time in its output.

---

## 📰 Technology news

An RSS Feed tool provides technology news.

The imported workflow contains:

```text
https://RSS_URL_PLACEHOLDER/
```

Replace this with your preferred RSS feed.

---

## 📈 Stock-market information

The example uses **Marketstack**.

The included workflow is configured around:

```text
TSLA
NVDA
GOOGL
```

Add your Marketstack credentials inside n8n and change the symbols as required.

---

## 📧 Gmail

The Gmail tool retrieves:

```text
Unread messages
Limit: 1
```

Configure your Gmail OAuth credentials in n8n before activating this part of the workflow.

---

## 🌦️ Weather

An **OpenWeatherMap** tool is included in the workflow.

The example city is:

```text
Buenos Aires
```

The weather node is currently **disabled in the supplied workflow**, so enable it and configure OpenWeatherMap credentials if you want weather in the feed.

---

# 🚗 AI-powered parking detection

One of the example data sources uses **Gemini 2.5 Flash multimodal analysis** to determine whether there appears to be an available parking space.

The workflow sends an image to Gemini with a prompt equivalent to:

```text
Is there a free space for a medium-sized car?
```

Conceptually:

```text
Parking area
     │
     ▼
Webcam
     │
     ▼
parking.jpg
     │
     ▼
Web server
     │
     ▼
n8n Gemini Tool
     │
     ▼
Free parking space?
```

The resulting answer becomes another tool available to the n8n AI Agent.

---

# 📷 Webcam utility

The repository includes:

```text
parkingWebcam.py
```

This optional Python script:

* captures a webcam frame
* saves it as `parking.jpg`
* uploads it to an FTP server
* repeats every five minutes

The default interval is:

```python
INTERVALO_SEGUNDOS = 5 * 60
```

Configure:

```python
WEBCAM_INDEX = 0

FTP_HOST = "your_ftp_host.com"
FTP_USER = "your_ftp_username"
FTP_PASSWORD = "your_ftp_password"
FTP_REMOTE_PATH = "/"
```

Install OpenCV:

```bash
pip install opencv-python
```

Then run:

```bash
python parkingWebcam.py
```

Once the image is available online, configure its URL in the Gemini image-analysis node in n8n.

> 🔐 Standard FTP transmits credentials without encryption. For a permanent deployment, prefer a secure upload mechanism such as SFTP, HTTPS, or authenticated object storage.

---

# 🌐 ESP32 → n8n communication

The ESP32 performs an HTTPS `GET` request to:

```cpp
const char* webhookURL = "";
```

The webhook response is expected to be a JSON array whose first item contains:

```json
[
  {
    "output": "Text to show on the matrix"
  }
]
```

The firmware extracts:

```text
output
```

and uses it as the scrolling message.

Messages longer than:

```text
500 characters
```

are truncated.

---

# 🔄 Display cycle

The normal display mode is scrolling text.

The current timing constants are:

```cpp
const unsigned long TEXT_DURATION = 120000;
const unsigned long ANIMATION_DURATION = 15000;
```

This means:

```text
Scrolling information
      │
      │ 2 minutes
      ▼
Retro animation
      │
      │ 15 seconds
      ▼
Scrolling information
```

The retro animation is selected randomly.

After several cycles, the ESP32 requests fresh information from n8n.

---

# 👾 Retro animations

The firmware contains three built-in animation modes.

## 🟡 Pac-Man

A small Pac-Man crosses the display while consuming dots.

```text
●   ●   ●   ●       ᗧ
```

---

## 🏓 Pong

The ESP32 animates:

* two paddles
* one moving ball
* simple autonomous paddle tracking

---

## 👾 Space Invaders

A small Space Invaders-inspired sequence contains:

* enemy sprite
* player ship
* movement
* laser fire

These animations run locally on the ESP32 and do not depend on n8n.

---

# 🚀 Installation

## 1. Clone the repository

```bash
git clone https://github.com/ronibandini/n8nMatrixDisplay.git
cd n8nMatrixDisplay
```

Repository structure:

```text
n8nMatrixDisplay/
├── LICENSE
├── README.md
├── n8nMatrixDisplay.ino
├── n8nMatrixDisplayUpload.json
└── parkingWebcam.py
```

---

# 🔧 Arduino setup

Install the ESP32 board package in Arduino IDE.

Select an ESP32-C3-compatible configuration.

The original source notes:

```text
DFRobot ESP32 C3 Dev Module
CDC on boot
160 MHz
Flash 4 MB
```

Install the required Arduino libraries:

* **Adafruit GFX Library**
* **Adafruit NeoMatrix**
* **Adafruit NeoPixel**
* **ArduinoJson**

The ESP32 core provides:

* `WiFi`
* `WiFiClientSecure`
* `HTTPClient`

---

# 📶 Configure Wi-Fi

Open:

```text
n8nMatrixDisplay.ino
```

Set:

```cpp
const char* ssid = "";
const char* password = "";
const char* webhookURL = "";
```

Example:

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* webhookURL = "https://YOUR_N8N_HOST/webhook/...";
```

---

# 🧩 Import the n8n workflow

In n8n:

1. Create or open a project.
2. Import `n8nMatrixDisplayUpload.json`.
3. Open the **ESP32 In** Webhook node.
4. Configure its webhook path.
5. Activate the workflow.
6. Copy the production webhook URL.
7. Paste that URL into `n8nMatrixDisplay.ino`.

Then configure whichever tools you want to use.

Depending on the imported workflow, these may include credentials for:

```text
Google Gemini
Gmail
Marketstack
OpenWeatherMap
```

Also replace:

```text
RSS_URL_PLACEHOLDER
IMAGE_URL_PLACEHOLDER
```

with your own sources.

---

# ⚡ Upload the firmware

Compile and upload:

```text
n8nMatrixDisplay.ino
```

to the Beetle ESP32-C3.

Open the Serial Monitor at:

```text
115200 baud
```

On boot, the display shows an n8n intro and then attempts to connect to Wi-Fi.

The serial console reports:

```text
WiFi connected
IP
RSSI
Webhook HTTP status
Webhook response
```

---

# 🖨️ 3D enclosure

The enclosure was designed in **Autodesk Fusion 360** and printed in PLA.

It consists of five STL files:

```text
Conector.stl
Cuerpo.stl
Left.stl
Right.stl
Tapa.stl
```

The two large pieces hold the matrix panels while the smaller parts provide the panel connector and ESP32 enclosure.

### Cults3D

**[n8n matrix display with ESP32 and WS2812B](https://cults3d.com/en/3d-model/gadget/n8n-matrix-display-with-esp32-and-ws2812b)**

Use six 3 mm screws for assembly.

---

# 🎥 Demo

**[▶️ n8n Matrix Display demo on YouTube](https://www.youtube.com/shorts/Qm-eOpMM7Nw)**

---

# 🔬 Ideas for extending the project

1. **🚇 Add local transit information** — use n8n to combine bus, subway, train, or traffic APIs into the display feed.

2. **🏠 Connect Home Assistant or MQTT** — show sensor values, alarms, energy consumption, door states, or automation events.

3. **🎨 Let n8n control display modes** — extend the webhook response so n8n can specify text color, brightness, animation, and display duration in addition to the message.

---

# 📰 External references

n8n Matrix Display is documented and referenced on several sites outside GitHub.

---

# 🛠️ Hackster.io

## n8n Matrix Display

**[n8n Matrix Display — Hackster.io](https://www.hackster.io/roni-bandini/n8n-matrix-display-6690ab)**

The complete English-language project tutorial covers:

* Beetle ESP32-C3
* dual WS2812B matrices
* n8n workflow
* AI Agent
* Gmail
* Marketstack
* Gemini
* parking-space analysis
* ESP32 firmware
* 3D enclosure
* retro animations

The Hackster project links back to this repository as the source-code location.

---

# 🛠️ DFRobot Maker Community

## n8n matrix display with Beetle ESP32C3

**[DFRobot Maker Community](https://community.dfrobot.com/makelog-318250.html)**

DFRobot's Maker Community also documents the project.

The article covers:

* hardware
* enclosure
* circuit
* n8n workflow
* ESP32 configuration
* retro game animations

---

# ✍️ Medium

## Display para n8n con ESP32 y WS2812B

**[Spanish-language article on Medium](https://bandini.medium.com/display-para-n8n-con-esp32-y-ws2812b-31fad68f1e68)**

Spanish-language article about the project, linked directly from the GitHub repository's **About** section.

---

# 🖨️ Cults3D

## n8n matrix display with ESP32 and WS2812B

**[Download the 3D enclosure](https://cults3d.com/en/3d-model/gadget/n8n-matrix-display-with-esp32-and-ws2812b)**

The Cults3D publication contains:

* completed-project photographs
* circuit image
* demo link
* five STL enclosure files
* hardware list
* links to the project tutorial

The Cults3D hardware list also preserves the same product/referral URLs used in this repository.

---

# 🎥 YouTube

## Project demo

**[n8n Matrix Display — YouTube Short](https://www.youtube.com/shorts/Qm-eOpMM7Nw)**

Short demonstration of the finished matrix, scrolling information, and animations.

---

# 📚 Useful references

* **[n8n](https://n8n.io/)**
* **[n8n Documentation](https://docs.n8n.io/)**
* **[Adafruit NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix)**
* **[Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)**
* **[ArduinoJson](https://arduinojson.org/)**
* **[Google AI Studio](https://aistudio.google.com/)**

---

# 🔗 You may also be interested in...

Other projects by **Roni Bandini** involving n8n, ESP32 hardware, AI agents, and physical interfaces.

## 🖥️🔘 n8n Terminal

**A dedicated physical screen-and-button interface for n8n workflows.**

It can trigger workflows, receive information from n8n, display responses, show QR codes, and add physical interaction to AI-agent automations.

**[github.com/ronibandini/n8nTerminal](https://github.com/ronibandini/n8nTerminal)**

---

## 🔘⚡ ESP32N8NButton

**Arduino library for triggering n8n workflows from a physical ESP32 button.**

A minimal approach to ESP32-to-n8n integration using HTTP webhooks.

**[github.com/ronibandini/ESP32N8NButton](https://github.com/ronibandini/ESP32N8NButton)**

---

## 🧠📈 RUBIK Pi 3 Anomaly Detection

**Edge Impulse anomaly detection on RUBIK Pi 3 integrated with n8n.**

Another example of using n8n as the automation layer connecting physical hardware, Machine Learning, and higher-level workflows.

**[github.com/ronibandini/rubikpi3-anomaly-detection](https://github.com/ronibandini/rubikpi3-anomaly-detection)**

---

# 🔐 Security notes

The project is intended for experimentation and trusted environments.

## Wi-Fi credentials

The ESP32 firmware currently stores:

```cpp
const char* ssid = "";
const char* password = "";
```

directly in the sketch.

Do not publish configured credentials.

---

## Webhook URL

The n8n webhook URL is also embedded in the firmware.

Depending on how your workflow is configured, anyone with access to that endpoint may be able to trigger executions.

Consider authentication or another access-control mechanism for public deployments.

---

## TLS verification

The current firmware uses:

```cpp
client.setInsecure();
```

This allows HTTPS connections without validating the remote server certificate.

It simplifies ESP32 deployment but removes normal TLS certificate verification.

For higher-security environments, configure proper certificate validation.

---

## n8n credentials

Keep service credentials such as:

```text
Gmail OAuth
Gemini API
Marketstack API
OpenWeatherMap API
```

inside n8n's credential system.

Do not hard-code those credentials into the ESP32 firmware or commit them to GitHub.

---

# 📜 License

n8n Matrix Display is released under the **MIT License**.

See [`LICENSE`](LICENSE) for details.

---

# 👤 Author

**Roni Bandini**

Maker, AI developer, electronic artist and writer.

* 🐙 GitHub: **[@ronibandini](https://github.com/ronibandini)**
* 📸 Instagram: **[@ronibandini](https://www.instagram.com/ronibandini/)**
* 🐦 X: **[@RoniBandini](https://x.com/RoniBandini)**
* ✍️ Medium: **[bandini.medium.com](https://bandini.medium.com/)**
* 🛠️ Hackster: **[Roni Bandini](https://www.hackster.io/roni-bandini)**

Contributions, forks, alternative feeds, animations, enclosures, and n8n integrations are welcome.


