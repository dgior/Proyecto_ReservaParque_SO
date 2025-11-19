/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Header del Controlador)
 *  Resumen:
 *  En este archivo se definen las estructuras y funciones
 *  que usa el controlador del sistema.
 *************************************************************/

#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include "../include/mensajes.h"

//estructura que guarda el estado del parque
typedef struct {
    int horaInicio;
    int horaActual;
    int horaFin;
    int segundosHoras;
    int capacidadMaxima;
    int ocupacion[24];
} EstadoParque;

//hilo que avanza la hora y muestra la ocupación
void *hiloDeeReloj(void *arg);

//hilo que recibe todas las solicitudes de los agentes
void *hiloDeSolicitudes(void *arg);

//función que procesa cada solicitud que llega
void gestioDeSolicitud(Mensaje *msg);

//inicia todo el controlador y crea los hilos
void iniControlador(int horaIni, int horaFin, int segHoras, int capacidad, const char *pipeGeneral);

#endif
