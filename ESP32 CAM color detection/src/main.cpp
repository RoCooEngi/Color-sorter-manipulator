#include <Arduino.h>
#include <esp_camera.h>
#include <math.h>

// Pinout
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} RGB;

typedef struct {
  RGB grid[3][3]; // 3x3 grid of average colors
} RGBGrid;

void setupCamera() {
  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_240X240;
  config.jpeg_quality = 8;
  config.fb_count = 1;

  while (esp_camera_init(&config) != ESP_OK){
    Serial.println(F("Camera initialization failed"));
    delay(500);
  }
  delay(100);

  sensor_t* s = esp_camera_sensor_get();
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 1);
  s->set_exposure_ctrl(s, 0);
  s->set_aec_value(s, 3000);
  Serial.println(F("Camera initialized successfully"));
}

void processRGB565Grid(uint8_t *buf, uint32_t width, uint32_t height, RGBGrid *grid) {
  const int gridSize = 3;
  const int cellWidth = width / gridSize;
  const int cellHeight = height / gridSize;
  const int step = 2;
  
  for (int gy = 0; gy < gridSize; gy++) {
      for (int gx = 0; gx < gridSize; gx++) {
          // Calculates the boundaries for the current grid cell
          int startX = gx * cellWidth;
          int endX = (gx + 1) * cellWidth;
          int startY = gy * cellHeight;
          int endY = (gy + 1) * cellHeight;
          
          unsigned long rSum = 0, gSum = 0, bSum = 0;
          int pixelCount = 0;
          
          // Sample pixels in the current grid cell
          for (int y = startY; y < endY; y += step) {
              for (int x = startX; x < endX; x += step) {
                  uint16_t pixel = ((uint16_t)buf[(y * width + x) * 2] << 8) | buf[(y * width + x) * 2 + 1];
                  
                  uint8_t r = (pixel >> 11) & 0x1F; // 5 bit red
                  uint8_t g = (pixel >> 5) & 0x3F; // 6 bit green
                  uint8_t b = pixel & 0x1F; // 6 bit blue
                  
                  // Conversion to 8 bit
                  r = (r << 3) | (r >> 2);
                  g = (g << 2) | (g >> 4);
                  b = (b << 3) | (b >> 2);
                  
                  rSum += r;
                  gSum += g;
                  bSum += b;
                  pixelCount++;
              }
          }
          
          // Calculates the average color value for a given grid cell
          if (pixelCount > 0) {
              grid->grid[gy][gx].r = rSum / pixelCount;
              grid->grid[gy][gx].g = gSum / pixelCount;
              grid->grid[gy][gx].b = bSum / pixelCount;
          }
      }
  }
}

int isColorDominant(int r, int g, int b, int brightnessThreshold, float saturationThreshold) {
  // Checking if a color is saturated enough
  int maxVal = fmax(fmax(r, g), b);
  int minVal = fmin(fmin(r, g), b);
  int brightness = (r + g + b) / 3;
  float saturation = (maxVal == 0) ? 0 : (float)(maxVal - minVal) / maxVal;

  return (brightness >= brightnessThreshold && saturation >= saturationThreshold);
}

const char* predominantColor(RGBGrid *grid) {
    int redCount = 0, greenCount = 0, blueCount = 0;
    const int brightnessThreshold = 20;
    const float saturationThreshold = 0.2f;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            RGB color = grid->grid[y][x];
            int r = color.r, g = color.g, b = color.b;

            if (!isColorDominant(r, g, b, brightnessThreshold, saturationThreshold)) {
                continue;
            }

            // Determine which channel is dominant
            if (r * 0.8 > g && r * 0.8 > b && r * 0.8 > (g + b) * 0.6) {
                redCount++;
            } else if (g > r && g > b && g > (r + b) * 0.6) {
                greenCount++;
            } else if (b > r && b > g && b > (r + g) * 0.6) {
                blueCount++;
            }
        }
    }

    if (redCount == 0 && greenCount == 0 && blueCount == 0) {
        return "-";  // No dominant color
    }

    // Return the color with the maximum number of occupied cells
    if (redCount >= greenCount && redCount >= blueCount) return "red";
    if (greenCount >= redCount && greenCount >= blueCount) return "green";
    if (blueCount >= redCount && blueCount >= greenCount) return "purple";

    return "-";
}

const char* color3(const RGB* color, int num = 3) {
  if (color->r == num) return "RED";
  else if (color->g == num) return "GREEN";
  else if (color->b == num) return "PURPLE";
  return NULL;
}

RGBGrid colorGrid;
RGB result_color{0, 0, 0};
const char* send_result;
bool working = true;
String arduino_value;

void setup() {
  Serial.begin(115200);
  setupCamera();
}

void loop() {
  // Shooting
  if (!working) return;
  delay(200);
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println(F("Camera capture failed"));
      return;
  }
  delay(100);
  
  // Getting the total RGB array
  processRGB565Grid(fb->buf, fb->width, fb->height, &colorGrid);
  // Вычисление доминирующего цвета
  const char* result = predominantColor(&colorGrid);
  Serial.println(result);

  // The Arduino will accept a color if the camera detects it three times in a row.
  if (strcmp(result, "red") == 0) {
    if (result_color.g != 0 || result_color.b != 0) {
      result_color.g = 0;
      result_color.b = 0;
    }
    result_color.r += 1;
  } else if (strcmp(result, "green") == 0) {
    if (result_color.r != 0 || result_color.b != 0) {
      result_color.r = 0;
      result_color.b = 0;
    }
    result_color.g += 1;
  } else if (strcmp(result, "purple") == 0) {
    if (result_color.r != 0 || result_color.g != 0) {
      result_color.r = 0;
      result_color.g = 0;
    }
    result_color.b += 1;
  } else {
    result_color.r = 0;
    result_color.g = 0;
    result_color.b = 0;
  }
  
  send_result = color3(&result_color);
  if (send_result) {
    Serial.println(send_result);
    result_color.r = 0;
    result_color.g = 0;
    result_color.b = 0;
  }
  esp_camera_fb_return(fb);

  while (arduino_value != "go") {
    // Accepts message from Arduino to make photos
    if (Serial.available()) {
      arduino_value = Serial.readStringUntil('\n');
      arduino_value.trim();
    }
    else {
      Serial.println(F("UART disconnected"));
      return;
    }
  }
  arduino_value = "";
  delay(500);
}