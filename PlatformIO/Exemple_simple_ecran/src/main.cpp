#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include "colors.h"
#include "bitmap.h"
#include <ArduinoSTL.h>

//Par Romain THOMAS le 25 juillet 2019
//https://github.com/romain894

//Ce code a pour but de montrer les possibilites de programmation d'un ecran led
//8x8 RGB. Cet exemple montre comment on pourrait programmer l'ecran pour s'en
//servir pour afficher du texte et des images. Vous pouvez envoyer ce code sur
//la carte Arduino nano pour voir ce qu'il fait. N'hesitez pas a le modifier afin
//de l'adapter a vos envies !

#define PIN 6

//taille de la matrice :
#define mw 8
#define mh 8

//declaration de la matrice :
//changer NEO_MATRIX_LEFT par NEO_MATRIX_RIGHT en fonction de la position du
//permier pixel relie
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(mw, mh, PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

//textes a afficher :
char texte1[] = "UTT 2019";//Ne pas mettre plus de 10 a 13 carracteres
uint8_t nb_char_texte1 = 8;

void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h);
void display_rgbBitmap(uint8_t bmp_num);



//definition des fonctions qui seront utilisees dans le code :
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
  uint16_t RGB_bmp_fixed[w * h];
  for (uint16_t pixel=0; pixel<w*h; pixel++) {
  	uint8_t r,g,b;
  	uint16_t color = pgm_read_word(bitmap + pixel);

  	//Serial.print(color, HEX);
  	b = (color & 0xF00) >> 8;
  	g = (color & 0x0F0) >> 4;
  	r = color & 0x00F;
  	//Serial.print(" ");
  	//Serial.print(b);
  	//Serial.print("/");
  	//Serial.print(g);
  	//Serial.print("/");
  	//Serial.print(r);
  	//Serial.print(" -> ");
  	// expand from 4/4/4 bits per color to2020 5/6/5
  	b = map(b, 0, 15, 0, 31);
  	g = map(g, 0, 15, 0, 63);
  	r = map(r, 0, 15, 0, 31);
  	//Serial.print(r);
  	//Serial.print("/");
  	//Serial.print(g);
  	//Serial.print("/");
  	//Serial.print(b);
  	RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
  	//Serial.print(" -> ");
  	//Serial.println(RGB_bmp_fixed[pixel], HEX);
  }
  matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

void display_rgbBitmap(uint8_t bmp_num) {
  static uint16_t bmx,bmy;

  fixdrawRGBBitmap(bmx, bmy, RGB_bmp[bmp_num], 8, 8);
  bmx += 8;
  if (bmx >= mw) bmx = 0;
  if (!bmx) bmy += 8;
  if (bmy >= mh) bmy = 0;
  matrix->show();
}

void display_scrollText(char texte[], uint8_t nb_char, uint16_t couleur, uint16_t vitesse) {
  matrix->clear();
  matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
  matrix->setTextSize(1);
  matrix->setRotation(0);
  for (int8_t x=7; x>=8-10*nb_char; x--) {
    matrix->clear();
    matrix->setCursor(x,0);
    matrix->setTextColor(couleur);
    matrix->print(texte);
    matrix->show();
    delay(vitesse);
  }
  matrix->setRotation(0);
  matrix->setCursor(0,0);
  matrix->show();
}

//Coce a modifier selon vos envies :
void setup() { //code utilise au demarrage
  Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  matrix->begin(); //initialisation de la matrice de led
  matrix->setTextWrap(false);
  matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255)); //on relge la
  //luminosite en fonction du potentiometre sur le circuit
  matrix->fillScreen(LED_WHITE_HIGH); //on regle l'ecran en blanc
  matrix->show(); //on affiche ce qu'on a regle avant
  delay(1000); //on attend une seconde
}

void loop() {
  //Affciher les bitmap (aka pixel art) du permier au dernier (donc du 0 au 1)
  for (uint8_t i = 0; i < 2; i++) { //on va de 0 a 1 pour afficher les 2 images
    display_rgbBitmap(i); //on affiche l'image i
    delay(2000); //on attend 2 secondes
  }

  display_scrollText(*texte1, nb_char_texte1, LED_BLUE_HIGH, 100); //on affiche
  // le texte contenu dans text1 en bleu avec un delai de 100ms

}
