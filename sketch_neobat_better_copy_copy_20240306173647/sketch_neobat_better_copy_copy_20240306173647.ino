#include <Adafruit_NeoPixel.h>
int num_pixels = 8;
int pin_pixels = 2;
int pin_button = 3;
int button_reading;
int lightstate = 0;
int buttonstate = -1;
unsigned long buttonmillis = 0;
unsigned long lightmillis = 0;
int lightcycle = 1;
int lightbright = 10;
int juststoppedramp = 0;
int juststartedlight = 0;
int lightpause = 0;
float lightvar = 1.0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(num_pixels, pin_pixels, NEO_GRBW + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(pin_button, INPUT_PULLUP);
  strip.begin();
  strip.clear();
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  button_reading = digitalRead(pin_button);
  if (button_reading == LOW) {
    if (buttonstate == -1){
      // Just pressed
      juststartedlight = 0;
      buttonstate = 1;
      buttonmillis = millis();
      if (lightstate == 2){
        // Fast stop the ramp
        lightstate = 3;
        juststoppedramp = 1;
        lightpause = 0;
      }
    }else{
      if (millis() > buttonmillis + 1000){
        // Still down, held 1 sec
        if (juststartedlight == 0){
          if (lightstate == 3){
            lightstate = 2;
            juststartedlight = 1;
          }else{
            lightstate = 3;
            //lightbright = 10;
            juststartedlight = 1;
          }
        }
      }
    }
  }else{
    if (buttonstate == 1){
      // Just released
      if (lightstate == 2){
        lightstate = 3;
        lightpause = 0;
      }
      if (millis() > buttonmillis + 1000){
        // Held at least 1 sec
      }else{
        // Held less than 1 sec
        if (juststoppedramp == 1){
          juststoppedramp = 0;
        }else{
          lightstate = lightstate != 0 ? 0 : 1;
        }
      }
      buttonstate = -1;
      buttonmillis = millis();
    }else{
      // Has been released for a time
      unsigned long lightlen = lightstate == 1 ? 5000 : 60000;
      if (millis() > lightlen + buttonmillis){
        lightstate = 0;
      }
    }
  }
  setlight();
}

void setlight(){
  if (lightstate == 1){
    float vccv = getVCC()/1000.0;
    // Charge	voltage from https://www.jackery.com/blogs/knowledge/lithium-ion-battery-voltage-chart
    float vval = 0.0;
    if (vccv > 3.35){
      vval = 1.0;
    }else if (vccv > 3.32){
      vval = 0.9;
    }else if (vccv > 3.30){
      vval = 0.8;
    }else if (vccv > 3.27){
      vval = 0.7;
    }else if (vccv > 3.26){
      vval = 0.6;
    }else if (vccv > 3.25){
      vval = 0.5;
    }else if (vccv > 3.22){
      vval = 0.4;
    }else if (vccv > 3.20){
      vval = 0.3;
    }else if (vccv > 3.00){
      vval = 0.2;
    }else if (vccv > 2.50){
      vval = 0.1;
    } else {
      vval = 0.0;
    }

    int numlights = vval*8;
    uint8_t rval = 0; //255*2*(1.0-vval);
    uint8_t gval = 0; //255;
    uint8_t bval = 0; //255;
    if (vval < 0.125){
      rval = 200;
      gval = 0;
      numlights = 1;
    }else if (vval < 0.25){
      rval = 200;
      gval = 50;
      numlights = 2;
    }else if (vval < 0.375){
      rval = 200;
      gval = 100;
      numlights = 3;
    }else if (vval < 0.5){
      rval = 200;
      gval = 150;
      numlights = 4;
    }else if (vval < 0.625){
      rval = 200;
      gval = 200;
      numlights = 5;
    }else if (vval < 0.75){
      rval = 150;
      gval = 200;
      numlights = 6;
    }else if (vval < 0.875){
      rval = 100;
      gval = 200;
      numlights = 7;
    }else if (vval < 1.0){
      rval = 50;
      gval = 200;
      numlights = 8;
    }else{
      rval = 0;
      gval = 0;
      bval = 200;
      numlights = 8;
    }
    
    //for (int i = 0;i<numlights;i++){
      //strip.setPixelColor(i, 0, 0,0,100);
    float cscale = .2;
    strip.fill(strip.Color(rval*cscale,gval*cscale,bval*cscale,0),0,numlights);
    //}
  }else if (lightstate > 1){
    //for (int i = 0;i<8;i++){
      //strip.setPixelColor(i, lightbright*lightvar,lightbright*lightvar,lightbright*lightvar,lightbright*lightvar);
      int cval = lightbright*lightvar;
      strip.fill(strip.Color(cval,cval,cval,cval), 0, 8);
    //}
    if (lightstate == 2 && lightpause == 0){ //} && millis() > lightmillis + 1500){
      lightbright = lightbright + lightcycle;
      if (lightbright == 255 || lightbright == 10){
        lightcycle *= -1;
        lightpause = 140;
      }
    }
    if (lightpause > 0){
      if (lightpause > 135){
        lightvar = .6;
      }else if (lightpause > 130){
        lightvar = 1;
      }else if (lightpause > 125){
        lightvar = .6;
      }else if (lightpause > 6){
        lightvar = 1;
      }else if (lightpause > 1){
        lightvar = .6;
      }else{
        lightvar = 1;
      }
      lightpause -= 1;
    }
  }else{
    strip.clear();
  }
  strip.show();
  delay(16);
}

int getVCC() {
  //#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
  ADMUX = _BV(MUX3) | _BV(MUX2);  // For ATtiny85/45
  delay(2);                       // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);            // Convert
  while (bit_is_set(ADCSRA, ADSC))
    ;
  uint8_t low = ADCL;
  unsigned int val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC);  // Convert
  while (bit_is_set(ADCSRA, ADSC))
    ;
  low = ADCL;
  val = (ADCH << 8) | low;
  return ((long)1024 * 1100) / val;
}