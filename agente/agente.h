/*************************************************************
 *          Pontificia universidad Javeriana
 *  Autores: Giovanny Andrés Durán Rentería
 *           Christian Becerra Enciso
 *  Materia: Sistemas Operativos
 *  Fecha: 18/11/2025
 *  Tema: Proyecto – Reserva de Parque (Agente)
 *  =====================================================
 *  Resumen:
 *  Archivo de cabecera del agente. Contiene las funciones
 *  necesarias para registrar el agente con el controlador
 *  y enviar las solicitudes del archivo CSV.
 *************************************************************/
#ifndef AGENTE_H
#define AGENTE_H

#include <stddef.h>            
#include "../include/mensajes.h"

// Envía todas las solicitudes del archivo .csv
void enviarSolicitudes(const char *CSVArhivo,
                         const char *nombreAgente,
                         const char *pipeControlador,
                         const char *pipeRespAgente,
                         int horaInicial);
                         
// Crea pipe propio
// Devuelve la hora simulada (>=0) o -1 si falla
int iniciarAgente(const char *nombreAgente,
                    const char *pipeControlador,
                    char *pipeRespuesta,
                    size_t tamanoMaximo);



#endif
