#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double x, y;
} Punct;

typedef struct {
    int u, v;
    double cost;
} Muchie;

/*----------------------------------------------------------
Structura Union-Find pentru gestionarea componentelor conexe
Funcționalitate:
- cautaRadacina: Găsește rădăcina unui nod cu compresie de cale
- unesteArbori: Unește doi arbori după rang (union by rank)
-----------------------------------------------------------*/
int cautaRadacina(int *parinte, int x) {
    // Compresie recursivă a căii până la rădăcină
    if (parinte[x] != x)
        parinte[x] = cautaRadacina(parinte, parinte[x]);
    return parinte[x];
}

void unesteArbori(int *parinte, int *rang, int x, int y) {
    int radacinaX = cautaRadacina(parinte, x);
    int radacinaY = cautaRadacina(parinte, y);

    // Union by rank pentru a menține arborele balansat
    if (radacinaX != radacinaY) {
        if (rang[radacinaX] > rang[radacinaY]) {
            parinte[radacinaY] = radacinaX;
        } else {
            parinte[radacinaX] = radacinaY;
            if (rang[radacinaX] == rang[radacinaY])
                rang[radacinaY]++;
        }
    }
}

/*----------------------------------------------------------
Funcții de comparare pentru sortare:
- comparMuchiCrescator: sortează muchiile crescător după cost
- comparMuchiDescrescator: sortează muchiile descrescător
-----------------------------------------------------------*/
int comparMuchiCrescator(const void *a, const void *b) {
    Muchie *m1 = (Muchie *)a;
    Muchie *m2 = (Muchie *)b;
    return (m1->cost > m2->cost) - (m1->cost < m2->cost);
}

int comparMuchiDescrescator(const void *a, const void *b) {
    return comparMuchiCrescator(b, a);
}

/*----------------------------------------------------------
Calculează distanța euclidiană între două puncte
-----------------------------------------------------------*/
double distantaEuclidiana(Punct a, Punct b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

/*----------------------------------------------------------
Funcții de gestionare a datelor și algoritmi principali
-----------------------------------------------------------*/

// Citirea datelor de intrare din fișier
void citesteDate(FILE *fin, int *n, int *k, Punct **puncte) {
    fscanf(fin, "%d %d", n, k);
    *puncte = (Punct *)malloc(*n * sizeof(Punct));
    for (int i = 0; i < *n; i++)
        fscanf(fin, "%lf %lf", &(*puncte)[i].x, &(*puncte)[i].y);
}

// Generează toate muchiile posibile între puncte
Muchie* genereazaMuchi(Punct *puncte, int n, int *numarMuchi) {
    *numarMuchi = n*(n-1)/2; // Combinări de n luate câte 2
    Muchie *muchii = (Muchie *)malloc(*numarMuchi * sizeof(Muchie));
    int cnt = 0;

    // Generează toate perechile unice de puncte
    for (int i = 0; i < n; i++)
        for (int j = i+1; j < n; j++) {
            muchii[cnt].u = i;
            muchii[cnt].v = j;
            muchii[cnt].cost = distantaEuclidiana(puncte[i], puncte[j]);
            cnt++;
        }
    return muchii;
}

/*----------------------------------------------------------
Algoritmul lui Kruskal pentru construirea APM:
1. Sortează muchiile crescător după cost
2. Selectează muchiile care conectează componente noi
3. Folosește Union-Find pentru a gestiona componentele conexe
-----------------------------------------------------------*/
Muchie* kruskal(Muchie *muchii, int numarMuchi, int n, int *numarMuchiiAPM) {
    Muchie *apm = (Muchie *)malloc((n-1) * sizeof(Muchie));
    int *parinte = (int *)malloc(n * sizeof(int));
    int *rang = (int *)calloc(n, sizeof(int));

    // Inițializare Union-Find
    for (int i = 0; i < n; i++)
        parinte[i] = i;

    qsort(muchii, numarMuchi, sizeof(Muchie), comparMuchiCrescator);

    int cnt = 0;
    for (int i = 0; i < numarMuchi && cnt < n-1; i++) {
        int u = muchii[i].u;
        int v = muchii[i].v;
        // Verifică dacă muchia conectează componente diferite
        if (cautaRadacina(parinte, u) != cautaRadacina(parinte, v)) {
            apm[cnt++] = muchii[i];
            unesteArbori(parinte, rang, u, v);
        }
    }

    free(parinte);
    free(rang);
    *numarMuchiiAPM = cnt;
    return apm;
}

/*----------------------------------------------------------
Formarea clusterelor prin eliminarea k-1 muchii din APM:
1. Sortează muchiile APM descrescător
2. Elimină primele k-1 muchii pentru a crea k componente conexe
-----------------------------------------------------------*/
void formeazaClustere(Muchie *apm, int numarMuchiiAPM, int k, int n, int **parinteCluster, int **rangCluster) {
    *parinteCluster = (int *)malloc(n * sizeof(int));
    *rangCluster = (int *)calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        (*parinteCluster)[i] = i;
        (*rangCluster)[i] = 0;
    }

    qsort(apm, numarMuchiiAPM, sizeof(Muchie), comparMuchiDescrescator);

    // Unim componentele, sărind primele k-1 muchii
    for (int i = k-1; i < numarMuchiiAPM; i++) {
        Muchie m = apm[i];
        unesteArbori(*parinteCluster, *rangCluster, m.u, m.v);
    }
}

/*----------------------------------------------------------
Identificarea clusterelor unice după operațiile de union:
1. Parcurge toate nodurile și colectează rădăcinile unice
2. Determină numărul total de clustere formate
-----------------------------------------------------------*/
void gasesteClustere(int *parinteCluster, int n, int **idCluster, int *numarClustere, int **radaciniUnice) {
    *idCluster = (int *)malloc(n * sizeof(int));
    // Determină rădăcina fiecărui nod
    for (int i = 0; i < n; i++)
        (*idCluster)[i] = cautaRadacina(parinteCluster, i);

    // Colectează rădăcini unice
    int *vizitat = (int *)calloc(n, sizeof(int));
    *numarClustere = 0;
    for (int i = 0; i < n; i++)
        if (!vizitat[(*idCluster)[i]]) {
            vizitat[(*idCluster)[i]] = 1;
            (*numarClustere)++;
        }

    // Salvează rădăcinile unice pentru clustere
    *radaciniUnice = (int *)malloc(*numarClustere * sizeof(int));
    int index = 0;
    for (int i = 0; i < n; i++)
        if (vizitat[i])
            (*radaciniUnice)[index++] = i;

    free(vizitat);
}

/*----------------------------------------------------------
Calculul centrelor și al deltei maxime:
1. Pentru fiecare cluster:
   a. Găsește punctul cu cea mai mică distanță maximă către celelalte
   b. Acesta devine centrul clusterului
2. Delta maximă este cea mai mare dintre distanțele maximale
-----------------------------------------------------------*/
void calculeazaCentreSiDelta(Punct *puncte, int *idCluster, int *radaciniUnice, int numarClustere, int n, Punct **centre, double *deltaMaxim) {
    int **puncteCluster = (int **)malloc(numarClustere * sizeof(int *));
    int *dimensiuni = (int *)calloc(numarClustere, sizeof(int));

    // Numără punctele din fiecare cluster
    for (int i = 0; i < n; i++) {
        int cluster = idCluster[i];
        for (int j = 0; j < numarClustere; j++) {
            if (radaciniUnice[j] == cluster) {
                dimensiuni[j]++;
                break;
            }
        }
    }

    // Alocă memorie pentru punctele din fiecare cluster
    for (int i = 0; i < numarClustere; i++)
        puncteCluster[i] = (int *)malloc(dimensiuni[i] * sizeof(int));
    int *indexCurent = (int *)calloc(numarClustere, sizeof(int));

    // Populează clusterele cu indicii punctelor
    for (int i = 0; i < n; i++) {
        int cluster = idCluster[i];
        for (int j = 0; j < numarClustere; j++) {
            if (radaciniUnice[j] == cluster) {
                puncteCluster[j][indexCurent[j]++] = i;
                break;
            }
        }
    }

    // Calcul centru și delta pentru fiecare cluster
    *centre = (Punct *)malloc(numarClustere * sizeof(Punct));
    double *deltaCluster = (double *)malloc(numarClustere * sizeof(double));
    *deltaMaxim = 0;

    for (int i = 0; i < numarClustere; i++) {
        double distantaMinimaMaxima = INFINITY;
        int indexCentru = -1;

        // Caută punctul cu cea mai mică distanță maximă în cluster
        for (int j = 0; j < dimensiuni[i]; j++) {
            int punctCurent = puncteCluster[i][j];
            double distantaMaxima = 0;

            // Calculează distanța maximă față de alte puncte din cluster
            for (int k = 0; k < dimensiuni[i]; k++) {
                if (j == k) continue;
                double d = distantaEuclidiana(puncte[punctCurent], puncte[puncteCluster[i][k]]);
                if (d > distantaMaxima)
                    distantaMaxima = d;
            }

            // Actualizează centrul dacă s-a găsit o distanță mai mică
            if (distantaMaxima < distantaMinimaMaxima) {
                distantaMinimaMaxima = distantaMaxima;
                indexCentru = punctCurent;
            }
        }

        (*centre)[i] = puncte[indexCentru];
        deltaCluster[i] = distantaMinimaMaxima;
        if (distantaMinimaMaxima > *deltaMaxim)
            *deltaMaxim = distantaMinimaMaxima;
    }

    // Eliberare resurse temporare
    for (int i = 0; i < numarClustere; i++)
        free(puncteCluster[i]);
    free(puncteCluster);
    free(dimensiuni);
    free(indexCurent);
    free(deltaCluster);
}

// Scrie rezultatele în fișierul de ieșire
void scrieRezultate(FILE *fout, Punct *centre, int numarClustere, double deltaMaxim) {
    for (int i = 0; i < numarClustere; i++)
        fprintf(fout, "%.6lf %.6lf\n", centre[i].x, centre[i].y);
    fprintf(fout, "%.6lf\n", deltaMaxim);
}

/*----------------------------------------------------------
Fluxul principal al programului:
1. Citește datele de intrare
2. Generează muchii
3. Construiește APM
4. Formează clustere
5. Calculează centre și delta
6. Scrie rezultate
-----------------------------------------------------------*/
int main() {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    if (!fin || !fout) {
        printf("Eroare la deschidere fisier!\n");
        return 1;
    }

    int n, k;
    Punct *puncte;
    citesteDate(fin, &n, &k, &puncte);

    int numarMuchi;
    Muchie *muchii = genereazaMuchi(puncte, n, &numarMuchi);

    int numarMuchiiAPM;
    Muchie *apm = kruskal(muchii, numarMuchi, n, &numarMuchiiAPM);

    int *parinteCluster, *rangCluster;
    formeazaClustere(apm, numarMuchiiAPM, k, n, &parinteCluster, &rangCluster);

    int *idCluster, numarClustere, *radaciniUnice;
    gasesteClustere(parinteCluster, n, &idCluster, &numarClustere, &radaciniUnice);

    Punct *centre;
    double deltaMaxim;
    calculeazaCentreSiDelta(puncte, idCluster, radaciniUnice, numarClustere, n, &centre, &deltaMaxim);

    scrieRezultate(fout, centre, numarClustere, deltaMaxim);

    // Eliberare memorie
    free(puncte);
    free(muchii);
    free(apm);
    free(parinteCluster);
    free(rangCluster);
    free(idCluster);
    free(radaciniUnice);
    free(centre);

    fclose(fin);
    fclose(fout);

    return 0;
}