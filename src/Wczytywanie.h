/**
 * @file Wczytywanie.h
 * @brief Odczytywanie zasobów z plików konfiguracyjnych JSON.
 * @details Udostępnia interfejs Fabryki dla Brył (obiektów 3D) oraz mechanizmy wczytujące właściwości 
 * sceny (światła, liczbę rdzeni, konfigurację obiektów).
 */
#pragma once
#include <memory>
#include "Bryly.h"


/**
 * @typedef KreatorObiektu3D
 * @brief Wskaźnik na funkcję tworzącą obiekt typu Obiekt3D na podstawie danych z JSON.
 */
using KreatorObiektu3D = std::unique_ptr<Obiekt3D>(*)(const nlohmann::json&);



/**
 * @class FabrykaObiektow
 * @brief Statyczna fabryka rejestrująca i alokująca obiekty z danych wejściowych JSON.
 * @details Pozwala na dynamiczne tworzenie obiektów potomnych Obiekt3D (jak Kula, Szescian) na podstawie 
 * łańcucha znaków (nazwy typu) odczytanego z pliku konfiguracyjnego, bez modyfikacji kodu klienta dla nowych typów.
 */
class FabrykaObiektow{
private:
    static std::map<unsigned,KreatorObiektu3D> kreatory_;
    static std::map<unsigned,std::string> nazwy_;
    static unsigned nastepne_id_;
public:
    /**
     * @brief Rejestruje nowy typ obiektu w fabryce.
     * @param kr Wskaźnik na funkcję kreującą obiekt.
     * @param nazwa Nazwa identyfikująca typ obiektu w pliku JSON.
     */
    static void rejestruj(KreatorObiektu3D kr, std::string nazwa){
        kreatory_[nastepne_id_] =kr;
        nazwy_[nastepne_id_] = nazwa;
        nastepne_id_ ++;
    }
    
    /**
     * @brief Instancjonuje obiekt na podstawie jego zarejestrowanej nazwy.
     * @param nazwa Zarejestrowana nazwa typu (np. "Kula").
     * @param dane Dane konfiguracyjne z obiektu JSON potrzebne do inicjalizacji bryły.
     * @return Wskaźnik na utworzony obiekt bazowy Obiekt3D lub nullptr w razie błędu.
     */
    static std::unique_ptr<Obiekt3D> utworz(std::string nazwa,const nlohmann::json &dane);
};

/**
 * @brief Główna funkcja parsująca plik konfiguracji sceny.
 * @param sciezka Ścieżka do pliku JSON.
 * @param obiekty Wektor do którego załadowane zostaną zidentyfikowane obiekty.
 * @param swiatla Wektor do którego załadowane zostaną światła.
 * @param liczba_rdzeni Zmienna, do której zostanie przepisana żądana liczba wątków z konfiguracji.
 * @return True jeśli plik poprawnie załadowano, false w przypadku problemów z odczytem.
 */
bool wczytaj_scene(const std::string& sciezka, 
                   std::vector<std::unique_ptr<Obiekt3D>>& obiekty, 
                   std::vector<Zrodlo_swiatla>& swiatla, 
                   int& liczba_rdzeni,
                   std::string& globalna_tekstura);