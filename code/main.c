#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ROUNDS 10

// MPI_Bcast(&x,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

// Función que simula la decisión de un general (leal o traidor)
bool tomar_decision() {
    return rand() % 2; // 0 para retirarse, 1 para atacar
}    

// Función que simula la comunicación entre generales
bool comunicar_decision(bool decision) {
    if (rand() % 2 == 0) { // Simula la posibilidad de enviar información errónea
        return !decision; // Cambiar la decisión
    }
    return decision;
}

int main(int argc, char** argv) {
    srand(time(NULL));

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0){
        // main process
        int rondas = 0;
        bool decision_generales[size]; // Inicialmente, todos los generales deciden retirarse
        bool consenso = false;
        bool continuar = true;
        bool decision_actual = decision_generales[0]; // Inicializamos con la decisión del comandante

        for (int i = 0; i < size; i++){
            decision_generales[i] = false;
        }

        while (rondas < MAX_ROUNDS) {
            rondas++;
            printf("Ronda %d\n", rondas);
            // receive decision from commanders
            for (int i = 1; i < size; i++) {
                MPI_Recv(&decision_generales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Decision del general %d: %s\n", i, decision_generales[i] ? "Atacar" : "Retirarse");
            }

            for (int i = 1; i < size; i++){
                if (decision_generales[i] != decision_actual){
                    consenso = false;
                    break;
                }
            }
            printf("Consensus: %d\n", consenso);
            continuar = !consenso;
            for (int i = 1; i < size; i++) {
                MPI_Send(&continuar, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

            if (consenso) {
                break;
            }
            
            
        }
        if (consenso) {
            printf("Los generales han llegado a un consenso en la ronda %d. La decisión es: %s\n", rondas, decision_actual ? "Atacar" : "Retirarse");
        }   else {
            printf("Los generales no han llegado a un consenso.\n");
        }
        
    } else {

        srand(time(NULL) + rank);
        int rondas = 0;
        bool continuar = true;

        do {
            rondas++;
            bool decision_actual;
            decision_actual = comunicar_decision(decision_actual);
        
            // send decision to node 0
            MPI_Send(&decision_actual, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

            // wait for decision from node 0
            MPI_Recv(&continuar, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);            
        } while (MAX_ROUNDS > rondas && continuar);
        
    }
    MPI_Finalize();
    return 0;
}
