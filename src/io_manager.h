#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include<stdio.h>

struct Graf;
int wczytajGraf(FILE *plikWejsciowy, struct Graf *graf);
void zapiszWyniki(FILE *plikWyjsciowy, struct Graf *graf);

#endif