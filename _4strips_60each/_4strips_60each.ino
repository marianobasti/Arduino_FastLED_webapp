#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 60
#define STRIPS 4
#define BRIGHTNESS  255
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;
bool isOn = false;
int alt = 0;
String data;
int secuencia = 0;
int bpm = 100;
int brillo;
int temp;
double h;
double s;
double l;

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// #define DATA_PIN 3
// #define CLOCK_PIN 13

// Define the array of leds
CRGB leds[STRIPS][NUM_LEDS];

CRGBPalette16 gPal;

void setup() {
  delay(1500); // sanity delay
  Serial.begin(9600);
  Serial.setTimeout(50);
  FastLED.setBrightness( BRIGHTNESS );
  gPal = CRGBPalette16( CRGB::Black, CRGB::DarkGreen, CRGB::YellowGreen,  CRGB::LightGreen);
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812B, 12, GRB>(leds[3], NUM_LEDS);
  FastLED.addLeds<WS2812B, 11, GRB>(leds[2], NUM_LEDS);
  FastLED.addLeds<WS2812B, 10, GRB>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2812B, 9, GRB>(leds[0], NUM_LEDS);
  while (!Serial.available()) {
  }
  Serial.print("OK");
  Serial.flush();
  secuenciaEncendido();
}

void loop() {

  if (Serial.available()) {
    secuencia = Serial.readStringUntil(',').toInt();
    temp = Serial.readStringUntil(',').toDouble();
    brillo = map(temp, 0, 100, 0, 255);
    bpm = Serial.readStringUntil(',').toInt();
    temp = Serial.readStringUntil(',').toDouble() * 100;
    h = map(temp, 0, 100, 0, 255);
    temp = Serial.readStringUntil(',').toDouble() * 100;
    s = map(temp, 0, 100, 0, 255);
    temp = Serial.readStringUntil(',').toDouble() * 100;
    l = map(temp, 0, 100, 0, 255);
  }

  FastLED.setBrightness( brillo );

  switch (secuencia) {
    case 0:
      for (int i = 0; i<STRIPS;i++) {
        fadeToBlackBy( leds[i], NUM_LEDS, 70);
      }
      break;
    case 1:
      solid(h, s, l);
      break;
    case 2:
      strobe(bpm, s, l);
      break;
    case 3:
      strobe_alternate(bpm, s, l);
      break;
    case 4:
      confetti(h, s);
      break;
    case 5:
      Fire2012WithPalette(h, s);
      break;
    case 6:
      sinelon(bpm);
      break;
    case 7:
      sinelon2(bpm, h);
      break;
    case 8:
      rainbow();
      break;
    case 9:
      sweep(h, s, l);
      break;
  }

  random16_add_entropy( random());

  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}

#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 100


void Fire2012WithPalette(int h, int s)
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    CRGB color = CHSV(h, colorindex, colorindex);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    for (int i = 0; i<STRIPS;i++) {
    leds[i][pixelnumber] = color;
    }
  }
}


void confetti(int h, int s)
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds[0], NUM_LEDS, 70);
  fadeToBlackBy( leds[1], NUM_LEDS, 70);
  int pos = random16(NUM_LEDS);
  int hue = beatsin16( h, 0, 80 );
  int sat = beatsin16( h, 100, 20 );
  for (int i = 0; i<STRIPS;i++) {
    leds[i][pos] += CHSV(h+hue-40, 80 + sat, 255);
  }
}

void strobe(int bpm, int s, int l) {
  if (isOn == false ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      for (int i = 0; i<STRIPS;i++) {
        leds[i][i] = CHSV(h, s, l);
      }
    }
    isOn = true;
    delay((2000/bpm)+1);

  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      for (int j = 0; j<STRIPS;j++) {
        leds[j][i] = CRGB::Black;
      }
    }
    isOn = false;
    for (int i = 0; i<STRIPS;i++) {
      fadeToBlackBy( leds[i], NUM_LEDS, 50);
    }
  }
}

void strobe_alternate(int bpm, int s, int l) {
  if (isOn == false ) {
    if (alt == 0 || alt == 1) {
      for (int j = 0; j < NUM_LEDS; j++) {
        for (int i = 0; i<STRIPS/2;i++) {
          leds[i][j] = CHSV(h, s, l);
        }
      }
      alt++;
    } else if (alt == 2) {
      for (int j = 0; j < NUM_LEDS; j++) {
        for (int i = STRIPS/2; i<STRIPS;i++) {
          leds[i][j] = CHSV(h, s, l);
        }
      }
      alt++;
    } else {
      for (int j = 0; j < NUM_LEDS; j++) {
        for (int i = STRIPS/2; i<STRIPS;i++) {
          leds[i][j] = CHSV(h, s, l);
        }
      }
      alt = 0;
    }
    isOn = true;
    delay((2000/bpm)+1);

  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      for (int j = 0; j<STRIPS;j++) {
        leds[j][i] = CRGB::Black;
      }
    }
    isOn = false;
  }
}

void sinelon(int bpm)
{
  // a colored dot sweeping back and forth, with fading trails
  for (int i = 0; i<STRIPS;i++) {
    fadeToBlackBy( leds[i], NUM_LEDS, 50);
  }
  int pos0 = beatsin16( bpm / 5, 0, NUM_LEDS - 1 );
  int pos1 = beatsin16( bpm / 5, 0, NUM_LEDS - 1 );
  for (int i = 0; i<STRIPS;i++) {
    leds[i][pos1] += CHSV( pos0 * 10, 255, 192);
  }
}

void sinelon2(int bpm, int h)
{
  // 2 colored dots sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS, 50);
  fadeToBlackBy( leds[1], NUM_LEDS, 50);
  int pos[STRIPS] = {beatsin16( bpm / 2, 0, NUM_LEDS - 1 ),beatsin16( bpm / 3, 0, NUM_LEDS - 1 ),beatsin16( bpm / 4, 0, NUM_LEDS - 1 ),beatsin16( bpm / 5, 0, NUM_LEDS - 1 )};
  for (int i = 0; i<STRIPS;i++) {
    leds[i][pos[i]] += CHSV( map(pos[i], 0, NUM_LEDS, -10, 10) + h, 255, 192);
  };
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  for (int i = 0; i<STRIPS;i++) {
    fill_rainbow( leds[i], NUM_LEDS, 10, 7);
  }
}

void solid(int h, int s, int l) {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j<STRIPS;j++) {
      leds[j][i] = CHSV(h, s, l);
    }
  }
}

void secuenciaEncendido() {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j<STRIPS;j++) {
      leds[j][i] = CRGB::White;
    }
    delay(10);
  }
}

void sweep(int h, int s, int l) {
  for (int i = 0; i < NUM_LEDS*1.5; i++) {
    if (i<NUM_LEDS) {
      for (int j = 0; j<STRIPS;j++) {
        leds[j][i] = CHSV(h, s, l);
      }
    }
    if (i-NUM_LEDS/2<0) {
      for (int j = 0; j<STRIPS;j++) {
      leds[j][i-(NUM_LEDS/2)] = CHSV(h, s, l);
    }
    }
    for (int i = 0; i<STRIPS;i++) {
      fadeToBlackBy(leds[i], NUM_LEDS, 80);
    }
    delay(30);
  }
}
