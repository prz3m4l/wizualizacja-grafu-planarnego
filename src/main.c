#include<stdio.h>

#define bufor 4096

int main(int argc, char *argv[]){
    if(argc<2){
        fprintf(stderr, "Nie udalo sie wczytac pliku!");
        return -1;
    }
    FILE *plik = fopen(argv[1], "r");
    fclose(plik);
    return 0;
}