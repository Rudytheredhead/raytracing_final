/**
 * @file Wczytywanie.cpp
 * @brief Implementacja mechanizmów budowy obiektów 3D z użyciem biblioteki nlohmann json.
 */
#include "Wczytywanie.h"

#include <vector>
#include <fstream>
#include <sstream>
#include "json.hpp"

std::map<unsigned, KreatorObiektu3D> FabrykaObiektow::kreatory_;
std::map<unsigned, std::string> FabrykaObiektow::nazwy_;
unsigned FabrykaObiektow::nastepne_id_ =1;

std::unique_ptr<Obiekt3D> FabrykaObiektow::utworz(std::string nazwa,const nlohmann::json &dane ){
    unsigned id =0;
    for (const auto&para : nazwy_){
        if(para.second == nazwa){
            id = para.first;
            break;
        }
    }
    
    if (kreatory_.find(id) != kreatory_.end() && id != 0){
        return kreatory_[id](dane);
    }
    return nullptr;
}

/**
 * @brief Weryfikuje czy niezbędne atrybuty dla standardowego obiektu są obecne.
 * @param parametry Zestaw parametrów opcjonalnych.
 * @return True jeśli wszystkie kluczowe parametry mają przypisaną wartość, false w przeciwnym razie.
 */
bool sprawdzenie_parametrow(const Parametry_obiektow &parametry){

    if(!parametry.kolor.has_value()) return false;
    else if(!parametry.pozycja.has_value()) return false;
    else if(!parametry.rozmiar.has_value()) return false;
    else if(!parametry.lustrzanosc.has_value()) return false;
    else if(!parametry.moc_emisji.has_value()) return false;
    else if(!parametry.metalicznosc.has_value()) return false;

    return true;

}

bool wczytaj_scene(const std::string& sciezka, 
                   std::vector<std::unique_ptr<Obiekt3D>>& obiekty, 
                   std::vector<Zrodlo_swiatla>& swiatla, 
                   int& liczba_rdzeni,
                   std::string& globalna_tekstura) { 
                   
    std::ifstream plik(sciezka);
    if (!plik.is_open()) return false;

    nlohmann::json j;
    plik >> j; 

   
    if (j.contains("ustawienia")) {
        liczba_rdzeni = j["ustawienia"].value("liczba_rdzeni", 4);
        if (j["ustawienia"].contains("globalna_tekstura")) {
            globalna_tekstura = j["ustawienia"]["globalna_tekstura"];
        }
    } else {
       
        liczba_rdzeni = 4; 
    }

   
    if (j.contains("swiatla")) {
        for (const auto& element : j["swiatla"]) {
            Zrodlo_swiatla swiatlo;
            Wektor3D kierunek_swiecenia(
                element["kierunek_swiecenia"][0],
                element["kierunek_swiecenia"][1],
                element["kierunek_swiecenia"][2]
            );
            Wektor3D kolor(
                element["kolor"][0],
                element["kolor"][1],
                element["kolor"][2]
            );
            Wektor3D srodek(
                element["pozycja"][0],
                element["pozycja"][1],
                element["pozycja"][2]
            );
            swiatlo.kat_swiecenia = element["kat_swiecenia"];
            swiatlo.kierunek_swiecenia = kierunek_swiecenia;
            swiatlo.kolor = kolor;
            swiatlo.moc_emisji = element["moc_emisji"];
            swiatlo.srodek = srodek;
            swiatla.push_back(swiatlo);
        }
    }

    
    if (j.contains("obiekty")) {
        for (const auto& element : j["obiekty"]) {
            std::string typ = element["typ"];
            
            
            auto obiekt = FabrykaObiektow::utworz(typ, element); 
            if (obiekt) {
                obiekty.push_back(std::move(obiekt));
            }
        }
    }
    
    return true;
}