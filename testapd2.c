#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int no_even(int number) {
    int digits = 0;

    while(number != 0) {
        if ((number % 10) % 2 == 0) {
            digits++;
        }
        number /= 10;
    }
    return digits;
}

int main (int argc, char *argv[])
{
	int  numtasks, rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);


	int recv_num;
	int task_num;
	MPI_Status status;
    
    //task-ul 0 citeste argumentele si le trimite mai departe
    //pastreaza numarul mai mare pentru a face sortarea
    if (rank == 0) {
        int cur_arg = atoi(argv[1]);

        for(int i = 1; i < numtasks; i++) {
            //se citeste urmatorul argument
            int next_arg  = atoi(argv[i + 1]);

            //pastrez numarul mai mare(dupa logica din enunt)
            if (no_even(cur_arg) > no_even(next_arg) || (no_even(cur_arg) == no_even(next_arg) && cur_arg > next_arg)) {
                MPI_Send(&next_arg, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            } else {
                MPI_Send(&cur_arg, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                cur_arg = next_arg;
            }
        }

        task_num = cur_arg;
    } else if (rank == numtasks - 1) {
        //ultimul task asteapta sa primeasca ultimul numar, care va fi cel mai mic
        MPI_Recv(&task_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
    } else {
        //restul task-urilor primesc numarul de la cel precedent
        //il compara si il pastreaza pe cel mai mare
        MPI_Recv(&task_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        for (int i = rank + 1; i < numtasks; i++) {
            MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);

            //pastrez numarul mai mare
            if (no_even(task_num) > no_even(recv_num) || (no_even(task_num) == no_even(recv_num) && task_num > recv_num)) {
                MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            } else {
                MPI_Send(&task_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                task_num = recv_num;
            }
        }
    }

    //se asteapta pentru sincronizare
	MPI_Barrier(MPI_COMM_WORLD);

    //fiecare task asteapta sa primeasca numere din stanga
    //apoi le trimite mai departe la urmatorul task
    
	for (int i = 0; i < rank; i++) {
		MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &status);

        //ultimul task va afisa numere
        //le primeste in ordine descrescatoare
		if (rank != numtasks - 1) {
			MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
		} else {
			printf("%d ", recv_num);
		}
	}

    //la final task-ul isi trimite si numarul sau il afiseasa daca este tasn-ul N - 1
	if (rank != numtasks - 1) {
		MPI_Send(&task_num, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
	} else {
		printf("%d\n", task_num);
	}

	MPI_Finalize();
}
