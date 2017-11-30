#include <Adafruit_NeoPixel.h>

const int BUTTON_PIN_01 = 0;
const int NEOPIXEL_PIN_01 = 5;
const int NEOPIXEL_LED_NUM = 10;

/**
 * Button.
 * 押されたかを一定間隔でチェックするやつ。
 */
class Button {
private:
  // define interval in millisec
  const int INTERVAL = 200;

  int pin;
  boolean status;
  boolean beforeStatus;

  long lastTime;
  long currentTime;

public:
  Button( int pin_number ) {
    pin = pin_number;
    pinMode(pin, INPUT);
    beforeStatus = false;
    lastTime = millis();
    currentTime = millis();
  }

  boolean IsPushed () {
    currentTime = millis();
    if ( currentTime - lastTime > INTERVAL ) {
      // The time is comming!!
      status = digitalRead(pin) == HIGH;
      if ( status && !beforeStatus ) {
        // pushed
        return true;
      }
      beforeStatus = status;
    }
    return false;
  }
};

class NeoPixelFlasher {
private:
  // define interval in millisec
  const int INTERVAL = 200;

  const int DEFAULT_R = 0;
  const int DEFAULT_G = 0;
  const int DEFAULT_B = 0;

  const int FULL_R = 100;
  const int FULL_G = 71;
  const int FULL_B = 148;

  Adafruit_NeoPixel controller;
  int pin;             // ピン番号
  int pixelNum;        // 電球何個か
  int currentLEDIdx;   // いま制御中のLEDの番号
  boolean flashing;    // 今光らせている状態かどうか

  int currentMode;     // モード

  long lastTime;
  long currentTime;
  int currentIdx;

  int current_R;
  int current_G;
  int current_B;

  void Initialize () {
    lastTime = millis();
    currentTime = millis();
    currentLEDIdx = 0;
    
    currentMode = 0;
    currentIdx = 0;

    current_R = 0;
    current_G = 0;
    current_B = 0;

    controller = Adafruit_NeoPixel( pixelNum, pin );

    controller.begin();
    controller.show();
  }

public:
  NeoPixelFlasher ( int pin_number, int number_of_led ) {
    pin = pin_number;
    pixelNum = number_of_led;
    flashing = false;

    Initialize();
  }

  void SwitchMode() {
    if (currentMode++ >= 2) {
      currentMode = 0;
      flashing = false;
    }
  }

  void Flash () {
    flashing = true;
    Initialize();
  }

  void Update () {
    currentTime = millis();
    if ( flashing ) {
      if ( lastTime - currentTime > INTERVAL ) {
        // The time is comming!!!
        for ( int i = 0; i < pixelNum; i++ ) {
          // これからのやつ
          if ( i < currentLEDIdx ) {
            switch(currentMode) {
            case 0:
              current_R = DEFAULT_R;
              current_G = DEFAULT_G;
              current_B = DEFAULT_B;
              break;
            case 1:
              current_R = FULL_R;
              current_G = FULL_G;
              current_B = FULL_B;
              break;
            }
          }

          // もう終わったやつ
          if ( i > currentLEDIdx ) {
            switch(currentMode) {
            case 0:
              current_R = FULL_R;
              current_G = FULL_G;
              current_B = FULL_B;
              break;
            case 1:
              current_R = DEFAULT_R;
              current_G = DEFAULT_G;
              current_B = DEFAULT_B;
              break;
            }
          }

          // 今やってるやつ
          if ( i == currentLEDIdx ) {
            switch(currentMode) {
            case 0:
              if ( current_R <= FULL_R ) {
                current_R++;
              }
              if ( current_G <= FULL_G ) {
                current_G++;
              }
              if ( current_B <= FULL_B ) {
                current_B++;
              }
              break;
            case 1:
              if ( current_R >= DEFAULT_R ) {
                current_R--;
              }
              if ( current_G >= DEFAULT_G ) {
                current_G--;
              }
              if ( current_B >= DEFAULT_B ) {
                current_B--;
              }
              break;
            default:
              break;
            }
          controller.setPixelColor( i, current_R, current_G, current_B );
          controller.show();
          }
        }
  
        if ( currentMode == 0 && current_R >= FULL_R && current_G >= FULL_G && current_B >= FULL_B ) {
          currentLEDIdx = currentLEDIdx == pixelNum - 1 ? 0 : currentLEDIdx + 1;
          if ( currentLEDIdx == pixelNum - 1 ) {
            SwitchMode();
          }
        }
      }
      controller.show();
    }
  }

  boolean Processing () {
    return flashing == true;
  }
};


Button button_01 ( BUTTON_PIN_01 );
NeoPixelFlasher neoPixel_01 ( NEOPIXEL_PIN_01, NEOPIXEL_LED_NUM );

void setup () {
  // serial setting
  Serial.begin(115200);
  Serial.println("start");

}

void loop () {
  if ( button_01.IsPushed() && !neoPixel_01.Processing() ) {
    neoPixel_01.Flash();
  }

  neoPixel_01.Update();
}
