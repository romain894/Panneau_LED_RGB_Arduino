#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include "colors.h"
#include "bitmap.h"
#include <ArduinoSTL.h>

//Par Romain THOMAS le 24 aout 2019
//https://github.com/romain894

//Ce code a pour but de montrer les possibilites de programmation d'un ecran led
//8x8 RGB. Cet exemple montre comment on pourrait programmer l'ecran pour s'en
//servir comme une lumiere d'ambiance, un ecran pour afficher du texte et des
//images ou un puissance 4. Vous pouvez envoyer ce code sur la carte Arduino
//nano pour voir ce qu'il fait. N'hesitez pas a le modifier afin de l'adapter a
//vos envies !
//Le code est separe un trois parties, qui sont les trois programmes :
// - Exemple_lumiere_ambiance
// - Exemple_simple_ecran
// - Exemple_puissance_4
//Au lieu d'envoyer ce code sur la carte, vous pouvez juste envoyer un des codes
//precedent, vous aurez ainsi juste acces aux fonctionnalites souhaites

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

/**************************** Code general ***********************************/
uint8_t pinBoutonModeGeneral = 3;
uint8_t modeGeneral = 0;
void interrupMode();//Permet de changer le mode general (principal)

/*************************** Lumiere ambiance ********************************/
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

/***************************** Simple ecran **********************************/
//textes a afficher :
char texte1[] = "UTT 2019";//Ne pas mettre plus de 10 a 13 carracteres
uint8_t nb_char_texte1 = 8;

void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h);
void display_rgbBitmap(uint8_t bmp_num);

/***************************** Puissance 4 ***********************************/

//uint8_t pinBouton = 2; //pin pour le bouton

uint8_t grille[8][7] = {
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0}
}; //ce tableau contient la position de chaque pion : 0:aucun, 1:joueur1 et
//2:joueur2

volatile uint8_t etat_partie; //Etat de la partie; 1:a J1 de jouer ; 2:a J2 de
//jouer ; 3:fin de la partie

uint8_t joueur_gagnant; //1 ou 2 pour J1 ou J2

volatile uint8_t verrouiller_bouton = LOW; //verrouiller le bouton si egale a
//HIGH : les pressions sur le bouton ne sont plus prisent en compte

volatile uint8_t descendrePion = LOW; //quand a HIGH, on a demande a descendre
//le pion

uint8_t i_precedent_position = 0;
uint8_t x_precedent_position = 0;

//textes a afficher :
char puissance4[] = "Puissance 4";
uint8_t nb_char_puissance4 = 11;
char j1[] = "J1 gagne !";
uint8_t nb_char_j1 = 10;
char j2[] = "J2 gagne !";
uint8_t nb_char_j2 = 10;

void display_scrollText(char texte[], uint8_t nb_char, uint16_t couleur, uint16_t vitesse);
//uint16_t recupIDPixel(uint8_t x, uint8_t y); //recupere le numero du pixel en
//fonction de sa position en x et y (l'origine est en bas a gauche)
void interrupBouton();//initialiser une nouvelle partie si en attente d'une
void afficherPositionPionJoueur(uint8_t x, uint8_t joueur); //affiche le pion en
//haut de la grille pour que le joueur le positionne
void updateMatrix(); //met a jour la matrice LED
void descendrePionJoueur(); //fait descendre le pion du joueur dans la grille
void afficherFinPartie(uint8_t joueur_gagnant); //affiche le joueur que a gagne
//nouvelle partie
uint8_t verifierGagne(uint8_t joueur, uint8_t x_tmp, uint8_t y_tmp); //verifier
//si le joueur a gagne
uint8_t compteNombrePionDirection(int8_t x, int8_t y, int8_t dir_x, int8_t dir_y);
uint8_t compteNombrePion(int8_t x, int8_t y, int8_t dir_x, int8_t dir_y);
void nouvellePartie();

void setup() { //code utilise au demarrage
  /**************************** Code general ***********************************/
  pinMode(pinBoutonModeGeneral, INPUT_PULLUP);
  delay(50);
  attachInterrupt(digitalPinToInterrupt(pinBoutonModeGeneral), interrupMode, FALLING);

  /************************** Lumiere ambiance *******************************/
  Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  matrix->begin(); //initialisation de la matrice de led
  matrix->setTextWrap(false);
  matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255)); //on relge la
  //luminosite en fonction du potentiometre sur le circuit
  randomSeed(analogRead(A7)); //initialisation de la generation aleatoire
  pinMode(pinBouton, INPUT_PULLUP); //on se sert du pin du bouton pour les
  //modes comme une entree pour un interuppteur

  /**************************** Simple ecran *********************************/
  //Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  //matrix->begin(); //initialisation de la matrice de led
  //matrix->setTextWrap(false);
  //matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255)); //on relge la
  //luminosite en fonction du potentiometre sur le circuit
  matrix->fillScreen(LED_WHITE_HIGH); //on regle l'ecran en blanc
  matrix->show(); //on affiche ce qu'on a regle avant
  delay(1000); //on attend une seconde


  /**************************** Puissance 4 **********************************/

  //pinMode(pinBouton, INPUT_PULLUP); //on se sert du pin du bouton comme une
  //entree pour un interuppteur
  delay(50);
  //Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  attachInterrupt(digitalPinToInterrupt(pinBouton), interrupBouton, FALLING);
  //Quand le bouton sera appuye, cette fonction sera appele, cela simplifie la
  //gestion du bouton pour le debut de la partie : sinon il faudrait appuyer
  //sur le bouton exactement au bon moment
  //randomSeed(analogRead(A7)); //initialisation de la generation aleatoire
  //matrix->begin(); //initialisation de la matrice de led
  //matrix->setTextWrap(false);
  //matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255));
  nouvellePartie();
}

void loop() {
  switch (modeGeneral) {
  /************************** Lumiere ambiance *******************************/
  case 0:
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
  break;

  /**************************** Simple ecran *********************************/
  case 1:
  //Affciher les bitmap (aka pixel art) du permier au dernier (donc du 0 au 1)
  for (uint8_t i = 0; i < 2; i++) { //on va de 0 a 1 pour afficher les 2 images
    display_rgbBitmap(i); //on affiche l'image i
    delay(2000); //on attend 2 secondes
  }

  display_scrollText(texte1, nb_char_texte1, LED_BLUE_HIGH, 100); //on affiche
  // le texte contenu dans text1 en bleu avec un delai de 100ms
  break;


  /**************************** Puissance 4 **********************************/
  case 2:
  switch (etat_partie) {

    case 1:
    afficherPositionPionJoueur((uint8_t)map(analogRead(A0), 0, 1023, 0, 6), 1);
    updateMatrix();
    break;

    case 2:
    afficherPositionPionJoueur((uint8_t)map(analogRead(A0), 0, 1023, 0, 6), 2);
    updateMatrix();
    break;

    case 3:
    afficherFinPartie(joueur_gagnant);
    nouvellePartie();
    break;
  }

  if (descendrePion == HIGH) {
    descendrePionJoueur();
    descendrePion = LOW;
    verrouiller_bouton = LOW;
  }
  break;
  }
}

/**************************** Code general ***********************************/
void interrupMode() {
  modeGeneral = (modeGeneral + 1)%3;//on incremente le mode general pour en
  //en changer et si on depasse 2 on revient avec le % entre 0 et 2 compris
  nouvellePartie();
}

/***************************** Simple ecran **********************************/
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


/***************************** Puissance 4 ***********************************/

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

void interrupBouton() {
  if (verrouiller_bouton == LOW) {//cela evite de rentrer dans la fonction si
    //quelque chose est deja en cours
    if (etat_partie == 3) {
      nouvellePartie();
    }
    else {
      verrouiller_bouton = HIGH;
      descendrePion = HIGH;
    }
  }
}

void afficherPositionPionJoueur(uint8_t x, uint8_t joueur) {
  uint8_t i = 6;
  for (size_t i = 0; i < 7; i++) {
    grille[7][i] = 0;
  }
  grille[7][x] = joueur;

  grille[i_precedent_position][x_precedent_position] = 0;
  while ((grille[i-1][x] == 0) && (i > 0)) {
    i -= 1;
  }
  grille[i][x] = 3;
  i_precedent_position = i;
  x_precedent_position = x;
}

void updateMatrix() {
  for (size_t i = 0; i < 8; i++) {//hauteur
    for (size_t j = 0; j < 7; j++) {
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 0;
      if (grille[i][j] == 1) { //J1 = rouge
        r = 255;
        //Serial.print(i*mw+j);
        //Serial.print("\t");
      }
      else if (grille[i][j] == 2) { //J2 = jaune
        r = 255;
        g =255;
      }
      else if (grille[i][j] == 3) { //vise = bleu
        b = 255;
      }
      //permet de prendre en charge le cablage des bandes leds (en zigzag) :
      if (i%2 == 0) {
        matrix->setPixelColor(i*mw+j, r, g, b);
      }
      else
      {
        matrix->setPixelColor((i+1)*mw-j-1, r, g, b);
      }
    }
  }
  matrix->show();
}

void descendrePionJoueur() {
  uint8_t x_pos = (uint8_t)map(analogRead(A0), 0, 1023, 0, 6); //position en x
  if ((grille[5][x_pos] == 0) || (grille[5][x_pos] == 3)) {//si place on descend
    grille[i_precedent_position][x_precedent_position] = 0; //on efface la vise
    i_precedent_position = 6;
    x_precedent_position = 6;
    uint8_t i = 7; //on commence a la derniere ligne
    while ((grille[i-1][x_pos] == 0) && (i > 0 )) {//tant qu'il n'y a pas de
    //pion et qu'on est pas en bas, on descend
      grille[i][x_pos] = 0;//on efface le pion sur l'ancienne ligne
      i -= 1; //on decremente i de 1
      grille[i][x_pos] = etat_partie;//on affiche le pion sur la ligne actuelle
      //etat_partie permet de savoir quelle est la couleur du pion du joueur
      updateMatrix(); //on actualise la matrice LED
      delay(50); //On attend un peu afin de faire apparaitre l'animation
    }
    if (verifierGagne(etat_partie, x_pos, i) != 0) {//on verifie si le joueur a
    //gagne comme on a positionne un pion en x_pos, i
      joueur_gagnant = etat_partie;
      etat_partie = 3;
    }
    else if (etat_partie == 1) {
      etat_partie = 2;
    }
    else {
      etat_partie = 1;
    }
  }
  else {
    uint8_t tmp = 0; //si aucune case n'est vide, tmp sera egal a 0
    for (size_t i = 0; i < 7; i++) {
      if (grille[5][i] == 0) {
        tmp = 1;
      }
    }
    if (tmp == 0) {//si la grille est pleine, on recommence la partie
      nouvellePartie();
    }
  }
}

void afficherFinPartie(uint8_t joueur_gagnant) {
  Serial.print("joueur ");
  Serial.print(joueur_gagnant);
  Serial.println(" gagne !!!");
  if (joueur_gagnant == 1) {
    display_scrollText(j1, nb_char_j1, LED_RED_HIGH, 100);
  }
  else {
    display_scrollText(j2, nb_char_j2, LED_ORANGE_HIGH, 100);
  }
}

uint8_t verifierGagne(uint8_t joueur, uint8_t x_tmp, uint8_t y_tmp) {
  Serial.print("verif... ");
  uint8_t res = 0;//0:le joueur a pas gagne; 1:le joueur a gagne
  int8_t x = (int8_t)x_tmp;
  int8_t y = (int8_t)y_tmp;
  //On verifie pour toute les direction si le joueur a gagne
  int8_t tab_dir_x[4] = {-1, 0, 1, 1};
  int8_t tab_dir_y[4] = { 1, 1, 1, 0};
  for (size_t i = 0; i < 4; i++) {//pour chaque direction
    if (compteNombrePionDirection(x, y, tab_dir_x[i], tab_dir_y[i]) >= 4) {
      res = i+1;
    }
  }
  Serial.println(res);
  //*****************COULEUR !!!************
  return res;
}

uint8_t compteNombrePionDirection(int8_t x, int8_t y, int8_t dir_x, int8_t dir_y) {
  uint8_t res =  compteNombrePion(x, y, dir_x, dir_y) //on compte dans un sens
               + compteNombrePion(x, y, -dir_x, -dir_y) + 1; //et dans le sens
  //oppose puis on ajoute 1 pour compter le pion de depart
  return res;
}

uint8_t compteNombrePion(int8_t x, int8_t y, int8_t dir_x, int8_t dir_y) {
  uint8_t res = 0;
  x += dir_x; //on va au x suivant
  y += dir_y; //on va au y suivant
  while ((x >= 0) && (x < 7) && (y >= 0) && (y < 6)) {//tant que la case existe
    if (grille[y][x] != etat_partie) {//si la case n'est pas celle du joueur
      break; //on arrete de compter
    }
    res += 1;
    x += dir_x;
    y += dir_y;
  }
  return res;
}

void nouvellePartie() {
  etat_partie = random(1, 3); //tire au sors le joueur qui commence (1 ou 2)
  for (size_t i = 0; i < 8; i++) {//efface la grille
    for (size_t j = 0; j < 7; j++) {
      grille[i][j] = 0;
    }
  }
}
