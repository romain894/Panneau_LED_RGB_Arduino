#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include "colors.h"
#include <ArduinoSTL.h>

//Par Romain THOMAS le 24 aout 2019
//https://github.com/romain894

//Ce code a pour but de montrer les possibilites de programmation d'un ecran led
//8x8 RGB. Cet exemple montre comment on pourrait programmer l'ecran pour s'en
//servir comme un puissance 4. Vous pouvez envoyer ce code sur la carte
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

uint8_t pinBouton = 2; //pin pour le bouton

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
  pinMode(pinBouton, INPUT_PULLUP); //on se sert du pin du bouton comme une
  //entree pour un interuppteur
  delay(50);
  Serial.begin(57600); //initialisation du moniteur serie pour pouvoir ecrire
  //des messages sur l'ordinateur si besoin
  attachInterrupt(digitalPinToInterrupt(pinBouton), interrupBouton, FALLING);
  //Quand le bouton sera appuye, cette fonction sera appele, cela simplifie la
  //gestion du bouton pour le debut de la partie : sinon il faudrait appuyer
  //sur le bouton exactement au bon moment
  randomSeed(analogRead(A7)); //initialisation de la generation aleatoire
  matrix->begin(); //initialisation de la matrice de led
  matrix->setTextWrap(false);
  matrix->setBrightness(map(analogRead(A0), 0, 1023, 0, 255));
  nouvellePartie();
}

void loop() {
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
