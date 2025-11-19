/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Agente)
 *  Resumen:
 *  Maneja toda la lógica del agente:
 *  - Crea su propio pipe
 *  - Se registra con el controlador
 *  - Lee el CSV y envía cada solicitud
 *  - Recibe las respuestas del controlador
*************************************************************/
#include "agente.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

/* Función para leer o escribir en un pipe según el modo */
static int usarPipe(const char *pipeNombre, Mensaje *m, int modo) {
    int fd;

    if (modo == 0) fd = open(pipeNombre, O_RDONLY);
    else           fd = open(pipeNombre, O_WRONLY);

    if (fd < 0) {
        perror("[Agente] problema con pipe");
        return -1;
    }

    ssize_t n;
    if (modo == 0) n = read(fd, m, sizeof(Mensaje));
    else           n = write(fd, m, sizeof(Mensaje));

    close(fd);

    if (n != sizeof(Mensaje)) {
        fprintf(stderr, "[Agente] transferencia incompleta en pipe\n");
        return -1;
    }

    return 0;
}

/* Registro inicial del agente con el controlador */
int iniciarAgente(const char *nombreAgente,
                  const char *pipeCtrl,
                  char *pipeLocal,
                  size_t maxLen)
{
    // Se arma el nombre del pipe personal del agente
    snprintf(pipeLocal, maxLen, "/tmp/pipe_%s", nombreAgente);

    if (mkfifo(pipeLocal, 0666) < 0) {
        if (errno != EEXIST) {
            perror("[Agente] no se pudo crear el pipe personal");
            return -1;
        }
    }

    Mensaje reg;
    memset(&reg, 0, sizeof(reg));

    strcpy(reg.tipo, "REGISTER");
    strncpy(reg.agente, nombreAgente, MAX_NOMBRE - 1);
    strncpy(reg.pipeRespuesta, pipeLocal, MAX_PIPE - 1);

    // Se envía al controlador el mensaje REGISTER
    if (usarPipe(pipeCtrl, &reg, 1) < 0) {
        printf("[Agente %s] el registro no salió\n", nombreAgente);
        return -1;
    }

    // Espera la hora simulada
    Mensaje r;
    if (usarPipe(pipeLocal, &r, 0) < 0) {
        printf("[Agente %s] no llegó la hora del controlador\n", nombreAgente);
        return -1;
    }

    if (strcmp(r.tipo, "TIME") != 0) {
        printf("[Agente %s] se esperaba TIME pero llegó %s\n",
               nombreAgente, r.tipo);
        return -1;
    }

    printf("[Agente %s] Registro listo. Hora sistema = %d\n",
           nombreAgente, r.hora);

    return r.hora;
}

/* Procesa todas las solicitudes cargando primero el CSV en memoria */
void enviarSolicitudes(const char *rutaCSV,
                       const char *nombreAgente,
                       const char *pipeCtrl,
                       const char *pipePersonal,
                       int horaBase)
{
    FILE *f = fopen(rutaCSV, "r");
    if (!f) {
        perror("[Agente] no pude abrir el CSV");
        return;
    }

    // Se guarda cada línea del CSV en memoria
    char **lineas = NULL;
    int total = 0;
    char buff[260];

    while (fgets(buff, sizeof(buff), f)) {
        lineas = realloc(lineas, sizeof(char*) * (total + 1));
        lineas[total] = strdup(buff);
        total++;
    }

    fclose(f);

    // Aquí se procesan las solicitudes ya cargadas
    for (int i = 0; i < total; i++) {

        Mensaje msg;
        memset(&msg, 0, sizeof(msg));

        char fam[70];
        int h = 0, c = 0;

        if (sscanf(lineas[i], "%69[^,],%d,%d", fam, &h, &c) != 3) {
            printf("[Agente %s] línea mal formada: %s",
                   nombreAgente, lineas[i]);
            continue;
        }

        strcpy(msg.tipo, "REQUEST");
        strncpy(msg.agente, nombreAgente, MAX_NOMBRE - 1);
        strncpy(msg.familia, fam, MAX_NOMBRE - 1);
        msg.hora = h;
        msg.cantidad = c;
        strncpy(msg.pipeRespuesta, pipePersonal, MAX_PIPE - 1);

        // Se ignoran solicitudes pasadas
        if (h < horaBase) {
            printf("[Agente %s] '%s' ignorado porque la hora %d ya pasó\n",
                   nombreAgente, fam, h);
            continue;
        }

        // Envío de solicitud
        printf("[Agente %s] --> Enviando solicitud: %s | hora %d | %d personas\n",
               nombreAgente, fam, h, c);

        if (usarPipe(pipeCtrl, &msg, 1) < 0) {
            printf("[Agente %s] error enviando solicitud\n", nombreAgente);
            continue;
        }

        // Se espera la respuesta del controlador
        Mensaje respuesta;
        if (usarPipe(pipePersonal, &respuesta, 0) < 0) {
            printf("[Agente %s] el controlador no respondió\n", nombreAgente);
            continue;
        }

        // Respuesta sin emojis
        if (!strcmp(respuesta.tipo, "OK")) {
            printf("[Agente %s] OK: '%s' quedó a las %d\n",
                   nombreAgente, respuesta.familia, respuesta.hora);

        } else if (!strcmp(respuesta.tipo, "REPROGRAMADO")) {
            printf("[Agente %s] Reprogramado '%s' para las %d\n",
                   nombreAgente, respuesta.familia, respuesta.hora);

        } else if (!strcmp(respuesta.tipo, "TARDE")) {
            printf("[Agente %s] '%s' estaba tarde, nueva hora %d\n",
                   nombreAgente, respuesta.familia, respuesta.hora);

        } else if (!strcmp(respuesta.tipo, "RECHAZADO")) {
            printf("[Agente %s] Rechazado '%s': %s\n",
                   nombreAgente, respuesta.familia, respuesta.motivo);

        } else {
            printf("[Agente %s] Respuesta desconocida: %s\n",
                   nombreAgente, respuesta.tipo);
        }

        // Pausa entre solicitudes
        sleep(2);
    }

    // Se libera memoria del CSV
    for (int j = 0; j < total; j++) free(lineas[j]);
    free(lineas);

    printf("[Agente %s] Terminé todas las solicitudes.\n", nombreAgente);
}
