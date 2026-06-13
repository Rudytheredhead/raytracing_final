/**
 * @file Matematyka.h
 * @brief Obliczenia globalne związane z generowaniem promieni oświetleniem i postprocessingiem.
 * @details Moduł grupuje globalne procedury służące do kluczowych etapów renderowania. 
 * Obejmuje transformacje przestrzenne w obrębie kamery, weryfikację wpływu oświetlenia 
 * punktowego i kierunkowego na dany piksel (cieniowanie, odbicia matowe i lustrzane) oraz 
 * implementację dwuprzebiegowego rozmycia blasku jasnych obszarów (efekt Bloom/Glare).
 */
#pragma once
#include <SFML/Graphics.hpp>
#include "Wektor3D.h"
#include "Bryly.h"
#include "Strukury.h"
#include <vector>
#include <memory>





/**
 * @brief Konstruuje lokalny, ortogonalny układ współrzędnych dla kamery.
 * @param kamera Wektor położenia kamery w przestrzeni.
 * @param cel Punkt na który skierowana jest kamera.
 * @param gora Globalny wektor określający orientację w górę.
 * @return Zbudowany układ współrzędnych zawierający bazy kierunkowe U, V, W.
 */
Uklad_wspolrzednych obliczanie_ukladu_wspolrzednych(const Wektor3D& kamera, const Wektor3D &cel, const Wektor3D & gora);

/**
 * @brief Konwertuje pozycję piksela ekranowego na wektor 3D wyznaczający kierunek rzucania promienia w głąb sceny.
 * @param grid_x Pozycja w osi X danego piksela na płótnie.
 * @param grid_y Pozycja w osi Y danego piksela na płótnie.
 * @param odleglosc_od_ekranu Skala ogniskowej (rzutni), zależy od niej pole widzenia (FOV).
 * @param uklad Układ odniesienia stworzony dla kamery.
 * @return Znormalizowany wektor promienia dla głównej wiązki widoku.
 */
Wektor3D oblicz_kierunek_promienia(float grid_x, float grid_y, float odleglosc_od_ekranu, Uklad_wspolrzednych &uklad);

/**
 * @brief Algorytm wyliczający odpowiedź oświetlenia dla wyznaczonego punktu przecięcia (Phong/Lambert).
 * @param zderzenie Wygenerowany zbiór informacji o zderzeniu (punkt trafienia, normalna powierzchni, surowy kolor bryły).
 * @param swiatla Wektor wszystkich aktywnych źródeł światła na scenie.
 * @param obiekty Spis brył, wykorzystywany do rzucania promieni wstecznych i weryfikacji twardych cieni (Shadow Rays).
 * @return Pakiet rezultatów kolorystycznych i współczynnik jasności wykorzystywany później w procesie bloom.
 */
WynikOswietlenia oblicz_oswietlenie(WynikZdarzenia& zderzenie, const std::vector<Zrodlo_swiatla>& swiatla, const std::vector<std::unique_ptr<Obiekt3D>>& obiekty);

/**
 * @brief Część algorytmu Post-Processingowego (Bloom). Filtruje i nakłada uśrednienie blasku pikseli pionowo.
 * @param pixels_odczyt Wejściowy zbuforowany zasób grafiki z wyrenderowaną sceną na czysto.
 * @param pixels_zapis Osobny bufor roboczy, do którego wędrują uśrednione próbki po przejściu jądra filtrującego.
 * @param start_y Zakres początkowy wiersza ramy roboczej wątku.
 * @param koniec_y Zakres końcowy wiersza ramy roboczej wątku.
 */
void rozmycie_jasnych_punktow_w_pionie(std::vector<sf::Uint8> &pixels_odczyt,std::vector<sf::Uint8> &pixels_zapis,int start_y , int koniec_y);

/**
 * @brief Część algorytmu Post-Processingowego (Bloom). Filtruje i nakłada uśrednienie blasku pikseli poziomo.
 * @param pixels_odczyt Wejściowy zbuforowany zasób na którym już operowano horyzontalnie / surowy zasób.
 * @param pixels_zapis Bufor przeznaczony do otrzymania ostatecznego szumu świetlnego na danej osi.
 * @param start_y Zakres początkowy wiersza dla operacji poziomej w podziale na ten wątek.
 * @param koniec_y Zakres końcowy wiersza ramy roboczej wątku.
 */
void rozmycie_jasnych_punktow_w_poziomie(std::vector<sf::Uint8> &pixels_odczyt,std::vector<sf::Uint8> &pixels_zapis,int start_y , int koniec_y);