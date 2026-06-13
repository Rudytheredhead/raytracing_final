# Raytracer

Prosty i wydajny silnik raytracingu napisany w języku C++. Program renderuje trójwymiarowe sceny z wykorzystaniem biblioteki SFML, obsługując różne obiekty geometryczne, oświetlenie oraz zaawansowane właściwości materiałów (takie jak lustrzaność czy metaliczność).

## Wymagania
Aby skompilować i uruchomić projekt, potrzebujesz:
* Kompilatora C++ z pełną obsługą standardu **C++20** (np. `g++`).
* Biblioteki **SFML** (`sfml-graphics`, `sfml-window`, `sfml-system`).
* Narzędzia **Make**.

## Kompilacja
Projekt używa pliku `Makefile` do zautomatyzowania procesu kompilacji. Zastosowano flagi optymalizacyjne `-O3` oraz `-ffast-math`, aby zapewnić jak najwyższą wydajność obliczeń.

W głównym folderze projektu wpisz w terminalu:
make

Zostanie wygenerowany plik wykonywalny o nazwie `raytracing` (na systemach Linux/macOS) lub `raytracing.exe` (na systemach Windows).

## Uruchomienie
Aby uruchomić skompilowany program:
./raytracing

## Konfiguracja Sceny (`parametry_wejsciowe.json`)
Scena renderowana przez aplikację jest konfigurowana w locie za pomocą pliku `parametry_wejsciowe.json`. Pozwala to na swobodne testowanie bez konieczności ponownej kompilacji. Plik ten ustala:
* **Ustawienia silnika:** w tym liczbę rdzeni używanych do wielowątkowego renderowania.
* **Światła:** definiowanie źródeł światła, ich pozycji, koloru, kierunku, kąta świecenia oraz mocy emisji.
* **Obiekty (Bryły):** możliwość umieszczania na scenie kul (`Kula`) i sześcianów (`Szescian`), wraz z określeniem ich właściwości takich jak rozmiar (promień lub połowa boku), pozycja, kolor, metaliczność (0.0 - 1.0) oraz lustrzaność.

## Struktura Kodu
Aplikacja została podzielona na modułowe komponenty:
* `main.cpp` – punkt wejścia, główna pętla aplikacji i obsługa okna.
* `Raytracer` – główny silnik obliczający przecięcia promieni z obiektami i cieniowanie.
* `Bryly` – matematyczne definicje i testy przecięć dla obiektów (np. Kula, Sześcian).
* `Kamera` – obsługa widoku i rzutowania promieni w głąb sceny.
* `Wektor3D` i `Matematyka` – struktury danych oraz optymalne operacje wektorowe.
* `Wczytywanie` – moduł odczytujący parametry środowiskowe i obiekty z pliku JSON.