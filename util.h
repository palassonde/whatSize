#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>


#define ERR_DROITS_INSUFFISANTS 0
#define ERR_FICHIER_REPERTOIRE_INEXISTANT 1
#define ERR_AUTRE 2


/**
* À utiliser pour afficher les erreurs
*/
void print_erreur(int err);

/**
* À utiliser pour afficher la taille et le nom d'un élément
*/
void print_element(unsigned long long taille, char * nom);

#endif
