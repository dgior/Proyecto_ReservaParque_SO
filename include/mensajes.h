/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Estructuras de Mensajes)
 *  Resumen:
 *  Archivo con la estructura usada para el envío de
 *  información entre el agente y el controlador.
 *************************************************************/

#ifndef MENSAJES_H
#define MENSAJES_H

#define MAX_NOMBRE   50
#define MAX_PIPE     100
#define MAX_MENSAJE  256
#define MAX_MOTIVO   128

//estructura general para todos los mensajes entre procesos
typedef struct {
    char tipo[20];
    char agente[MAX_NOMBRE];
    char familia[MAX_NOMBRE];
    int  hora;
    int  cantidad;
    char pipeRespuesta[MAX_PIPE];
    char motivo[MAX_MOTIVO];
} Mensaje;

#endif

