# Nombre del ejecutable
EXECUTABLE = main

# Archivos fuente
SRC = main.c


# Bibliotecas necesarias
LIBS = -lpthread

# Regla para compilar el ejecutable
$(EXECUTABLE): $(SRC)
	gcc -o $(EXECUTABLE) $(SRC) $(LIBS)

# Regla para limpiar archivos objeto y ejecutable
clean:
	rm -f $(EXECUTABLE)

# Regla para compilar y ejecutar
run: $(EXECUTABLE)
	./$(EXECUTABLE)