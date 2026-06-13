/**
 * @file Strukury.h
 * @brief Definicje podstawowych struktur danych, stałych i parametrów konfiguracyjnych silnika.
 * @details Grupuje najważniejsze struktury typu Pod (Plain Old Data) reprezentujące m.in. światła, 
 * układy współrzędnych, zdarzenia zderzeń i konfigurację wielowątkowości.
 */
#pragma once
#include "Wektor3D.h"
#include <vector>
#include "SFML/Graphics.hpp"
#include <optional>

/** @brief Domyślna liczba wątków (może być nadpisana w JSON). */
const int LICZBA_WATKOW = 4;

/** @brief Długość boku głównego okna programu i tekstury w pikselach. */
const int DLUGOSC = 800;

/** @brief Rozmiar boku pojedynczego kafelka obszaru renderowania wątku. */
const int ROZMIAR_KAFELKA = 32;

/** @brief Obliczona liczba kafelków na osi X. */
const int LICZBA_KAWELKOW_X = (DLUGOSC+ROZMIAR_KAFELKA -1)/ROZMIAR_KAFELKA;
/** @brief Obliczona liczba kafelków na osi Y. */
const int LICZBA_KAWELKOW_Y = (DLUGOSC+ROZMIAR_KAFELKA -1)/ROZMIAR_KAFELKA;
/** @brief Całkowita ilość kafelków do przetworzenia w jednej klatce. */
const int CALKOWITA_LICZBA_KAFELKOW = LICZBA_KAWELKOW_X*LICZBA_KAWELKOW_Y;

/** @brief Stała definiująca fizyczny promień gracza dla systemu detekcji kolizji. */
const float ROZMIAR_POSTACI = 0.5f;

/** @brief Standardowa moc światła podpiętego do kamery gracza (czołówka). */
const float MOC_CZOLOWKI =2.0f;

/**
 * @struct Uklad_wspolrzednych
 * @brief Reprezentacja lokalnego układu współrzędnych (przestrzeni kamery).
 * @details Zawiera wektory ortonormalne U, V i W wyznaczające orientację płaszczyzny ekranowej dla promieni.
 */
struct Uklad_wspolrzednych {
    Wektor3D W; /**< @brief Wektor głębokości (patrzenia wstecz/w przód kamery). */
    Wektor3D U; /**< @brief Wektor poziomy ekranu (prawo/lewo). */
    Wektor3D V; /**< @brief Wektor pionowy ekranu (góra/dół). */
};

/**
 * @struct Zrodlo_swiatla
 * @brief Dane opisujące pojedyncze źródło światła na scenie.
 */
struct Zrodlo_swiatla {
    Wektor3D srodek;                /**< @brief Punkt umiejscowienia światła. */
    Wektor3D kolor;                 /**< @brief Barwa światła. */
    float moc_emisji;               /**< @brief Siła emisji (natężenie). */
    Wektor3D kierunek_swiecenia;    /**< @brief Dla świateł kierunkowych/reflektorów. */
    float kat_swiecenia;            /**< @brief Kąt stożka dla reflektorów (kąt półpełny). */
};

/**
 * @struct KontekstWatkow
 * @brief Bufory robocze oraz lokalne kopie danych wymagane przez wątki renderujące.
 * @details Zapobiega to rywalizacji o zasoby, zapewniając każdemu wątkowi dostęp do aktualnej klatki 
 * fizycznych własności sceny oraz własny zestaw wektorów wyjściowych obrazu przed synchronizacją główną.
 */
struct KontekstWatkow{
    std::vector<sf::Uint8> bufor_roboczy;           /**< @brief Główny bufor kolorów RGBA pikseli. */
    std::vector<sf::Uint8> post_procesing_bufor;    /**< @brief Bufor pomocniczy dla blur'a / post-processingu. */


    Wektor3D kamera_copy;                           /**< @brief Kopia wektora pozycji kamery na daną klatkę. */
    Uklad_wspolrzednych uklad_copy;                 /**< @brief Kopia orientacji kamery (W, U, V). */
    std::vector<Zrodlo_swiatla> swiatla_copy;       /**< @brief Kopia listy świateł sceny na daną klatkę. */
};





/**
 * @struct WynikOswietlenia
 * @brief Rezultat kalkulacji podstawowych właściwości koloru oświetlonego punktu bez refleksji wtórnych.
 */
struct WynikOswietlenia {
    Wektor3D kolor_matowy; /**< @brief Kolor z uwzględnieniem cieniowania wariantu matowego (diffuse). */
    float maska_blasku;    /**< @brief Wyliczony współczynnik służący do bloom / blasku wokół najjaśniejszych stref. */
};

/**
 * @struct Promien
 * @brief Promień (ray) wypuszczany w przestrzeń.
 * @details Podstawa algorytmu śledzenia promieni: wektor ułożenia i wektor kierunku.
 */
struct Promien {
    Wektor3D kierunek; /**< @brief Znormalizowany wektor kierunku lotu. */
    Wektor3D poczatek; /**< @brief Punkt startowy promienia. */
};

/**
 * @struct Parametry_obiektow
 * @brief Zbiór opcjonalnych parametrów wczytywanych z pliku konfiguracyjnego dla brył.
 */
struct Parametry_obiektow{
    std::optional<Wektor3D> kolor;
    std::optional<Wektor3D> pozycja;
    std::optional<float> rozmiar; 
    std::optional<float> lustrzanosc;
    std::optional<float> metalicznosc;
    std::optional<float> moc_emisji;

};

/**
 * @struct WynikZdarzenia
 * @brief Informacje zebrane po trafieniu promienia w powierzchnię obiektu.
 * @details Służy do propagowania i generowania promieni odbitych oraz obliczania koloru piksela w danym punkcie przecięcia.
 */
struct WynikZdarzenia {
    bool trafienie;             /**< @brief Prawda jeśli nastąpiło przecięcie bryły z promieniem. */
    float t;                    /**< @brief Odległość od początku promienia do miejsca trafienia. */
    Wektor3D wektor_normalny;   /**< @brief Wektor prostopadły do powierzchni w punkcie trafienia. */
    Wektor3D punkt_zderzenia;   /**< @brief Bezwzględna współrzędna uderzenia 3D. */
    Wektor3D kolor;             /**< @brief Baza koloru bryły. */
    Promien promien_odbity;     /**< @brief Wygenerowany promień na wypadek odbić (rekurencja ograniczona iteracją). */
    float lustrzanosc;          /**< @brief Siła odbić lustrzanych (0.0 do 1.0). */
    float moc_emisji;           /**< @brief Siła światła własnego bryły (dla świecących materiałów). */
    float metalicznosc;         /**< @brief Jak bardzo kolor odbicia jest podyktowany przez kolor własny materiału. */
};

/**
 * @struct AABB
 * @brief Axis-Aligned Bounding Box używany do przyspieszania detekcji kolizji za pomocą drzewa BVH.
 */
struct AABB {
    Wektor3D min_;
    Wektor3D max_;
    
    AABB() : min_(1e9f, 1e9f, 1e9f), max_(-1e9f, -1e9f, -1e9f) {}
    AABB(Wektor3D minimum, Wektor3D maximum) : min_(minimum), max_(maximum) {}
    
    void rozszerz(const Wektor3D& p) {
        min_.set_x(std::min(min_.x(), p.x()));
        min_.set_y(std::min(min_.y(), p.y()));
        min_.set_z(std::min(min_.z(), p.z()));
        max_.set_x(std::max(max_.x(), p.x()));
        max_.set_y(std::max(max_.y(), p.y()));
        max_.set_z(std::max(max_.z(), p.z()));
    }
    
    void rozszerz(const AABB& b) {
        rozszerz(b.min_);
        rozszerz(b.max_);
    }
    
    bool przecina(const Promien& p, float t_min, float t_max) const {
        for (int i = 0; i < 3; i++) {
            float invD = 1.0f / p.kierunek[i];
            float t0 = (min_[i] - p.poczatek[i]) * invD;
            float t1 = (max_[i] - p.poczatek[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min) return false;
        }
        return true;
    }
};