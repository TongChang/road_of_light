// Debugする際にコメントアウトをはずしてください
// #define DEBUG

#include <Adafruit_NeoPixel.h>
#include <ESP8266IFTTT.h>
#include <ESP8266WiFi.h>

#define IFTTT_WEBHOOK_NAME "IFTTTのWebhookの名前"
#define IFTTT_KEY "IFTTTのWebhook Key"

#define WIFI_SSID "接続するWiFiのSSID"
#define WIFI_PASSWORD "接続するWiFiのPassword"

#define PURPLE_R 130
#define PURPLE_G 65
#define PURPLE_B 130

#define OFF_R 0
#define OFF_G 0
#define OFF_B 0

#define NEOPIXEL_PIN 5
#define NUMBER_OF_PIXEL 10

#define BUTTON_PIN 12

#define POWER_LED_PIN 4

class LEDFlasher {
  private:
    unsigned int pin;

  public:
    LEDFlasher( unsigned int pin_number ) {
      pin = pin_number;
      pinMode(pin, OUTPUT);
    }
  
    void On() {
      digitalWrite(pin, HIGH);
    }

    void blink() {
      delay(200);
      digitalWrite(pin, LOW);
      delay(200);
      digitalWrite(pin, HIGH);
      delay(200);
      digitalWrite(pin, LOW);
      delay(200);
      digitalWrite(pin, HIGH);
      delay(200);
      digitalWrite(pin, LOW);
      delay(200);
      digitalWrite(pin, HIGH);
    }
};

class Button {
private:
  // define interval in millisec
  const unsigned int INTERVAL = 200;

  unsigned int pin;
  boolean status;
  boolean beforeStatus;

  unsigned long lastTime;
  unsigned long currentTime;

public:
  Button( unsigned int pin_number ) {
    pin = pin_number;
    pinMode(pin, INPUT);
    beforeStatus = false;
    lastTime = millis();
    currentTime = millis();
  }

  boolean isPushed () {
    currentTime = millis();
    if ( currentTime - lastTime > INTERVAL ) {
      // The time is comming!!
      status = digitalRead(pin) == HIGH;
      if ( status && !beforeStatus ) {
#ifdef DEBUG
    Serial.print("pushed.");
#endif
        // pushed
        return true;
      }
      beforeStatus = status;
    }
    return false;
  }
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_PIXEL, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Button button( BUTTON_PIN );
LEDFlasher power_led( POWER_LED_PIN );

/**
 * modeOfLights = 0 の場合、B ⇒ G ⇒ R の順番で紫色にしていく
 * modeOfLights = 1 の場合、B ⇒ G ⇒ R の順番で消していく
 * 上記を決まった数回したら終了する
 */
void purpleChase() {

  boolean flashing = true;
  unsigned int modeOfLights = 0; // 点灯はじまり
  unsigned int currentRoudTime = 0;
  unsigned int maxRoudTime = 2;

  unsigned int           currentPixel = 0;               // 何個目が点いたか

  unsigned int nowBrightnessLevel_R = OFF_R;
  unsigned int nowBrightnessLevel_G = OFF_R;
  unsigned int nowBrightnessLevel_B = OFF_R;

#ifdef DEBUG
  Serial.println("purpleChase start.");
#endif

  while( flashing ) {
    if ( modeOfLights == 0 ) {
      // だんだん明るくする
      if ( nowBrightnessLevel_B < PURPLE_B ) {
        nowBrightnessLevel_B++;
      } else if ( nowBrightnessLevel_G < PURPLE_G ) {
        nowBrightnessLevel_G++;
      } else if ( nowBrightnessLevel_R < PURPLE_R ) {
        nowBrightnessLevel_R++;
      }

      for ( int i = 0; i < NUMBER_OF_PIXEL; i++ ) {
        if ( i < currentPixel ) {
          strip.setPixelColor( i, strip.Color(PURPLE_R, PURPLE_G, PURPLE_B) );
       } else if ( i == currentPixel ) {
          strip.setPixelColor( i, strip.Color(nowBrightnessLevel_R, nowBrightnessLevel_G,nowBrightnessLevel_B) );
        } else {
          strip.setPixelColor( i, strip.Color(OFF_R, OFF_G, OFF_B) );
        }
      }
    } else if ( modeOfLights == 1 ) {
      // だんだん暗くする
      if ( nowBrightnessLevel_R > OFF_R ) {
        nowBrightnessLevel_R--;
      } else if ( nowBrightnessLevel_G > OFF_G ) {
        nowBrightnessLevel_G--;
      } else if ( nowBrightnessLevel_B > OFF_B ) {
        nowBrightnessLevel_B--;
      }

      for ( int i = 0; i < NUMBER_OF_PIXEL; i++ ) {
        if ( i < currentPixel ) {
          strip.setPixelColor( i, strip.Color(OFF_R, OFF_G, OFF_B) );
        } else if ( i == currentPixel ) {
          strip.setPixelColor( i, strip.Color(nowBrightnessLevel_R, nowBrightnessLevel_G,nowBrightnessLevel_B) );
        } else {
          strip.setPixelColor( i, strip.Color(PURPLE_R, PURPLE_G, PURPLE_B) );
        }
      }
    }
    strip.show();

    // 色が全開になったら次のpixel
    if ( ( modeOfLights == 0 && nowBrightnessLevel_R >= PURPLE_R && nowBrightnessLevel_G >= PURPLE_G && nowBrightnessLevel_B >= PURPLE_B )
      || ( modeOfLights == 1 && nowBrightnessLevel_R <= OFF_R && nowBrightnessLevel_G <= OFF_G && nowBrightnessLevel_B <= OFF_B ) ) {

      // Pixel切り替え
      currentPixel++;

      if ( currentPixel >= NUMBER_OF_PIXEL ) {
#ifdef DEBUG
  Serial.println("mode change.");
#endif
        modeOfLights++;
        if ( modeOfLights > 1 ) {
          modeOfLights = 0;
        }

        // 一周した
        if ( modeOfLights == 0 ) {
#ifdef DEBUG
  Serial.println("one cycle done.");
#endif
          currentRoudTime++;
          // 終了まで回った
          if ( currentRoudTime >= maxRoudTime ) {
#ifdef DEBUG
  Serial.println("finish.");
#endif
            flashing = false;
          }
        }
        currentPixel = 0;
      }

      // brightnessLevelのリセット
      if ( modeOfLights == 0 ) {
        nowBrightnessLevel_R = OFF_R;
        nowBrightnessLevel_G = OFF_G;
        nowBrightnessLevel_B = OFF_B;
      } else if (modeOfLights == 1) {
        nowBrightnessLevel_R = PURPLE_R;
        nowBrightnessLevel_G = PURPLE_G;
        nowBrightnessLevel_B = PURPLE_B;
      }
    }

    delay(30);
  }
#ifdef DEBUG
  Serial.println("purpleChase end.");
#endif
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("");
#endif
  delay(1000);
#ifdef DEBUG
  Serial.println("serial initialize done.");
#endif

  strip.begin();
  strip.show();

  delay(1000);
#ifdef DEBUG
  Serial.println("strip initialize done.");
#endif

  WiFi.mode(WIFI_STA);
  power_led.On();
#ifdef DEBUG
  Serial.println("start.");
#endif
}

void loop() {
  if ( button.isPushed() ) {
    power_led.blink();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
    }
    IFTTT.trigger( IFTTT_WEBHOOK_NAME, IFTTT_KEY );
    WiFi.disconnect();
    purpleChase();
  }
  delay(100);
}

