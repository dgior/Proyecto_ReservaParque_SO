/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Main del Controlador)
 *  Resumen:
 *  Este archivo recibe los parámetros desde consola y
 *  valida que estén completos. Luego llama a la función
 *  que inicia toda la simulación del controlador.
 *************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "controlador.h"

//función de ayuda para mostrar cómo se debe ejecutar
static void ayuda(const char *p) {
    printf("Uso: %s -i inicio -f fin -s segs -t capacidad -p pipe\n", p);
    printf("Ejemplo: %s -i 7 -f 19 -s 5 -t 20 -p /tmp/pipeControl\n", p);
}

int main(int argc, char *argv[]) {

    int ini = -1;
    int fin = -1;
    int seg = -1;
    int cap = -1;
    char pipeC[150] = "";

    int c;

    //aquí se leen los valores enviados por consola con getopt
    while ((c = getopt(argc, argv, "i:f:s:t:p:h")) != -1) {
        if (c == 'i') ini = atoi(optarg);
        else if (c == 'f') fin = atoi(optarg);
        else if (c == 's') seg = atoi(optarg);
        else if (c == 't') cap = atoi(optarg);
        else if (c == 'p') strncpy(pipeC, optarg, sizeof(pipeC)-1);
        else {
            ayuda(argv[0]);
            return 1;
        }
    }

    //validaciones básicas de los parámetros
    if (ini < 7 || ini > 19) {
        printf("Hora inicial fuera del rango.\n");
        ayuda(argv[0]);
        return 1;
    }

    if (fin < 7 || fin > 19) {
        printf("Hora final fuera del rango.\n");
        ayuda(argv[0]);
        return 1;
    }

    if (fin < ini) {
        printf("La hora final no puede ser menor.\n");
        ayuda(argv[0]);
        return 1;
    }

    if (seg <= 0) {
        printf("Los segundos por hora deben ser positivos.\n");
        ayuda(argv[0]);
        return 1;
    }

    if (cap <= 0) {
        printf("La capacidad debe ser mayor a cero.\n");
        ayuda(argv[0]);
        return 1;
    }

    if (pipeC[0] == 0) {
        printf("Pipe no válido.\n");
        ayuda(argv[0]);
        return 1;
    }

    //si todo está bien, se inicia el controlador
    printf("[Controlador] Iniciando...\n");

    iniControlador(ini, fin, seg, cap, pipeC);

    printf("[Controlador] Terminado.\n");
    return 0;
}
