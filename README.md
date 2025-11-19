Proyecto: Sistema de Reservas del Parque
Este proyecto implementa un controlador y un agente que se comunican usando pipes FIFO en Linux. El controlador maneja el tiempo simulado, la capacidad del parque y todas las solicitudes que llegan de los agentes. El agente lee un archivo CSV con familias, horas y cantidades, y va enviando cada solicitud por tuberías para que el controlador responda si se acepta, se reprograma o se rechaza.

Para probarlo solo se compila con make, se ejecuta primero el controlador indicando horarios, aforo y el pipe principal, y luego el agente con su nombre, el archivo CSV y el pipe del controlador. Ambos programas funcionan sin ejecutables incluidos, y el informe completo con más detalles se entrega aparte.
