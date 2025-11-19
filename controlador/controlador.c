/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Controlador)
 *  Resumen:
 *  En este archivo está toda la lógica del controlador.
 *  Aquí se maneja el reloj, la ocupación del parque y las
 *  respuestas que se envían a los agentes.
 *************************************************************/

#include "controlador.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

EstadoParque parque;
char pipeGeneral[120];

static int aceptadas = 0;
static int reprogramadas = 0;
static int tardes = 0;
static int rechazadas = 0;

//mutex para proteger la ocupación cuando los hilos trabajan al tiempo
pthread_mutex_t lockParque = PTHREAD_MUTEX_INITIALIZER;

//revisa si hay espacio en dos horas seguidas para meter un grupo
static int revisarHueco(int h, int cant) {
    if (h < parque.horaActual) return 0;
    if (h + 1 > parque.horaFin) return 0;
    if (parque.ocupacion[h] + cant > parque.capacidadMaxima) return 0;
    if (parque.ocupacion[h+1] + cant > parque.capacidadMaxima) return 0;
    return 1;
}

//aumenta la ocupación en las dos horas
static void sumarGrupo(int h, int cant) {
    parque.ocupacion[h] += cant;
    parque.ocupacion[h+1] += cant;
}

//envía la respuesta al agente según lo que pasó con su solicitud
static void mandarRta(Mensaje *m, const char *tipo, int hora, const char *mot) {
    Mensaje x;
    memset(&x, 0, sizeof(x));

    strcpy(x.tipo, tipo);
    strcpy(x.agente, m->agente);
    strcpy(x.familia, m->familia);
    x.hora = hora;
    x.cantidad = m->cantidad;
    strcpy(x.pipeRespuesta, m->pipeRespuesta);

    if (mot != NULL) {
        strncpy(x.motivo, mot, MAX_MOTIVO - 1);
    }

    int f = open(m->pipeRespuesta, O_WRONLY);
    if (f < 0) return;
    write(f, &x, sizeof(x));
    close(f);
}

//aquí se procesa cada solicitud que manda el agente
void gestioDeSolicitud(Mensaje *m) {

    printf("[Controlador] Llegó solicitud de %s: %s pide %d para %d personas\n",
           m->agente, m->familia, m->hora, m->cantidad);

    //si el grupo es demasiado grande, se rechaza
    if (m->cantidad > parque.capacidadMaxima) {
        rechazadas++;
        mandarRta(m, "RECHAZADO", -1, "Grupo supera el aforo maximo");
        printf("   -> No entra: sobrepasa la capacidad.\n");
        return;
    }

    //si piden una hora fuera del rango del parque
    if (m->hora > parque.horaFin) {
        rechazadas++;
        mandarRta(m, "RECHAZADO", -1, "Hora solicitada fuera del rango del dia");
        printf("   -> No entra: hora fuera del horario del parque.\n");
        return;
    }

    pthread_mutex_lock(&lockParque);

    int hs = m->hora;
    int actual = parque.horaActual;

    //si pidieron en una hora que ya pasó
    if (hs < actual) {
        int nueva = -1;

        //aquí se busca otra hora libre
        for (int h = actual; h <= parque.horaFin; h++) {
            if (revisarHueco(h, m->cantidad)) {
                sumarGrupo(h, m->cantidad);
                nueva = h;
                break;
            }
        }

        //si encontramos hora nueva, se reubica
        if (nueva != -1) {
            tardes++;
            pthread_mutex_unlock(&lockParque);
            mandarRta(m, "TARDE", nueva, "Hora solicitada ya paso, asignado en otra hora");
            printf("   -> Llego tarde, lo puse en la hora %d\n", nueva);
            return;
        } else {
            //si no hay espacio, se rechaza
            rechazadas++;
            pthread_mutex_unlock(&lockParque);
            mandarRta(m, "RECHAZADO", -1, "No hay cupo posterior disponible (hora pasada)");
            printf("   -> No se pudo reubicar (tarde y sin espacio)\n");
            return;
        }
    }

    //si pidió dentro del rango y hay espacio justo en esa hora
    if (revisarHueco(hs, m->cantidad)) {
        sumarGrupo(hs, m->cantidad);
        aceptadas++;
        pthread_mutex_unlock(&lockParque);
        mandarRta(m, "OK", hs, "Reserva aceptada en la hora solicitada");
        printf("   -> Aceptado exactamente a las %d\n", hs);
        return;
    }

    //si no hubo espacio, se intenta reprogramar a otra hora
    {
        int nueva = -1;
        for (int h = hs + 1; h <= parque.horaFin; h++) {
            if (revisarHueco(h, m->cantidad)) {
                sumarGrupo(h, m->cantidad);
                nueva = h;
                break;
            }
        }

        if (nueva != -1) {
            reprogramadas++;
            pthread_mutex_unlock(&lockParque);
            mandarRta(m, "REPROGRAMADO", nueva, "No habia cupo en la hora pedida; reprogramado");
            printf("   -> Reprogramado a la hora %d\n", nueva);
            return;
        } else {
            rechazadas++;
            pthread_mutex_unlock(&lockParque);
            mandarRta(m, "RECHAZADO", -1, "Sin cupo en ninguna franja de 2h");
            printf("   -> Rechazo total, no hay espacio.\n");
            return;
        }
    }
}

//este hilo avanza la hora y va mostrando cómo está la ocupación
void *hiloDeeReloj(void *arg) {

    while (1) {
        sleep(parque.segundosHoras);

        pthread_mutex_lock(&lockParque);
        int h = parque.horaActual;

        if (h > parque.horaFin) {
            pthread_mutex_unlock(&lockParque);
            break;
        }

        printf("[Controlador] Hora actual: %d\n", h);
        printf("   Ocupación en %d: %d\n", h, parque.ocupacion[h]);
        if (h + 1 <= 23) {
            printf("   Ocupación en %d: %d\n", h + 1, parque.ocupacion[h+1]);
        }

        parque.horaActual++;
        pthread_mutex_unlock(&lockParque);
    }

    pthread_mutex_lock(&lockParque);

    //aquí se imprimen las estadísticas finales del día
    printf("\n=== REPORTE FINAL ===\n");

    int max = -1;
    int min = 99999;

    for (int h = parque.horaInicio; h <= parque.horaFin; h++) {
        if (parque.ocupacion[h] > max) max = parque.ocupacion[h];
        if (parque.ocupacion[h] < min) min = parque.ocupacion[h];
    }

    printf("Horas pico (ocupacion = %d): ", max);
    for (int h = parque.horaInicio; h <= parque.horaFin; h++) {
        if (parque.ocupacion[h] == max) printf("%d ", h);
    }
    printf("\n");

    printf("Horas valle (ocupacion = %d): ", min);
    for (int h = parque.horaInicio; h <= parque.horaFin; h++) {
        if (parque.ocupacion[h] == min) printf("%d ", h);
    }
    printf("\n");

    printf("Solicitudes OK: %d\n", aceptadas);
    printf("Reprogramadas: %d\n", reprogramadas);
    printf("Tarde: %d\n", tardes);
    printf("Rechazadas: %d\n", rechazadas);
    printf("=== FIN DEL DIA ===\n");

    pthread_mutex_unlock(&lockParque);

    return NULL;
}

//este hilo se queda escuchando las solicitudes que llegan por el pipe
void *hiloDeSolicitudes(void *arg) {

    int f = open(pipeGeneral, O_RDONLY);
    if (f < 0) {
        perror("No pude abrir el pipe del controlador");
        pthread_exit(NULL);
    }

    while (1) {

        Mensaje m;
        ssize_t n = read(f, &m, sizeof(Mensaje));

        if (n == 0) {
            usleep(90000);
            continue;
        }

        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }

        //cuando un agente se registra
        if (strcmp(m.tipo, "REGISTER") == 0) {

            Mensaje x;
            memset(&x, 0, sizeof(x));

            strcpy(x.tipo, "TIME");
            strcpy(x.agente, m.agente);

            pthread_mutex_lock(&lockParque);
            x.hora = parque.horaActual;
            pthread_mutex_unlock(&lockParque);

            int fr = open(m.pipeRespuesta, O_WRONLY);
            if (fr >= 0) {
                write(fr, &x, sizeof(Mensaje));
                close(fr);
            }

            printf("[Controlador] Registro de %s | Pipe %s | Hora %d\n",
                   m.agente, m.pipeRespuesta, x.hora);
        }
        //cuando es solicitud normal
        else if (strcmp(m.tipo, "REQUEST") == 0) {
            gestioDeSolicitud(&m);
        }
        else {
            printf("[Controlador] Mensaje raro: %s\n", m.tipo);
        }
    }

    close(f);
    return NULL;
}

//aquí se inicia todo el controlador y se arrancan los hilos
void iniControlador(int horaIni, int horaFin, int segHoras, int capacidad, const char *pipeG) {

    parque.horaInicio = horaIni;
    parque.horaFin = horaFin;
    parque.horaActual = horaIni;
    parque.segundosHoras = segHoras;
    parque.capacidadMaxima = capacidad;

    memset(parque.ocupacion, 0, sizeof(parque.ocupacion));

    strcpy(pipeGeneral, pipeG);

    printf("[Controlador] Arrancando %d-%d | cap=%d | seg/hora=%d\n",
           horaIni, horaFin, capacidad, segHoras);

    if (mkfifo(pipeGeneral, 0666) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo controlador");
            exit(1);
        }
    }

    pthread_t h1, h2;
    pthread_create(&h1, NULL, hiloDeeReloj, NULL);
    pthread_create(&h2, NULL, hiloDeSolicitudes, NULL);

    pthread_join(h1, NULL);

    pthread_cancel(h2);
    pthread_join(h2, NULL);
}
