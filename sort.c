#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SORT 1
#define END 2
#define TABSIZE 10000

void bubel_sort(int rank, int tmp, int batch_size, int sorted[]){
    int tmp_two;
    for(int i=0; i< batch_size; i++){
        if (sorted[i] == -1){
            sorted[i] = tmp;
            tmp = -1;
            break;
        } else {
            if (sorted[i] > tmp){
                tmp_two = sorted[i];
                sorted[i]=tmp;
                tmp = tmp_two;
            }	                
        }
    }
    if (tmp != -1){
        MPI_Send(&tmp, 1, MPI_INT, rank+1, SORT, MPI_COMM_WORLD);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int size,rank;
    int tablica[TABSIZE]={0};
    int sorted[TABSIZE]={-1};
    memset(sorted, -1, TABSIZE);
    
    int max=-1;
    int tmp=-1;
    int i;
    int end=0;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc<2) {
	    MPI_Finalize();
	    printf("Za malo argumentow\n");
	    exit(0);
    }
    /*
        Szkielet dzieli procesy na trzy kategorie: wierzchołek, liście oraz pozostałe.
        W Twoim rozwiązaniu być może wystarczą tylko dwie kategorie
    */

    if (rank == 0) {
        /* Wczytywanie liczb do posortowania z pliku podanego jako pierwszy argument */
        printf("Otwieram plik\n");
	    FILE *f;
	    int i;
	    f = fopen(argv[1], "r");
	    for (i=0;i<TABSIZE;i++) {
	        fscanf(f, "%d", &(tablica[i]));
	    }
	    for (i=0;i<TABSIZE;i++) {
	        //printf("%d ", (tablica[i]));
	    }

        max=tablica[0];
        printf("\n------------\n");
        /**/

        /* Tutaj wstaw wysyłanie liczb do bezpośrednich następników wierzchołka */
    	printf("%d: Wczytuje %d, wysylam\n", rank, max);
#define BATCH_SIZE 10    	
        for (i=1;i<TABSIZE;i++) {
            // MPI_SEND( &(tablica[i]).....
           // int address = (i/BATCH_SIZE) % (size-1) + 1;
            
//            printf("%d\n", address);
            MPI_Send(&tablica[i], 1, MPI_INT, 1, SORT, MPI_COMM_WORLD);
            //MPI_SEND( skąd, ile, typ, do kogo, z jakim tagiem, MPI_COMM_WORLD)
        }
        printf("%d\n", TABSIZE);
        /**/
        /* Zawiadamiamy, że więcej liczb do wysyłania nie będzie*/
        int dummy=-1;
	    MPI_Send( &dummy, 1, MPI_INT, 1, END, MPI_COMM_WORLD); //koniec liczb

        /* Tutaj wstaw odbieranie posortowanych liczb */
        for (i=1;i<size-1;i++) {
	//    MPI_Recv( gdzie, ile , jakiego typu, od kogo, z jakim tagiem, MPI_COMM_WORLD, &status);
		    MPI_Recv(&sorted[i-1], BATCH_SIZE , MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }

        /* Wyświetlanie posortowanej tablicy */
        for (i=0;i<TABSIZE-(size-1);i++) {
            printf("%d ",sorted[i]);
            if (sorted[i] == -1) break;
        }
        printf("\n");

    } else if (rank==size-1) { //LISC 
        int cnt = 0;
                printf("Rank: %d %d\n", rank, cnt);        
        while (!end) {      
	        MPI_Recv(&tmp, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	        cnt++;
	        if (status.MPI_TAG==END) {
		        end=1;
	             // cos jeszcze?
	        } else {
    	        //cos jeszcze?
	        } 
        }
        printf("Last Rank: %d %d\n", rank, cnt);        
    } else { //Ani wierzchołek, ani liść
        int cnt = 0;
      
        int tmp_two;
        while (!end) {
    	    MPI_Recv(&tmp, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	        if (status.MPI_TAG==END) {
	        	end=1;

	        	//coś jeszcze?
	        } else {
	            cnt +=1;
                bubel_sort(rank, tmp, BATCH_SIZE, sorted);	            
	        	//sortowanie babelkowe
	        }

        }
        int elements = 0;
        for(int i=0; i< BATCH_SIZE; i++){
            if (sorted[i] == -1){
                break;
            }
            elements ++;

            //printf("%d ", sorted[i]);
        }
        MPI_Send(sorted, elements, MPI_INT, 0, END, MPI_COMM_WORLD);
        printf("\nRank: %d %d\n", rank, cnt);
        MPI_Send(&tmp, 1, MPI_INT, rank+1, END, MPI_COMM_WORLD);
    }
   
    MPI_Finalize();
}
