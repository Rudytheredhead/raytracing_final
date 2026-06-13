# 1. Wykrywanie systemu operacyjnego
ifeq ($(OS),Windows_NT)
    EXEC = raytracing.exe
else
    EXEC = raytracing
endif

# 2. Lista plików źródłowych (żeby nie pisać ich dwa razy)
SOURCES = main.cpp Bryly.cpp Wektor3D.cpp Matematyka.cpp Wczytywanie.cpp

# 3. Główna reguła kompilacji
all: $(EXEC)

$(EXEC): $(SOURCES)
	g++ -std=c++20 $(SOURCES) -o $(EXEC) -lsfml-graphics -lsfml-window -lsfml-system -O3 -march=native -ffast-math