/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Agente)
 *  Resumen:
 *  Lee los parámetros enviados, después valida que estén
 *  completos, registra el agente con el controlador y envía
 *  las solicitudes del archivo CSV usando tuberías
*************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "agente.h"

//funcion de ayuda para saber que escribir al momento de compilar, por si algo se escribe mal
void mostrarAyuda(const char *prog) {
    printf("Formato: %s -s nombre -a archivo.csv -p pipeCtrl\n", prog);
    printf("Ejemplo: %s -s A1 -a data/datos.csv -p /tmp/control\n", prog);
}

int main(int argc, char *argv[]) {

    char nom[80] = "";
    char csv[200] = "";
    char pipeC[150] = "";

    int c;
    while ((c = getopt(argc, argv, "s:a:p:h")) != -1) {
        if (c == 's') {
            strncpy(nom, optarg, sizeof(nom)-1);
        } 
        else if (c == 'a') {
            strncpy(csv, optarg, sizeof(csv)-1);
        } 
        else if (c == 'p') {
            strncpy(pipeC, optarg, sizeof(pipeC)-1);
        }
        else {
            mostrarAyuda(argv[0]);
            return 1;
        }
    }

    //Aqui se revisa que se haya enviado todo desde la consola y si no salta a ayuda para que se pida lo que debe enviar
    if (nom[0] == 0 || csv[0] == 0 || pipeC[0] == 0) {
        printf("[Agente] Parámetros incompletos\n");
        mostrarAyuda(argv[0]);
        return 1;
    }

    char miPipe[150];
    memset(miPipe, 0, sizeof(miPipe));

    // Aquí el agente intenta registrarse con el controlador.
    int horaSim = iniciarAgente(nom, pipeC, miPipe, sizeof(miPipe));
    if (horaSim < 0) {
        printf("[Agente %s] No se pudo hacer el registro inicial.\n", nom);
        return 1;
    }

    printf("[Agente %s] Empezando a enviar solicitudes desde '%s'\n",
           nom, csv);

    //Aqui el agente lee el archivo csv y se lo va enviando al controlador con tuberias
    enviarSolicitudes(csv, nom, pipeC, miPipe, horaSim);

    printf("[Agente %s] Terminé todo, cerrando.\n", nom);
    return 0;
}
