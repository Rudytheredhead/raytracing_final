/**
 * @file Matematyka.cpp
 * @brief Implementacja funkcji przestrzennych, cieniowania i postprocessingu (Bloom).
 */
#include "Wektor3D.h"
#include "Matematyka.h"
#include <algorithm>




Uklad_wspolrzednych obliczanie_ukladu_wspolrzednych(const Wektor3D& kamera, const Wektor3D &cel, const Wektor3D & gora) {
    Wektor3D W = kamera - cel;
    W.normalizuj();
    Wektor3D U = gora % W;
    U.normalizuj();
    Wektor3D V = W % U;
    V.normalizuj();
    Uklad_wspolrzednych uklad{W, U, V};
    return uklad;
}

Wektor3D oblicz_kierunek_promienia(float grid_x, float grid_y, float odleglosc_od_ekranu, Uklad_wspolrzednych &uklad) {
    grid_x /= DLUGOSC;
    grid_y /= DLUGOSC;
    grid_x = grid_x * 2.0f - 1.0f;
    grid_y = grid_y * 2.0f - 1.0f;

    Wektor3D kierunek = (grid_x * uklad.U) - (grid_y * uklad.V) - (odleglosc_od_ekranu * uklad.W);
    kierunek.normalizuj();
    return kierunek;
}

WynikOswietlenia oblicz_oswietlenie(WynikZdarzenia& zderzenie, const std::vector<Zrodlo_swiatla>& swiatla, const std::vector<std::unique_ptr<Obiekt3D>>& obiekty) {
    Wektor3D calkowite_energia_swiatla(0.0f, 0.0f, 0.0f);
    
    for (const auto& swiatlo : swiatla) {
        Wektor3D droga_swiatla = swiatlo.srodek - zderzenie.punkt_zderzenia;
        float odleglosc_od_swiatla = droga_swiatla.modul();
        
        Wektor3D kierunek_do_swiatla = droga_swiatla;
        kierunek_do_swiatla.normalizuj();
        
        float moc_swiatla_od_kata = 1.0f;

        Promien promien_cienia;
        promien_cienia.poczatek = zderzenie.punkt_zderzenia + 0.001f*zderzenie.wektor_normalny;
        promien_cienia.kierunek = kierunek_do_swiatla;

        bool w_cieniu = false;
        WynikZdarzenia wyniki_cien;
        float t_min = 0.001f;
        
        for (const auto& obiekt : obiekty) {
            if (obiekt->sprawdz_trafienie(promien_cienia, wyniki_cien, t_min, odleglosc_od_swiatla)) {
                
                w_cieniu = true;
                break;
            }
        }
        
        if (!w_cieniu) {
            
            if (swiatlo.kat_swiecenia > -1.0f) {
                moc_swiatla_od_kata = 0.0f;
                Wektor3D ujemny_kierunek =  -1.0f*kierunek_do_swiatla ;
                float kat_swiatla = ujemny_kierunek * swiatlo.kierunek_swiecenia;
                if (kat_swiatla > swiatlo.kat_swiecenia) {
                    moc_swiatla_od_kata = (kat_swiatla - swiatlo.kat_swiecenia) / (1.0f - swiatlo.kat_swiecenia);
                }
            }

            float natezenie = std::max(0.0f, zderzenie.wektor_normalny * kierunek_do_swiatla);
            float tlumienie = 1.0f / (odleglosc_od_swiatla * odleglosc_od_swiatla);
            
            Wektor3D wynik_pojedynczego =  (swiatlo.moc_emisji * natezenie * tlumienie * moc_swiatla_od_kata)* swiatlo.kolor;
            calkowite_energia_swiatla = calkowite_energia_swiatla + wynik_pojedynczego;
        }
       
    }
    
    WynikOswietlenia wynik;
    Wektor3D ambient(0.05f, 0.05f, 0.05f);
    wynik.kolor_matowy = zderzenie.kolor.przemnoz(calkowite_energia_swiatla+ambient);
    
    
    wynik.maska_blasku = calkowite_energia_swiatla.x() * zderzenie.lustrzanosc; 
    
    return wynik;
}

void rozmycie_jasnych_punktow_w_poziomie(std::vector<sf::Uint8> &pixels_odczyt,std::vector<sf::Uint8> &pixels_zapis,int start_y , int koniec_y){
    const int promien_rozmycia = 4;
    const float kolor_swiatla_r = 1.0f;
    const float kolor_swiatla_g = 0.95f;
    const float kolor_swiatla_b = 0.8f;

    for(int y=start_y;y<koniec_y; y++){
        for (int x=0;x<DLUGOSC ;x++){
            float suma_blasku = 0.0f;
            float ilosc_probek = 0.0f;

            for(int k = -promien_rozmycia;k<=promien_rozmycia;k++){
                int grid_x = std::clamp(x+k,0,DLUGOSC-1);
                int idx = (y*DLUGOSC + grid_x)*4;
                suma_blasku += pixels_odczyt[idx+3]/255.0f;
                ilosc_probek+=1.0f;
            }
            int idx_docelowy = (DLUGOSC*y + x)*4;
            float blask = suma_blasku/ilosc_probek;
            pixels_zapis[idx_docelowy+3] =static_cast<sf::Uint8>(blask*255.0f);

        }
    }
}

void rozmycie_jasnych_punktow_w_pionie(std::vector<sf::Uint8> &pixels_odczyt,std::vector<sf::Uint8> &pixels_zapis,int start_y , int koniec_y){
    const int promien_rozmycia = 4;
    Wektor3D kolor_swiatla(1.0f,0.95f,0.8f);

    for(int y=start_y;y<koniec_y; y++){
        for (int x=0;x<DLUGOSC;x++){
            float suma_blasku = 0.0f;
            float ilosc_probek = 0.0f;

            for(int k = -promien_rozmycia;k<=promien_rozmycia;k++){
                int grid_y = std::clamp(y+k,0,DLUGOSC-1);
                int idx = (grid_y*DLUGOSC + x)*4;
                suma_blasku += pixels_zapis[idx+3]/255.0f;
                ilosc_probek+=1.0f;
            }
            int idx_docelowy = (DLUGOSC*y + x)*4;
            float blask = suma_blasku/ilosc_probek;
            
            Wektor3D kolor_orginalny(
                pixels_odczyt[idx_docelowy]/255.0f,
                pixels_odczyt[idx_docelowy+1]/255.0f,
                pixels_odczyt[idx_docelowy+2]/255.0f
            );
            Wektor3D kolor_out;
            kolor_out = kolor_orginalny + (blask*kolor_swiatla);
            

            pixels_odczyt[idx_docelowy] = std::clamp(static_cast<int>(kolor_out.x()*255.0f),0,255) ;
            pixels_odczyt[idx_docelowy+1] = std::clamp(static_cast<int>(kolor_out.y()*255.0f),0,255) ;
            pixels_odczyt[idx_docelowy+2] = std::clamp(static_cast<int>(kolor_out.z()*255.0f),0,255) ;
            pixels_odczyt[idx_docelowy+3] =255;

        }

    }
}