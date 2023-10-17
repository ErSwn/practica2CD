#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define NUM_GENERALES 3
#define MAX_ROUNDS 10

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

int main() {
    srand(time(NULL));
    
    int rondas = 0;
    bool decision_generales[NUM_GENERALES] = {false}; // Inicialmente, todos los generales deciden retirarse
    
    while (rondas < MAX_ROUNDS) {
        rondas++;
        bool decision_actual = decision_generales[0]; // Inicializamos con la decisión del comandante

        for (int i = 1; i < NUM_GENERALES; i++) {
            decision_actual = comunicar_decision(decision_actual);
            decision_generales[i] = decision_actual;
        }

        // Comprobar si todos los generales han llegado a un consenso
        bool consenso = true;
        for (int i = 0; i < NUM_GENERALES; i++) {
            if (decision_generales[i] != decision_actual) {
                consenso = false;
                break;
            }
        }

        if (consenso) {
            printf("Los generales han llegado a un consenso en la ronda %d. La decisión es: %s\n", rondas, decision_actual ? "Atacar" : "Retirarse");
            break;
        }
    }

    if (rondas == MAX_ROUNDS) {
        printf("Se ha alcanzado el número máximo de rondas (%d) sin lograr un consenso.\n", MAX_ROUNDS);
    }

    return 0;
}
