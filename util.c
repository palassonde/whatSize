#include "util.h"


/**
* À utiliser pour afficher la taille et le nom d'un élément
*/
void print_element(unsigned long long taille, char * nom)
{
  printf("%-30llu\t%s\n",taille,nom);
}


void print_erreur(int err)
{
  switch (err)
  {
    case ERR_DROITS_INSUFFISANTS :
      printf("Erreur : droit insuffisant\n");
      break;
    case ERR_FICHIER_REPERTOIRE_INEXISTANT :
      printf("Erreur : fichier ou répertoire inexistant\n");
      break;
    case ERR_AUTRE :
      printf("Erreur : autre\n");
      break;
    default:
      break;
  }
}
