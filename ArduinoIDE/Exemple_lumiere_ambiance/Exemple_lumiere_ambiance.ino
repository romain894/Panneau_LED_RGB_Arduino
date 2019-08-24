#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include "colors.h"

//Par Romain THOMAS le 31 juillet 2019
//https://github.com/romain894

//Ce code a pour but de montrer les possibilites de programmation d'un ecran led
//8x8 RGB. Cet exemple montre comment on pourrait programmer l'ecran pour s'en
//servir comme une lampe d'ambiance. Vous pouvez envoyer ce code sur la carte
//Arduino nano pour voir ce qu'il fait. N'hesitez pas a le modifier afin de
//l'adapter a vos envies !

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

uint8_t mode = 0; //mode actuel
const uint8_t nombreModes = 7;

uint8_t e; //vairable temporaire

uint8_t memoireBouton = HIGH; //memoire du bouton pour les modes :
// elle contient l'ancienne valeur du bouton
uint8_t pinBouton = 2; //pin pour le bouton

uint8_t led1[3] = {0, 0, 0}; //(R, G, B) couleurs des leds
uint8_t ledv1[3] = {0, 0, 0}; //couleurs voulu pour le changement progressif
uint8_t leda1[3] = {0, 0, 0}; //couleurs avant le debut du changement progressif

//Idem (pour faire une deuxieme variation en paralelle) :
uint8_t led2[3] = {0, 0, 0};
uint8_t ledv2[3] = {0, 0, 0};
uint8_t leda2[3] = {0, 0, 0};

uint8_t led3[3] = {0, 0, 0};
uint8_t ledv3[3] = {0, 0, 0};
uint8_t leda3[3] = {0, 0, 0};

uint8_t led4[3] = {0, 0, 0};
uint8_t ledv4[3] = {0, 0, 0};
uint8_t leda4[3] = {0, 0, 0};

uint8_t suiteBruijn[9] = {0, 0, 255, 255, 255, 0, 255, 0, 0};
//Cette suite permet d'obtenir toutes les combinations de couleurs composee
//uniquement de 0 et 255 sauf (0, 0, 0) qui correspond au noir, cela permet
//d'avoir des couleurs plus vives et de plus grands degrades et en ayant la meme
//sequence qui se repete
//Ceci est base sur les suites de Bruijn, voir :
//https://fr.wikipedia.org/wiki/Suite_de_de_Bruijn

uint8_t compteurBruijn = 6;

unsigned long t = millis(); //variable servant pour la mesure du temps

unsigned long delai_couleur = 1000; //delai entre chaque tirage aleatoire pour
//l'enchainement des couleurs (en ms)

void setup() { //code utilise au demarrage
  Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  matrix->begin(); //initialisation de la matrice de led
  matrix->setTextWrap(false);
  matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255)); //on relge la
  //luminosite en fonction du potentiometre sur le circuit
  randomSeed(analogRead(A7)); //initialisation de la generation aleatoire
  pinMode(pinBouton, INPUT_PULLUP); //on se sert du pin du bouton pour les
  //modes comme une entree pour un interuppteur
}

void loop() {
  e = digitalRead(pinBouton); //on lit l'etat du bouton pour les modes
  if ((e == LOW) && (memoireBouton == HIGH)) {//si on vient d'appuyer dessus
    mode++; //on change de mode
    if (mode >= nombreModes) {//si on a depasse le nombre de modes
      mode = 0; //on revient au premier mode
    }
  }
  memoireBouton = e; //on enregistre dans la memoire la valeur de l'etat du
  //bouton

  if (mode == 0) { //pour le premier mode
    if(millis() - t >= delai_couleur) //si on a fini le changement de couleur
    {
      for(int i = 0; i < 3; i++) //pour toutes les couleurs (R, G, B)
      {
        leda1[i] = led1[i]; //on actualise les valeurs de leda1[]
        ledv1[i] = random(256); //on tire au sort une nouvelle couleur
      }
      t = millis(); //on recommence un nouveau changement progressif
    }

    for(uint8_t i = 0; i < 3; i++) //pour toutes de couleurs
    {
      led1[i] = map(millis() - t, 0, delai_couleur, leda1[i], ledv1[i]); //on
      //calcul la couleur actuelle : on fait varier la couleur de leda1[] a
      //led1[] sur un interval de temps de delai_couleur. Pour savoir a quel
      //endroit on se situe sur l'interval de temps, on fait millis() - t, ce
      //qui nous permet d'avoir une valeur entre 0 et delai_couleur.
    }

    //on rempli l'ecran avec la meme couleur
    uint16_t n; //on declare la variable n
    n = matrix->numPixels(); //n = nombre de pixel
    for(uint16_t i=0; i<n; i++) matrix->setPixelColor(i, led1[0], led1[1], led1[2]);
    //on va de 0 a n pour mettre chaque pixel a la bonne couleur

    //matrix->fillScreen(matrix->Color(led1[0], led1[1], led1[2])); //si on utilise
    //les fonctions de la bibliotheque NeoMatrix (Color(), fillScreen()...), on
    //limite le nombre de couleurs possibles et donc on peut avoir des
    //changements de couleurs non fluides, vous pouvez decommenter cette ligne
    //et mettre en commentaire les 3 autres lignes pour essayer et voir la
    //difference
    matrix->show(); //on affiche ce qu'on a regle avant
  }

  if (mode == 1) {
    if(millis() - t >= delai_couleur) {
      compteurBruijn = (compteurBruijn + 1) % 7;//On passe a la couleur suivante
      for(int i = 0; i < 3; i++) {
        leda1[i] = led1[i];
        ledv1[i] = suiteBruijn[compteurBruijn + i];
      }
      t = millis();
    }

    for(uint8_t i = 0; i < 3; i++) {
      led1[i] = map(millis() - t, 0, delai_couleur, leda1[i], ledv1[i]);
    }

    uint16_t n;
    n = matrix->numPixels();
    for(uint16_t i=0; i<n; i++) matrix->setPixelColor(i, led1[0], led1[1], led1[2]);
    matrix->show();
  }

  if (mode == 2) {//on utilise le meme principe que pour le premier mode en
    //faisant varier les couleurs sur les lignes du haut et du bas. Ensuite on
    //fait un degrade du bas vers le haut
    if(millis() - t >= delai_couleur) //si on a fini le changement de couleur
    {
      for(int i = 0; i < 3; i++) //pour toutes les couleurs (R, G, B)
      {
        leda1[i] = led1[i]; //on actualise les valeurs de leda1[]
        ledv1[i] = random(2)*255;//on tire au sort une nouvelle couleur 0 ou 255
        leda2[i] = led2[i]; //pareil pour la deuxieme couleur...
        ledv2[i] = random(2)*255;
      }
      t = millis(); //on recommence un nouveau changement progressif
    }

    for(uint8_t i = 0; i < 3; i++) //pour toutes de couleurs
    {
      led1[i] = map(millis() - t, 0, delai_couleur, leda1[i], ledv1[i]); //on
      //calcul la couleur actuelle : on fait varier la couleur de leda1[] a
      //led1[] sur un interval de temps de delai_couleur. Pour savoir a quel
      //endroit on se situe sur l'interval de temps, on fait millis() - t, ce
      //qui nous permet d'avoir une valeur entre 0 et delai_couleur.
      led2[i] = map(millis() - t, 0, delai_couleur, leda2[i], ledv2[i]);
    }

    //on rempli l'ecran avec un degrade de couleurs
    for(uint16_t i = 0; i < mh; i++) //pour chaque ligne de la matrice de led
    {
      uint8_t led[3] = {0, 0, 0}; //variable pour la couleur des pixel
      for (uint8_t j = 0; j < 3; j++) {//calcul de la couleur de la ligne: R,G,B
        led[j] = map(i, 0, mh, led1[j], led2[j]); //on a une echelle de 0 a mh
        //(ici mh = 8, c'est le nombre de lignes) et i correspond a la ligne. On
        //part de la couleur led1[] en bas a la couleur led2[] en haut pour le
        //degrade. Pour chaque ligne, on fait un calcul de proportionnalité
        //avec la fonction map() et comme i va de 0 a mh, on obtient un degrade
      }
      for (uint16_t k = i*mh; k < i*mh + mw; k++) {//Pour les pixels de la ligne
        //On s'adresse au pixels avec leur numero, on obient le numero du
        //premier pixel de la ligne avec i*mh puis on va jusqu'a i*mh+mw pour
        //avoir le dernier pixel de la ligne
        matrix->setPixelColor(k, led[0], led[1], led[2]);
      }
    }
    matrix->show();
  }

  if (mode == 3) {//on utilise le meme principe que pour le troisieme mode en
    //faisant varier les couleurs sur les lignes du haut et du bas et les
    //les colones de droite et de gauche. Ensuite on fait un degrade du bas vers
    //le haut et de la droite vers la gauche
    if(millis() - t >= delai_couleur)
    {
      for(int i = 0; i < 3; i++)
      {
        leda1[i] = led1[i];
        ledv1[i] = random(2)*255;
        leda2[i] = led2[i];
        ledv2[i] = random(2)*255;
        leda3[i] = led3[i];
        ledv3[i] = random(2)*255;
        leda4[i] = led4[i];
        ledv4[i] = random(2)*255;
      }
      t = millis();
    }

    for(uint8_t i = 0; i < 3; i++)
    {
      led1[i] = map(millis() - t, 0, delai_couleur, leda1[i], ledv1[i]);
      led2[i] = map(millis() - t, 0, delai_couleur, leda2[i], ledv2[i]);
      led3[i] = map(millis() - t, 0, delai_couleur, leda3[i], ledv3[i]);
      led4[i] = map(millis() - t, 0, delai_couleur, leda4[i], ledv4[i]);
    }

    for(uint16_t i = 0; i < mh; i++)
    {
      for (uint16_t j = 0; j < mw; j++) {
        uint8_t led[3] = {0, 0, 0};
        for (uint8_t k = 0; k < 3; k++) {
          led[k] = map(i, 0, mh, led1[k], led2[k])/2
                   + map(j, 0, mw, led3[k], led4[k])/2;
          uint16_t nPixel;
          if (i % 2 == 0) {
            nPixel = i*mw + j;
          }
          else {
            nPixel = i*mw + mw - 1 - j;
          }
          matrix->setPixelColor(nPixel, led[0], led[1], led[2]);
        }
      }
    }
    matrix->show();
  }

  if (mode == 4) {//ce mode est une version du mode 2 qui avait un bug : dans
    //partie du code, je n'ai pas pense au fait que le tableau contenant la
    //couleur des led ne contenait que des nombres de 0 a 255 alors que je
    //l'utilisais avec des nombre de 0 a 510. Cela provoque un effet de couleur
    //differents mais originaux, je l'ai donc mis comme mode a part
    if(millis() - t >= delai_couleur)
    {
      for(int i = 0; i < 3; i++)
      {
        leda1[i] = led1[i];
        ledv1[i] = random(256);
        leda2[i] = led2[i];
        ledv2[i] = random(256);
        leda3[i] = led3[i];
        ledv3[i] = random(256);
        leda4[i] = led4[i];
        ledv4[i] = random(256);
      }
      t = millis();
    }

    for(uint8_t i = 0; i < 3; i++)
    {
      led1[i] = map(millis() - t, 0, delai_couleur, leda1[i], ledv1[i]);
      led2[i] = map(millis() - t, 0, delai_couleur, leda2[i], ledv2[i]);
      led3[i] = map(millis() - t, 0, delai_couleur, leda3[i], ledv3[i]);
      led4[i] = map(millis() - t, 0, delai_couleur, leda4[i], ledv4[i]);
    }

    for(uint16_t i = 0; i < mh; i++)
    {
      for (uint16_t j = 0; j < mw; j++) {
        uint8_t led[3] = {0, 0, 0};
        for (uint8_t k = 0; k < 3; k++) {
          led[k] = map(i, 0, mh, led1[k], led2[k])
                   + map(j, 0, mw, led3[k], led4[k]); //partie ou les /2 ont ete
          //retire, dans la version originale avec le bug, il y avait une ligne
          //avec led[k] = led[k]/2; mais afin d'exploiter l'effet au mieux elle
          //a ete retire
          uint16_t nPixel;
          if (i % 2 == 0) {
            nPixel = i*mw + j;
          }
          else {
            nPixel = i*mw + mw - 1 - j;
          }
          matrix->setPixelColor(nPixel, led[0], led[1], led[2]);
        }
      }
    }
    matrix->show();
  }

  if (mode == 5) {//ce mode affiche change de couleur un pixel aleatoirement a
    //une frequence proportionnelle a la position du slider
    if(millis() - t > (unsigned long)analogRead(A0))
    {
      matrix->setPixelColor(random(mw*mh), random(256), random(256), random(256));
      matrix->show();
      t = millis();
    }
  }

  if (mode == 6) {
    if(millis() - t < (unsigned long)analogRead(A0)/6)
    {
      matrix->fillScreen(LED_WHITE_HIGH); //on regle l'ecran en blanc
      matrix->show(); //on affiche ce qu'on a regle avant
    }
    else if(millis() - t < (unsigned long)analogRead(A0))
    {
      matrix->fillScreen(LED_BLACK); //on regle l'ecran en noir = eteint
      matrix->show(); //on affiche ce qu'on a regle avant
    }
    else
    {
      t = millis();
    }
  }
}
