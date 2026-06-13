/**
 * @file Bryly.h
 * @brief Deklaracje klas dla abstrakcyjnego obiektu 3D oraz konkretnych brył: Kula, Szescian.
 * @details Hierarchia klas oparta na wzorcu polimorfizmu, ułatwiająca łatwą iterację w pętli renderującej 
 * i silniku fizycznym. Rozszerzana jest przez konkretne kształty geometryczne posiadające 
 * własne matematyczne równania przecięć i kolizji.
 */
#pragma once
#include "Wektor3D.h"
#include "Strukury.h"
#include "json.hpp"

#include <memory>
#include <vector>
#include <map>
#include <optional>






/**
 * @class Obiekt3D
 * @brief Klasa abstrakcyjna będąca interfejsem dla wszystkich brył renderowanych na scenie.
 * @details Udostępnia zestaw czysto wirtualnych metod (interfejs) gwarantujących obsługę zjawisk ray-tracingu (trafienia promienia) 
 * i fizyki świata (kolizji ze sferycznym graczem).
 */
class Obiekt3D {
protected:
    Wektor3D kolor_; /**< @brief Podstawowy bazowy kolor obiektu. */
public:
    Obiekt3D(Wektor3D kolor): kolor_(kolor) {}
    virtual ~Obiekt3D() = default;
    
    Wektor3D get_kolor() const { return kolor_; }
    
    /**
     * @brief Weryfikuje punkt oraz kąt uderzenia rzuconego wektora w dany obiekt 3D.
     * @param promien Matematycznie sformułowany wektor promienia ze swoim początkiem i kierunkiem.
     * @param wyniki Pakiet danych wyjściowych zostawiający komplet informacji w przypadku trafienia (kolor, punkt zderzenia, normalna, refleksy).
     * @param t_min Zabezpieczenie przed artefaktami i za-szybkim uderzaniem promieni w źródła z których startują.
     * @param t_max Dystans maksymalny zjawiska zderzenia z promieniem.
     * @return Prawda - promien przetnie ten kształt. Fałsz - brak przecięcia.
     */
    virtual bool sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const = 0;
    
    void test(){std::cout<<kolor_;};
    
};


/**
 * @class Kula
 * @brief Implementacja sfery, posiada zdefiniowany matematyczny środek i promień.
 */
class Kula : public Obiekt3D {
private:
    Wektor3D srodek_;       /**< @brief Punkt środkowy w układzie świata 3D. */
    float promien_;         /**< @brief Odległość od środka kuli na jej brzeg. */
    float lustrzanosc_;     /**< @brief Odczynnik odbijający (np. 1.0 to idealne lustro). */
    float metalicznosc_;    /**< @brief Zabarwia obicia w kolorze materiału (w przeciwieństwie do tworzywa sztucznego bez zabarwienia refleksji). */
    float moc_emisji_;      /**< @brief Obiekt oddaje własne światło uderzającemu mu promieniowi. */
    
public:
    /**
     * @brief Konstruktor dla Kuli inicjujący materiał i jej parametry.
     */
    Kula(Wektor3D kolor, Wektor3D srodek, float promien, float lustrzanosc = 0.0f,float metalicznosc =0.0f, float moc_emisji = 0.0f)
        : Obiekt3D(kolor), srodek_(srodek), promien_(promien), lustrzanosc_(lustrzanosc),metalicznosc_(metalicznosc), moc_emisji_(moc_emisji) {};
    
    /**
     * @brief Fabryczna funkcja kreatora statycznego wczytującego model na podstawie definicji JSON.
     */
    static std::unique_ptr<Obiekt3D> kreator(const nlohmann::json &dane);
    
    bool sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const override;
    

    
    ~Kula() = default;
};

/**
 * @class Szescian
 * @brief Implementacja sześcianu wyrównanego do osi układu (Axis-Aligned Bounding Box - AABB).
 */
class Szescian : public Obiekt3D {
private:
    Wektor3D srodek_;       /**< @brief Pozycja środka geometrycznego sześcianu. */
    float polowa_boku_;     /**< @brief Wymiar odległości od środka ściany wewnętrznej do srodka sześcianu (bok/2). */
    float lustrzanosc_;     /**< @brief Odczynnik materiału (lustro). */
    float metalicznosc_;    /**< @brief Odczynnik materiału zabarwionego refleksji metalu. */
    float moc_emisji_;      /**< @brief Odczynnik emisyjny (świecenie). */
    
public:
    /**
     * @brief Konstruktor dla Szescianu AABB na podstawie koloru i parametrów materiałowych.
     */
    Szescian(Wektor3D kolor, Wektor3D srodek, float polowa_boku, float lustrzanosc = 0.0f, float metalicznosc = 0.0f, float moc_emisji = 0.0f)
        : Obiekt3D(kolor), srodek_(srodek), polowa_boku_(polowa_boku), lustrzanosc_(lustrzanosc), metalicznosc_(metalicznosc), moc_emisji_(moc_emisji) {};

    /**
     * @brief Fabryczna funkcja kreatora statycznego wyodrębniająca atrybuty na podstawie parsy JSON'a.
     */
    static std::unique_ptr<Obiekt3D> kreator(const nlohmann::json &dane);
    bool sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const override;
};

/**
 * @class Trojkat
 * @brief Implementacja płaskiego trójkąta w przestrzeni 3D zdefiniowanego przez 3 wierzchołki.
 */
class Trojkat : public Obiekt3D {
private:
    Wektor3D v0_;           /**< @brief Pierwszy wierzchołek trójkąta. */
    Wektor3D v1_;           /**< @brief Drugi wierzchołek trójkąta. */
    Wektor3D v2_;           /**< @brief Trzeci wierzchołek trójkąta. */
    Wektor3D n0_;           /**< @brief Normalna pierwszego wierzchołka. */
    Wektor3D n1_;           /**< @brief Normalna drugiego wierzchołka. */
    Wektor3D n2_;           /**< @brief Normalna trzeciego wierzchołka. */
    float u_[3];            /**< @brief Współrzędne U dla wierzchołków. */
    float v_tex_[3];        /**< @brief Współrzędne V dla wierzchołków. */
    int has_texture_;       /**< @brief Flaga informująca, czy trójkąt posiada nałożoną teksturę. */
    float lustrzanosc_;     /**< @brief Odczynnik materiału (lustro). */
    float metalicznosc_;    /**< @brief Odczynnik materiału zabarwionego refleksji metalu. */
    float moc_emisji_;      /**< @brief Odczynnik emisyjny (świecenie). */

public:
    /**
     * @brief Konstruktor dla Trojkata na podstawie koloru, wierzchołków, normalnych i parametrów materiałowych.
     */
    Trojkat(Wektor3D kolor, Wektor3D v0, Wektor3D v1, Wektor3D v2, Wektor3D n0, Wektor3D n1, Wektor3D n2, 
            float u0 = 0.0f, float v0_tex = 0.0f, float u1 = 0.0f, float v1_tex = 0.0f, float u2 = 0.0f, float v2_tex = 0.0f, int has_texture = 0,
            float lustrzanosc = 0.0f, float metalicznosc = 0.0f, float moc_emisji = 0.0f)
        : Obiekt3D(kolor), v0_(v0), v1_(v1), v2_(v2), n0_(n0), n1_(n1), n2_(n2), has_texture_(has_texture), lustrzanosc_(lustrzanosc), metalicznosc_(metalicznosc), moc_emisji_(moc_emisji) {
            u_[0] = u0; v_tex_[0] = v0_tex;
            u_[1] = u1; v_tex_[1] = v1_tex;
            u_[2] = u2; v_tex_[2] = v2_tex;
        };

    /**
     * @brief Fabryczna funkcja kreatora statycznego wyodrębniająca atrybuty na podstawie parsy JSON'a.
     */
    static std::unique_ptr<Obiekt3D> kreator(const nlohmann::json &dane);
    bool sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const override;
    
    AABB oblicz_obwiednie() const;
    Wektor3D srodek_geometryczny() const;
    
    Wektor3D get_v0() const { return v0_; }
    Wektor3D get_v1() const { return v1_; }
    Wektor3D get_v2() const { return v2_; }
    Wektor3D get_n0() const { return n0_; }
    Wektor3D get_n1() const { return n1_; }
    Wektor3D get_n2() const { return n2_; }
    float get_u(int i) const { return u_[i]; }
    float get_v(int i) const { return v_tex_[i]; }
    int has_texture() const { return has_texture_; }
    Wektor3D get_kolor() const { return kolor_; }
    float get_lustrzanosc() const { return lustrzanosc_; }
    float get_metalicznosc() const { return metalicznosc_; }
    float get_moc_emisji() const { return moc_emisji_; }
};

/**
 * @struct BVHWezel
 * @brief Węzeł w drzewie Bounding Volume Hierarchy.
 */
struct BVHWezel {
    AABB obwiednia;
    int lewy = -1;
    int prawy = -1;
    int miss_link = -1;
    int pierwszy_trojkat = -1;
    int ilosc_trojkatow = 0;
};

struct Material {
    Wektor3D kolor = Wektor3D(1.0f, 1.0f, 1.0f);
    Wektor3D emisja = Wektor3D(0.0f, 0.0f, 0.0f);
    float ns = 0.0f; // Specular exponent
    float ni = 1.0f; // Optical density
    std::string texture_map = ""; // Diffuse texture map filename
};

class Model3D {
public:
    std::vector<Trojkat> trojkaty_;
    std::vector<BVHWezel> wezly_bvh_;
    
    Model3D(const std::string& sciezka);
    void buduj_bvh();
private:
    void aktualizuj_obwiednie_wezla(int nodeIdx);
    void podzial_wezla(int nodeIdx);
    void build_skip_pointers(int nodeIdx, int next_skip_idx);
};

/**
 * @class SiatkaOBJ
 * @brief Klasa reprezentująca instancję modelu 3D na scenie.
 */
class SiatkaOBJ : public Obiekt3D {
private:
    std::shared_ptr<Model3D> model_;
    Wektor3D pozycja_;
    float skala_;
    float lustrzanosc_;
    float metalicznosc_;
    float moc_emisji_;

public:
    SiatkaOBJ(std::shared_ptr<Model3D> model, Wektor3D kolor, Wektor3D pozycja, float skala, float lustrzanosc = 0.0f, float metalicznosc = 0.0f, float moc_emisji = 0.0f);

    static std::unique_ptr<Obiekt3D> kreator(const nlohmann::json &dane);
    
    bool sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const override;
    
    std::shared_ptr<Model3D> get_model() const { return model_; }
    Wektor3D get_pozycja() const { return pozycja_; }
    float get_skala() const { return skala_; }
    float get_lustrzanosc() const { return lustrzanosc_; }
    float get_metalicznosc() const { return metalicznosc_; }
    float get_moc_emisji() const { return moc_emisji_; }
    AABB oblicz_obwiednie_swiata() const;
};