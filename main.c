#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


/// Definim structura Punct ce contine coordonatele unui punct
typedef struct {
    double x, y;
} Punct;


/// Definim structura Muchie ce contine cele doua puncte ale unei muchii si costul acesteia
typedef struct {
    int u, v;
    double cost;
} Muchie;


/// Functia de calculare a distantei euclidiene dintre doua puncte
double distantaEuclidiana(Punct a, Punct b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}


/// Functia de comparare a muchiilor in functie de cost
int comparMuchiCrescator(const void *a, const void *b) {
    Muchie *m1 = (Muchie *)a;
    Muchie *m2 = (Muchie *)b;
    return (m1->cost > m2->cost) - (m1->cost < m2->cost);
}


/// Functia de comparare a muchiilor in functie de cost (descrescator)
int comparMuchiDescrescator(const void *a, const void *b) {
    return comparMuchiCrescator(b, a);
}


/// Functia de implementare a algoritmului lui Kruskal pentru gasirea arborelui parțial minim
Muchie* kruskal(Muchie *muchii, int numarMuchi, int n, int *numarMuchiiAPM) {
    // Alocăm spațiu pentru n-1 muchii în APM
    Muchie *apm = malloc((n-1) * sizeof(Muchie));
    *numarMuchiiAPM = 0;

    // Inițializare vector de componente
    int *componente = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) componente[i] = i;  // fiecare nod este considerat componenta proprie

    // Sortăm muchiile crescător după cost
    qsort(muchii, numarMuchi, sizeof(Muchie), comparMuchiCrescator);

    // Iterăm prin muchii adăugându-le dacă nu creează ciclu
    for (int i = 0; i < numarMuchi; i++) {
        int u = muchii[i].u;
        int v = muchii[i].v;

        // Dacă nodurile sunt în componente diferite, le unim
        if (componente[u] != componente[v]) {
            apm[(*numarMuchiiAPM)++] = muchii[i];  // adaugăm muchia în APM
            int compVechi = componente[v];
            // Reatribuim componenta lui v și nodurilor din vechea componentă la componenta lui u
            for (int j = 0; j < n; j++) {
                if (componente[j] == compVechi)
                    componente[j] = componente[u];
            }
        }
    }

    free(componente);  // eliberăm memoria pentru componente
    return apm;        // returnăm vectorul de muchii al APM
}


/// Functia de citire a datelor din fisier
void citesteDate(FILE *fin, int *n, int *k, Punct **puncte) {
    fscanf(fin, "%d %d", n, k);
    *puncte = malloc(*n * sizeof(Punct));
    for (int i = 0; i < *n; i++)
        fscanf(fin, "%lf %lf", &(*puncte)[i].x, &(*puncte)[i].y);
}


/// Functia de generare a muchiilor dintre puncte
Muchie* genereazaMuchi(Punct *puncte, int n, int *numarMuchi) {
    *numarMuchi = n*(n-1)/2;
    Muchie *muchii = malloc(*numarMuchi * sizeof(Muchie));
    int cnt = 0;
    for (int i = 0; i < n; i++)
        for (int j = i+1; j < n; j++) {
            muchii[cnt].u = i;
            muchii[cnt].v = j;
            muchii[cnt].cost = distantaEuclidiana(puncte[i], puncte[j]);
            cnt++;
        }
    return muchii;
}


// Formează clusterele tăind cele k-1 muchii de cost maxim din APM
void formeazaClustere(Muchie *apm, int numarMuchiiAPM, int k, int n, int *componente) {
    for (int i = 0; i < n; i++) componente[i] = i;  // resetăm componentele

    // Sortăm APM descrescător după cost pentru a elimina cele mai mari k-1 muchii
    qsort(apm, numarMuchiiAPM, sizeof(Muchie), comparMuchiDescrescator);

    // Pentru fiecare muchie de la index k-1 încolo, o adăugăm în cluster 
    for (int i = k-1; i < numarMuchiiAPM; i++) {
        int u = apm[i].u;
        int v = apm[i].v;

        if (componente[u] != componente[v]) {
            int compVechi = componente[v];
            for (int j = 0; j < n; j++) {
                if (componente[j] == compVechi)
                    componente[j] = componente[u];
            }
        }
    }
}


/// Identifică etichetele unice ale clusterelor și numărul acestora
void gasesteClustere(int *componente, int n, int **idCluster, int *numarClustere, int **radaciniUnice) {
    *idCluster = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) (*idCluster)[i] = componente[i];  // copiem componentele

    bool *vizitat = calloc(n, sizeof(bool));
    *numarClustere = 0;
    // Numărăm câte componente unice există
    for (int i = 0; i < n; i++) {
        if (!vizitat[componente[i]]) {
            vizitat[componente[i]] = true;
            (*numarClustere)++;
        }
    }

    *radaciniUnice = malloc(*numarClustere * sizeof(int));
    int cnt = 0;
    // Colectăm indicii rădăcinilor (componentelor) unice
    for (int i = 0; i < n; i++)
        if (vizitat[i]) (*radaciniUnice)[cnt++] = i;

    free(vizitat);  // eliberăm memoria pentru vizitat
}


/// Calculează centrele clusterelor și distanța maximă de la centru la punctele din cluster
void calculeazaCentre(Punct *puncte, int *idCluster, int *radaciniUnice, int numarClustere, int n, Punct **centre, double *deltaMaxim) {
    *centre = calloc(numarClustere, sizeof(Punct));  // inițializăm vectorul de centre cu 0
    int *contor = calloc(numarClustere, sizeof(int)); // contor pentru numărul de puncte în fiecare cluster

    // Sumăm coordonatele punctelor în funcție de cluster
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < numarClustere; j++) {
            if (idCluster[i] == radaciniUnice[j]) {
                (*centre)[j].x += puncte[i].x;
                (*centre)[j].y += puncte[i].y;
                contor[j]++;
                break;
            }
        }
    }

    // Calculăm media coordonatelor (centrul de greutate) și deltaMaxim
    *deltaMaxim = 0;
    for (int i = 0; i < numarClustere; i++) {
        (*centre)[i].x /= contor[i];  // coordonata x medie
        (*centre)[i].y /= contor[i];  // coordonata y medie

        double maxDist = 0;
        for (int j = 0; j < n; j++) {
            if (idCluster[j] == radaciniUnice[i]) {
                double dist = distantaEuclidiana((*centre)[i], puncte[j]);
                if (dist > maxDist) maxDist = dist;
            }
        }
        if (maxDist > *deltaMaxim) *deltaMaxim = maxDist;  // actualizăm distanța maximă globală
    }

    free(contor);  // eliberăm contorul
}

/// Functia de scriere a rezultatelor in fisier
void scrieRezultate(FILE *fout, Punct *centre, int numarClustere, double deltaMaxim) {
    for (int i = 0; i < numarClustere; i++)
        fprintf(fout, "%.6lf %.6lf\n", centre[i].x, centre[i].y);
    fprintf(fout, "%.6lf\n", deltaMaxim);
}

int main() {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    if (!fin || !fout) {
        printf("Eroare la deschidere fisier!\n");
        return 1;
    }

    int n, k;
    Punct *puncte;
    citesteDate(fin, &n, &k, &puncte);  // citim datele de intrare

    int numarMuchi;
    Muchie *muchii = genereazaMuchi(puncte, n, &numarMuchi);  // generăm muchiile grafului complet

    int numarMuchiiAPM;
    Muchie *apm = kruskal(muchii, numarMuchi, n, &numarMuchiiAPM);  // construim APM cu Kruskal

    int *componente = malloc(n * sizeof(int));
    formeazaClustere(apm, numarMuchiiAPM, k, n, componente);  // formăm k clustere din APM

    int *idCluster, numarClustere, *radaciniUnice;
    gasesteClustere(componente, n, &idCluster, &numarClustere, &radaciniUnice);  // identificăm clusterele

    Punct *centre;
    double deltaMaxim;
    calculeazaCentre(puncte, idCluster, radaciniUnice, numarClustere, n, &centre, &deltaMaxim); // calculăm centre

    scrieRezultate(fout, centre, numarClustere, deltaMaxim);  // scriem rezultatele

    // Eliberare memorie
    free(puncte);
    free(muchii);
    free(apm);
    free(componente);
    free(idCluster);
    free(radaciniUnice);
    free(centre);

    fclose(fin);
    fclose(fout);

    return 0;
}