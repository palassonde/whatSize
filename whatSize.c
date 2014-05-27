//NOM et Prénom : LASSONDE, Pierre-Alexandre
//Code permanent : LASP16128901

//NOM et Prénom : GENTILCORE, Mathieu
//Code permanent : GENM19109005

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"

#define NOM_MAX 10000
#define ELEMENTS_INDESIRABLES 2

/* 	
Structure pour chaque élément (répertoire, fichier,..) du répertoire passé en argument.
	size -> La taille de l'élément ainsi que tous les éléments des potentiels sous-répertoires
			Si il s'agit d'une répertoire, tout lien physique dans sa sous-hiérarchie n’est comptabilisé qu’une fois.
	nom  -> Le nom de l'élément
	flag -> Attribut pour marquer les éléments déjà selectionés
*/			
struct element {

    unsigned long long size;
    char *nom;
    bool flag;

};

// Déclaration des fonctions
unsigned long long getSize(char *file_path, ino_t * tabNoInode, int * indice_tab_inode);
struct element creer_element(long long taille,char *nom);
int nbre_elements(DIR *dirp);
struct element trier(struct element *elements, int nb_elements);
bool lien_physique_existe(ino_t * tabNoInode, int * indice_tab_inode, ino_t noInode);
int get_nb_inodes(char *file_path);


int main(int argc, char *argv[]){

	DIR *dirp;
	struct dirent *direntp;
    struct stat file_stat;
    char file_path[NOM_MAX+1];

    struct element *elements;

    if (argc != 2) {
        print_erreur(ERR_AUTRE);
        return 1;
    }

    if ((dirp = opendir(argv[1])) == NULL) {
        print_erreur(ERR_FICHIER_REPERTOIRE_INEXISTANT);
        return 1;
    }
    int nb_elements = nbre_elements(dirp);
    while ((closedir(dirp) == -1) && (errno == EINTR));

    elements = malloc(nb_elements * sizeof(struct element));
    ino_t *tabNoInode;
    int *indice_tab_inode;
    int nbinodes;
    int i = 0;

    dirp = opendir(argv[1]);

    while ((direntp = readdir(dirp)) != NULL)
    {

        sprintf( file_path, "%s/%s", argv[1], direntp->d_name );
        
        if ((strcmp(direntp->d_name, ".") == 0) || (strcmp(direntp->d_name, "..") == 0))
			continue;

		if ( lstat( file_path, &file_stat ) != 0 ) {
	        print_erreur(ERR_DROITS_INSUFFISANTS);
	        exit(1);
	    }
	    
	    if(S_ISDIR(file_stat.st_mode)){

	    	nbinodes = get_nb_inodes(file_path);
	    	tabNoInode = calloc(nbinodes, sizeof(ino_t));
	    	indice_tab_inode = calloc(1, sizeof(int));

	    	elements[i] = creer_element((getSize(file_path, tabNoInode, indice_tab_inode) + file_stat.st_size), direntp->d_name);

	    	free(tabNoInode);
	    	free(indice_tab_inode);
	    }
	    else{

	    	elements[i] = creer_element(file_stat.st_size, direntp->d_name);
	    }
	    i++;
    }
    // Boucle d'affichage en ordre décroissant de taille et alphabétique
    for (int i = 0; i < nb_elements; i++){

    	struct element e;
    	
    	e = trier(elements,nb_elements);
        print_element(e.size, e.nom);
    }
    // Fermeture des dossier et libération la mémoire du tableau des éléments
	while ((closedir(dirp) == -1) && (errno == EINTR));

	free(elements);

	return 0;
}

/* 
Cette fonction récursive prends en paramêtre le file path d'un répertoire.
Elle retourne le nombre de inodes dans ça sous-hiérarchie.
(Ne prend pas en compte les liens physiques dans le calcul.)
*/

int get_nb_inodes(char *file_path){

	DIR *dirp;
	struct dirent *direntp;
    struct stat file_stat;
    char file_path_enfant[NOM_MAX+1];
    int nb_inodes = 0;

    dirp = opendir(file_path);

	while ((direntp = readdir(dirp)) != NULL)
    {
        sprintf( file_path_enfant, "%s/%s", file_path, direntp->d_name );
		
		if ((strcmp(direntp->d_name, ".") == 0) || (strcmp(direntp->d_name, "..") == 0))
			continue;
			
		if ( lstat( file_path_enfant, &file_stat ) != 0 ) {
	            print_erreur(ERR_DROITS_INSUFFISANTS);
	            exit(1);
	    }
 		else{

 			if (S_ISDIR(file_stat.st_mode)){

 			 	nb_inodes += (get_nb_inodes(file_path_enfant) + 1);
 			}
 			else {

 			 	nb_inodes ++;
 			}
 		}
 	}
 	
 	while ((closedir(dirp) == -1) && (errno == EINTR));

	return nb_inodes;
}

/* 
Cette fonction récursive prends en paramêtre le file path d'un répertoire, 
le tableau des inodes déjà comptabilisés, ainsi que l'indice du prochain no d'inode à ajouter.
Elle retourne la taille de tous les éléments de ça sous-hiérarchie.
Tout lien physique n'est comptabilisé qu'une seule fois dans le calcul.
*/

unsigned long long getSize(char *file_path, ino_t * tabNoInode, int * indice_tab_inode){

	unsigned long long size = 0.0;
	struct dirent *direntp;
    DIR *dirp;
    char file_path_enfant[NOM_MAX+1];
    struct stat file_stat;


    dirp = opendir(file_path);

	while ((direntp = readdir(dirp)) != NULL)
    {
        sprintf( file_path_enfant, "%s/%s", file_path, direntp->d_name );
        
        if ((strcmp(direntp->d_name, ".") == 0) || (strcmp(direntp->d_name, "..") == 0))
			continue;

		if ( lstat( file_path_enfant, &file_stat ) != 0 ) {
	            print_erreur(ERR_DROITS_INSUFFISANTS);
	            exit(1);
	    }
 		else {
 			if (S_ISDIR(file_stat.st_mode)){

 				if (!lien_physique_existe(tabNoInode, indice_tab_inode, file_stat.st_ino))
 					size += (getSize(file_path_enfant, tabNoInode, indice_tab_inode) + file_stat.st_size);
 			}
 			else {

 				if (!lien_physique_existe(tabNoInode, indice_tab_inode, file_stat.st_ino))
					size += file_stat.st_size;	
 			}
 			tabNoInode[*indice_tab_inode] = file_stat.st_ino;
			(*indice_tab_inode)++;
 		}
 	}
 	
 	while ((closedir(dirp) == -1) && (errno == EINTR));

	return size;
}

/* 
Cette fonction prend en paramêtre le tableau des numéros d'inodes déja observés, 
la taille du tableau, ainsi que le numéro d'inode à vérifier.
Retourne vrai si le numéro d'inode à déjà été observé et qu'il y a donc un lien physique.
*/

bool lien_physique_existe(ino_t * tabNoInode, int * taille_tab, ino_t noInode){

	bool existe = false;

	for (int i = 0; i < *taille_tab; ++i)
	{
		if (tabNoInode[i] == noInode)
			existe = true;
	}
	return existe;
}

/* 
Cette fonction prend en paramêtre les attributs d'un élément à créer.
Retourne l'element créé avec les attributs en arguments.
*/

struct element creer_element(unsigned long long size,char *nom){

	struct element e = {size,nom,false};

	return e;
}

/* 
Cette fonction calcul le nombre d'élément (fichier, répertoire,..) d'un répertoire
dont le pointeur est passé en paramêtre.
Elle retourne le nombre d'éléments en ignorant les sous répertoires.
*/

int nbre_elements(DIR *dirp){

	struct dirent *direntp;
	int nbre_elements = 0 - ELEMENTS_INDESIRABLES;

	while ((direntp = readdir(dirp)) != NULL)
    {
        nbre_elements++;
    }

    return nbre_elements;
}

/* 
Cette fonction prend en paramêtre le tableau des éléments à afficher et sa taille.
Elle trouve l'élément qui a la plus grande taille, et le premier dans l'ordre alphabétique
en cas d'égalité de taille. Elle marque le flag de l'élément à true pour qu'il ne puisse
pas être resélectionné lors des appels suivants.
Elle retourne l'élément qui vient en premier pour l'afficher.
*/

struct element trier(struct element *elements, int nb_elements){

	struct element e;
	unsigned long long size = 0;
	int index = 0;
	bool end = false;

	for (int i = 0; i < nb_elements ; ++i)
	{
		if (elements[i].size > size && elements[i].flag == false)
		{
			size = elements[i].size;
			e = elements[i];
			index = i;
		}
		else if (elements[i].size == size && elements[i].flag == false)
		{
			int taille_min = strlen(elements[i].nom) > strlen(e.nom) ? strlen(e.nom) : strlen(elements[i].nom);

			for (int j = 0; j < taille_min && end == false; ++j)
			{
				if (elements[i].nom[j] < e.nom[j])
				{	
					e = elements[i];
					end = true;
					index = i;
				}
				else if (elements[i].nom[j] > e.nom[j]){

					end = true;
				}
			}
			end = false;
		}
	}

	elements[index].flag = true;

	return e;
}
