#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ROUNDS 10

// Función que simula la decisión de un general (leal o traidor)
bool tomar_decision() {
    return rand() % 2; // 0 para retirarse, 1 para atacar
}

bool decision_aleatoria(){ // para los traidores
    return rand() % 2;
}

int main(int argc, char** argv) {


    int rank, size;
    int t = size/3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL));

    if (rank == 0)
    printf("\nKing consensus algorithm\n\n");

    bool traidor;
    

    // Seleccionamos a los traidores
    if (rank == 0){
        // elegimos los traidores aleatoriamente (1/3 de los generales)
        bool traidores[size];

        for (int i = 0; i < size; i++){
            traidores[i] = false;
        }

        int count = 0;
        while (count < size/3){
            int random = rand() % size;
            printf("random: %d\n", random);
            if (!traidores[random]){
                traidores[random] = true;
                count++;
                printf("El general %d es un traidor\n", random);
            }
        }

        // sent traitor role to the generals
        for (int i = 1; i < size; i++){
            MPI_Send(&traidores[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        traidor = traidores[0];

    } else {
        // reiceive traitor role
        MPI_Recv(&traidor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // printf("El general %d es %s\n", rank, traidor ? "un traidor" : "leal");

    // choose king
    int king;
    if (rank == 0){
        king = rand() % size;
        printf("El rey es el general %d\n", king);
        for (int i = 1; i < size; i++){
            MPI_Send(&king, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        int king;
        MPI_Recv(&king, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // initial desitions of the generals
    bool decision_generales[size];
    bool decision_actual; // Inicializamos con la decisión del comandante
    bool king_decision = false;

    srand(time(NULL) + rank);

    bool decision = tomar_decision();

    // loop until consensus
    for (int ronda = 1; ronda < MAX_ROUNDS; ronda++){
        if (rank == 0) printf("Ronda %d\n", ronda);

        if (king_decision) {
            // send king decision to generals if general then reiceive decision
            if (rank == 0){
                for (int j = 0; j < size; j++){
                    if (j == rank){
                        decision_generales[j] = decision;
                        continue;
                    }
                    MPI_Send(&decision, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                }
            } else {
                MPI_Recv(&decision, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

        } else {
            // receive decision from commanders except from itself
            for (int j = 0; j < size; j++) {
                if (j == rank){

                    decision_generales[j] = decision;
                    // send decision to generals
                    for (int k = 0; k < size; k++){
                        if (!(rank == k)){
                            int temp = (!traidor) ? decision: decision_aleatoria();
                            MPI_Send(&temp, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
                        }
                    }

                    continue;
                }
                
                MPI_Recv(&decision_generales[j], 1, MPI_INT, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // mayority decision
        int sum = 0;
        for (int j = 0; j < size; j++){
            if (decision_generales[j]){
                sum++;
            }
        }

        // print information about the decision
        // printf("Informacion del general %d: %s con %d votos\n", rank, 
        //             sum > size/2 + t? "Atacar" : "Retirarse", 
        //             sum > size/2 + t? sum: size - sum + 1);

        decision_actual = (sum >= size/2);
        
        // check consensus
        sum = 0;
        for (int j = 0; j < size; j++){
            if (decision_generales[j] == decision_actual){
                sum++;
            }
        }

        bool consensus = (sum > size/2);
        bool consensus_other_general;

        // verify consensus (send even if not consensus)
        for (int j = 0; j < size; j++){
            if (j == rank){
                for (int k = 0; k < size; k++){
                    if (k == rank){
                        continue;
                    }
                    MPI_Send(&consensus, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
                }
                continue;
            }
            MPI_Recv(&consensus_other_general, 1, MPI_INT, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (consensus_other_general != consensus){
                consensus = false;
            }
        }


        if (consensus){
            if (rank == 0)
            printf("Los generales han llegado a un consenso en la ronda %d. La decisión es: %s\n", ronda, decision_actual ? "Atacar" : "Retirarse");
            break;
        } else {
            if (rank == 0)
            printf("Los generales no han llegado a un consenso.\n");

            // change king if not consensus
            if (king_decision){
                if (rank == 0){
                    king = rand() % size;
                    printf("El rey es el general %d\n", king);
                    for (int t = 1; t < size; t++){
                        MPI_Send(&king, 1, MPI_INT, t, 0, MPI_COMM_WORLD);
                    }
                } else {
                    MPI_Recv(&king, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                king_decision = false;
            } else {
                king_decision = true;
            }
           
        }
    }

    printf("El general %d es %s\n", rank, traidor ? "un traidor" : "leal");

    // if (traidor){
    //     printf("El general %d era un traidor y habia elegido %s\n", rank, decision ? "Atacar" : "Retirarse");
    // } else {
    //     printf("El general %d era leal y habia elegido %s\n", rank, decision ? "Atacar" : "Retirarse");
    // }
    // printf("Habia elegido %s\n", decision ? "Atacar" : "Retirarse");



    if (rank == 0)
    printf("El general %d era el rey\n", king);

    MPI_Finalize();

}