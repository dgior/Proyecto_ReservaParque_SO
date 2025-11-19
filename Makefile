###########################################################
#  Pontificia Universidad Javeriana
#  Autores: 
#      - Giovanny Andres Duran Renteria
#      - Christian Becerra Enciso
#  Materia: Sistemas Operativos
#  Proyecto: Sistema de Reservas del Parque
#  Objetivo: Automatizar el proceso de compilación de los 
#           programas de agente y controlador
# 	    para el proyecto de reserva del Parque
#  Fecha: 18-11-2025
###########################################################

GCC = gcc
FLAGS = -Wall -pthread -Iinclude

controladorr:
	$(GCC) controlador/controlador.c controlador/mainControllador.c -o controladorr $(FLAGS)

agentee:
	$(GCC) agente/agente.c agente/mainAgente.c -o agentee $(FLAGS)

clean:
	$(RM) controladorr agentee /tmp/pipe_* /tmp/pipeControl
