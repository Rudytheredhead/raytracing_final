/**
 * @file Bryly.cpp
 * @brief Implementacje matematycznych rzutowań promieni oraz detekcji kolizji dla konkretnych typów Brył.
 */
#include "Bryly.h"
#include "json.hpp"

#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>


std::unique_ptr<Obiekt3D> Kula::kreator(const nlohmann::json &dane){
    Wektor3D pozycja(dane["pozycja"][0], dane["pozycja"][1], dane["pozycja"][2]);
    Wektor3D kolor(dane["kolor"][0], dane["kolor"][1], dane["kolor"][2]);
    
    
    return std::make_unique<Kula>(
        kolor,
        pozycja,
        dane["promien"],
        dane.value("lustrzanosc", 0.0f),
        dane.value("metalicznosc", 0.0f),
        dane.value("moc_emisji", 0.0f)
    );

}

std::unique_ptr<Obiekt3D> Szescian::kreator(const nlohmann::json &dane){
    Wektor3D pozycja(dane["pozycja"][0], dane["pozycja"][1], dane["pozycja"][2]);
    Wektor3D kolor(dane["kolor"][0], dane["kolor"][1], dane["kolor"][2]);
    
    
    return std::make_unique<Szescian>(
        kolor,
        pozycja,
        dane["polowa_boku"],
        dane.value("lustrzanosc", 0.0f),
        dane.value("metalicznosc", 0.0f),
        dane.value("moc_emisji", 0.0f)
    );

}

bool Kula::sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const {
    Wektor3D OC = promien.poczatek - srodek_;
    float a = promien.kierunek * promien.kierunek;
    float b = 2.0f * (OC * promien.kierunek);
    float c = (OC * OC) - promien_ * promien_;
    float delta = b * b - 4.0f * a * c;
    
    if (delta < 0) return false;

    float delta_pierwiastek = std::sqrt(delta);
    float t = (-b - delta_pierwiastek) / (2.0f * a);
    if (t < t_min || t > t_max) {
        t = (-b + delta_pierwiastek) / (2.0f * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }

    wyniki.t = t;
    wyniki.punkt_zderzenia = promien.poczatek + t * promien.kierunek;
    
    Wektor3D normalna = wyniki.punkt_zderzenia - srodek_;
    normalna.normalizuj();
    
    wyniki.wektor_normalny = normalna;
    wyniki.kolor = kolor_;
    wyniki.trafienie = true;

    
    Wektor3D idealne_odbicie = promien.kierunek.odbij(wyniki.wektor_normalny);
    idealne_odbicie.normalizuj();

    if (idealne_odbicie * wyniki.wektor_normalny > 0.0f) {
        wyniki.promien_odbity.kierunek = idealne_odbicie;
    }
    wyniki.promien_odbity.poczatek = wyniki.punkt_zderzenia+0.001f*wyniki.wektor_normalny;

    wyniki.lustrzanosc = lustrzanosc_;
    wyniki.moc_emisji = moc_emisji_;
    wyniki.metalicznosc = metalicznosc_;
    

    return true;
}


bool Szescian::sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const {
    Wektor3D box_min = srodek_ + (-polowa_boku_);
    Wektor3D box_max = srodek_ + polowa_boku_;

    
    Wektor3D inv_dir(1.0f / promien.kierunek.x(), 1.0f / promien.kierunek.y(), 1.0f / promien.kierunek.z());

    float t1 = (box_min.x() - promien.poczatek.x()) * inv_dir.x();
    float t2 = (box_max.x() - promien.poczatek.x()) * inv_dir.x();
    float tmin = std::min(t1, t2);
    float tmax_val = std::max(t1, t2);

    float t3 = (box_min.y() - promien.poczatek.y()) * inv_dir.y();
    float t4 = (box_max.y() - promien.poczatek.y()) * inv_dir.y();
    tmin = std::max(tmin, std::min(t3, t4));
    tmax_val = std::min(tmax_val, std::max(t3, t4));

    float t5 = (box_min.z() - promien.poczatek.z()) * inv_dir.z();
    float t6 = (box_max.z() - promien.poczatek.z()) * inv_dir.z();
    tmin = std::max(tmin, std::min(t5, t6));
    tmax_val = std::min(tmax_val, std::max(t5, t6));

    
    if (tmax_val < tmin || tmax_val < 0.0f) return false;

    float t = tmin < t_min ? tmax_val : tmin;
    if (t < t_min || t > t_max) return false;

    wyniki.t = t;
    wyniki.punkt_zderzenia = promien.poczatek + t * promien.kierunek;

    
    Wektor3D p = wyniki.punkt_zderzenia - srodek_;
    Wektor3D normalna(0, 0, 0);
   
    float bias = 1.0001f; 
    
    if (std::abs(p.x()) >= polowa_boku_ / bias) normalna = Wektor3D(std::copysign(1.0f, p.x()), 0, 0);
    else if (std::abs(p.y()) >= polowa_boku_ / bias) normalna = Wektor3D(0, std::copysign(1.0f, p.y()), 0);
    else normalna = Wektor3D(0, 0, std::copysign(1.0f, p.z()));

    wyniki.wektor_normalny = normalna;
    wyniki.kolor = kolor_;
    wyniki.trafienie = true;

    
    Wektor3D idealne_odbicie = promien.kierunek.odbij(wyniki.wektor_normalny);
    idealne_odbicie.normalizuj();

    if (idealne_odbicie * wyniki.wektor_normalny > 0.0f) {
        wyniki.promien_odbity.kierunek = idealne_odbicie;
    }
    wyniki.promien_odbity.poczatek = wyniki.punkt_zderzenia + 0.001f * wyniki.wektor_normalny;

    wyniki.lustrzanosc = lustrzanosc_;
    wyniki.moc_emisji = moc_emisji_;
    wyniki.metalicznosc = metalicznosc_;

    return true;
}


std::unique_ptr<Obiekt3D> Trojkat::kreator(const nlohmann::json &dane){
    Wektor3D v0(dane["v0"][0], dane["v0"][1], dane["v0"][2]);
    Wektor3D v1(dane["v1"][0], dane["v1"][1], dane["v1"][2]);
    Wektor3D v2(dane["v2"][0], dane["v2"][1], dane["v2"][2]);
    Wektor3D kolor(dane["kolor"][0], dane["kolor"][1], dane["kolor"][2]);

    Wektor3D edge1 = v1 - v0;
    Wektor3D edge2 = v2 - v0;
    Wektor3D normal = edge1 % edge2;
    normal.normalizuj();

    return std::make_unique<Trojkat>(
        kolor,
        v0,
        v1,
        v2,
        normal,
        normal,
        normal,
        dane.value("lustrzanosc", 0.0f),
        dane.value("metalicznosc", 0.0f),
        dane.value("moc_emisji", 0.0f)
    );
}

bool Trojkat::sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const {
    Wektor3D edge1 = v1_ - v0_;
    Wektor3D edge2 = v2_ - v0_;
    Wektor3D h = promien.kierunek % edge2;
    float a = edge1 * h;
    if (a > -1e-8f && a < 1e-8f)
        return false; // Promień jest równoległy do trójkąta

    float f = 1.0f / a;
    Wektor3D s = promien.poczatek - v0_;
    float u = f * (s * h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Wektor3D q = s % edge1;
    float v = f * (promien.kierunek * q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * (edge2 * q);
    if (t < t_min || t > t_max)
        return false;

    wyniki.t = t;
    wyniki.punkt_zderzenia = promien.poczatek + t * promien.kierunek;
    
    // Interpolacja barycentryczna normalnych
    float w = 1.0f - u - v;
    Wektor3D normalna = w * n0_ + u * n1_ + v * n2_;
    normalna.normalizuj();
    
    // Upewnij się, że wektor normalny jest skierowany przeciwnie do kierunku promienia
    if (promien.kierunek * normalna > 0.0f) {
        normalna = (-1.0f) * normalna;
    }

    wyniki.wektor_normalny = normalna;
    wyniki.kolor = kolor_;
    wyniki.trafienie = true;

    Wektor3D idealne_odbicie = promien.kierunek.odbij(wyniki.wektor_normalny);
    idealne_odbicie.normalizuj();

    if (idealne_odbicie * wyniki.wektor_normalny > 0.0f) {
        wyniki.promien_odbity.kierunek = idealne_odbicie;
    }
    wyniki.promien_odbity.poczatek = wyniki.punkt_zderzenia + 0.001f * wyniki.wektor_normalny;

    wyniki.lustrzanosc = lustrzanosc_;
    wyniki.moc_emisji = moc_emisji_;
    wyniki.metalicznosc = metalicznosc_;

    return true;
}


AABB Trojkat::oblicz_obwiednie() const {
    AABB box(
        Wektor3D(
            std::min({v0_.x(), v1_.x(), v2_.x()}) - 0.001f,
            std::min({v0_.y(), v1_.y(), v2_.y()}) - 0.001f,
            std::min({v0_.z(), v1_.z(), v2_.z()}) - 0.001f
        ),
        Wektor3D(
            std::max({v0_.x(), v1_.x(), v2_.x()}) + 0.001f,
            std::max({v0_.y(), v1_.y(), v2_.y()}) + 0.001f,
            std::max({v0_.z(), v1_.z(), v2_.z()}) + 0.001f
        )
    );
    return box;
}

Wektor3D Trojkat::srodek_geometryczny() const {
    return (v0_ + v1_ + v2_) / 3.0f;
}

Model3D::Model3D(const std::string& sciezka) {
    std::ifstream plik(sciezka);
    if (!plik.is_open()) {
        std::cerr << "Nie udalo sie otworzyc pliku .obj: " << sciezka << std::endl;
        return;
    }

    std::vector<Wektor3D> vertices;
    std::vector<Wektor3D> normals;
    std::map<std::string, Material> materials;
    Material* active_material = nullptr;
    
    std::string line;
    while (std::getline(plik, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string typ;
        iss >> typ;
        
        if (typ == "mtllib") {
            std::string mtl_file;
            iss >> mtl_file;
            
            std::string dir = "";
            size_t slash = sciezka.find_last_of("/\\");
            if (slash != std::string::npos) {
                dir = sciezka.substr(0, slash + 1);
            }
            
            std::ifstream mtl(dir + mtl_file);
            if (mtl.is_open()) {
                std::string mtl_line;
                std::string current_mat = "";
                while (std::getline(mtl, mtl_line)) {
                    if (mtl_line.empty() || mtl_line[0] == '#') continue;
                    std::istringstream miss(mtl_line);
                    std::string mtyp;
                    miss >> mtyp;
                    if (mtyp == "newmtl") {
                        miss >> current_mat;
                        materials[current_mat] = Material();
                    } else if (mtyp == "Kd" && !current_mat.empty()) {
                        float r, g, b; miss >> r >> g >> b;
                        materials[current_mat].kolor = Wektor3D(r, g, b);
                    } else if (mtyp == "Ke" && !current_mat.empty()) {
                        float r, g, b; miss >> r >> g >> b;
                        materials[current_mat].emisja = Wektor3D(r, g, b);
                    } else if (mtyp == "Ns" && !current_mat.empty()) {
                        float ns; miss >> ns;
                        materials[current_mat].ns = ns;
                    } else if (mtyp == "Ni" && !current_mat.empty()) {
                        float ni; miss >> ni;
                        materials[current_mat].ni = ni;
                    }
                }
            } else {
                std::cerr << "Nie udalo sie otworzyc pliku .mtl: " << (dir + mtl_file) << std::endl;
            }
        } else if (typ == "usemtl") {
            std::string mat_name;
            iss >> mat_name;
            if (materials.find(mat_name) != materials.end()) {
                active_material = &materials[mat_name];
            }
        } else if (typ == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Wektor3D(x, y, z));
        } else if (typ == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            Wektor3D norm(x, y, z);
            norm.normalizuj();
            normals.push_back(norm);
        } else if (typ == "f") {
            std::vector<int> face_vertices;
            std::vector<int> face_normals;
            std::string part;
            while (iss >> part) {
                size_t pos1 = part.find('/');
                std::string v_idx_str = part.substr(0, pos1);
                if (v_idx_str.empty()) continue;
                int v_idx = std::stoi(v_idx_str);
                if (v_idx < 0) v_idx = vertices.size() + v_idx + 1;
                face_vertices.push_back(v_idx - 1);

                if (pos1 != std::string::npos) {
                    size_t pos2 = part.find('/', pos1 + 1);
                    if (pos2 != std::string::npos) {
                        std::string n_idx_str = part.substr(pos2 + 1);
                        if (!n_idx_str.empty()) {
                            int n_idx = std::stoi(n_idx_str);
                            if (n_idx < 0) n_idx = normals.size() + n_idx + 1;
                            face_normals.push_back(n_idx - 1);
                        }
                    }
                }
            }
            
            bool has_normals = (face_normals.size() == face_vertices.size());
            for (size_t i = 1; i + 1 < face_vertices.size(); ++i) {
                if (face_vertices[0] < vertices.size() && face_vertices[i] < vertices.size() && face_vertices[i+1] < vertices.size()) {
                    Wektor3D v0 = vertices[face_vertices[0]];
                    Wektor3D v1 = vertices[face_vertices[i]];
                    Wektor3D v2 = vertices[face_vertices[i+1]];

                    Wektor3D n0, n1, n2;
                    if (has_normals && face_normals[0] < normals.size() && face_normals[i] < normals.size() && face_normals[i+1] < normals.size()) {
                        n0 = normals[face_normals[0]];
                        n1 = normals[face_normals[i]];
                        n2 = normals[face_normals[i+1]];
                    } else {
                        Wektor3D edge1 = v1 - v0;
                        Wektor3D edge2 = v2 - v0;
                        Wektor3D flat_norm = edge1 % edge2;
                        flat_norm.normalizuj();
                        n0 = flat_norm;
                        n1 = flat_norm;
                        n2 = flat_norm;
                    }
                    
                    Wektor3D tri_kolor(1.0f, 1.0f, 1.0f);
                    float tri_lustrzanosc = 0.0f;
                    float tri_metalicznosc = 0.0f;
                    float max_emisja = 0.0f;
                    
                    if (active_material) {
                        tri_kolor = active_material->kolor;
                        tri_lustrzanosc = std::min(active_material->ns / 100.0f, 1.0f);
                        max_emisja = std::max({active_material->emisja.x(), active_material->emisja.y(), active_material->emisja.z()});
                    }

                    trojkaty_.emplace_back(
                        tri_kolor, 
                        v0, v1, v2, 
                        n0, n1, n2,
                        tri_lustrzanosc, 
                        tri_metalicznosc, 
                        max_emisja
                    );
                }
            }
        }
    }
    buduj_bvh();
}

void Model3D::buduj_bvh() {
    if (trojkaty_.empty()) return;
    
    BVHWezel root;
    root.pierwszy_trojkat = 0;
    root.ilosc_trojkatow = trojkaty_.size();
    wezly_bvh_.push_back(root);
    
    aktualizuj_obwiednie_wezla(0);
    podzial_wezla(0);
    build_skip_pointers(0, -1);
}

void Model3D::aktualizuj_obwiednie_wezla(int nodeIdx) {
    BVHWezel& wezel = wezly_bvh_[nodeIdx];
    wezel.obwiednia.min_ = Wektor3D(1e9f, 1e9f, 1e9f);
    wezel.obwiednia.max_ = Wektor3D(-1e9f, -1e9f, -1e9f);
    for (int i = 0; i < wezel.ilosc_trojkatow; i++) {
        const Trojkat& t = trojkaty_[wezel.pierwszy_trojkat + i];
        wezel.obwiednia.rozszerz(t.oblicz_obwiednie());
    }
}

void Model3D::podzial_wezla(int nodeIdx) {
    if (wezly_bvh_[nodeIdx].ilosc_trojkatow <= 4) return;
    
    Wektor3D rozmiar = wezly_bvh_[nodeIdx].obwiednia.max_ - wezly_bvh_[nodeIdx].obwiednia.min_;
    int os = 0;
    if (rozmiar.y() > rozmiar.x()) os = 1;
    if (rozmiar.z() > rozmiar[os]) os = 2;
    
    float split_pos = wezly_bvh_[nodeIdx].obwiednia.min_[os] + rozmiar[os] * 0.5f;
    
    int i = wezly_bvh_[nodeIdx].pierwszy_trojkat;
    int j = i + wezly_bvh_[nodeIdx].ilosc_trojkatow - 1;
    while (i <= j) {
        if (trojkaty_[i].srodek_geometryczny()[os] < split_pos) {
            i++;
        } else {
            std::swap(trojkaty_[i], trojkaty_[j--]);
        }
    }
    
    int left_count = i - wezly_bvh_[nodeIdx].pierwszy_trojkat;
    if (left_count == 0 || left_count == wezly_bvh_[nodeIdx].ilosc_trojkatow) {
        left_count = wezly_bvh_[nodeIdx].ilosc_trojkatow / 2;
        i = wezly_bvh_[nodeIdx].pierwszy_trojkat + left_count;
    }
    
    int left_child_idx = wezly_bvh_.size();
    wezly_bvh_.push_back(BVHWezel());
    int right_child_idx = wezly_bvh_.size();
    wezly_bvh_.push_back(BVHWezel());
    
    wezly_bvh_[left_child_idx].pierwszy_trojkat = wezly_bvh_[nodeIdx].pierwszy_trojkat;
    wezly_bvh_[left_child_idx].ilosc_trojkatow = left_count;
    
    wezly_bvh_[right_child_idx].pierwszy_trojkat = i;
    wezly_bvh_[right_child_idx].ilosc_trojkatow = wezly_bvh_[nodeIdx].ilosc_trojkatow - left_count;
    
    wezly_bvh_[nodeIdx].lewy = left_child_idx;
    wezly_bvh_[nodeIdx].prawy = right_child_idx;
    wezly_bvh_[nodeIdx].ilosc_trojkatow = 0; 
    
    aktualizuj_obwiednie_wezla(left_child_idx);
    aktualizuj_obwiednie_wezla(right_child_idx);
    
    podzial_wezla(left_child_idx);
    podzial_wezla(right_child_idx);
}

void Model3D::build_skip_pointers(int nodeIdx, int next_skip_idx) {
    wezly_bvh_[nodeIdx].miss_link = next_skip_idx;
    if (wezly_bvh_[nodeIdx].ilosc_trojkatow == 0) {
        build_skip_pointers(wezly_bvh_[nodeIdx].lewy, wezly_bvh_[nodeIdx].prawy);
        build_skip_pointers(wezly_bvh_[nodeIdx].prawy, next_skip_idx);
    }
}

#include <unordered_map>
std::unordered_map<std::string, std::shared_ptr<Model3D>> model_cache;

std::shared_ptr<Model3D> get_or_load_model(const std::string& sciezka) {
    if (model_cache.find(sciezka) == model_cache.end()) {
        model_cache[sciezka] = std::make_shared<Model3D>(sciezka);
    }
    return model_cache[sciezka];
}

SiatkaOBJ::SiatkaOBJ(std::shared_ptr<Model3D> model, Wektor3D kolor, Wektor3D pozycja, float skala, float lustrzanosc, float metalicznosc, float moc_emisji)
    : Obiekt3D(kolor), model_(model), pozycja_(pozycja), skala_(skala), lustrzanosc_(lustrzanosc), metalicznosc_(metalicznosc), moc_emisji_(moc_emisji) {}

std::unique_ptr<Obiekt3D> SiatkaOBJ::kreator(const nlohmann::json &dane) {
    Wektor3D pozycja(0, 0, 0);
    if (dane.contains("pozycja")) {
        pozycja = Wektor3D(dane["pozycja"][0], dane["pozycja"][1], dane["pozycja"][2]);
    }
    
    Wektor3D kolor(0.8f, 0.8f, 0.8f);
    if (dane.contains("kolor")) {
        kolor = Wektor3D(dane["kolor"][0], dane["kolor"][1], dane["kolor"][2]);
    }
    
    std::string sciezka = dane.value("sciezka", "");
    float skala = dane.value("skala", 1.0f);
    float lustrzanosc = dane.value("lustrzanosc", 0.0f);
    float metalicznosc = dane.value("metalicznosc", 0.0f);
    float moc_emisji = dane.value("moc_emisji", 0.0f);

    auto model = get_or_load_model(sciezka);
    return std::make_unique<SiatkaOBJ>(model, kolor, pozycja, skala, lustrzanosc, metalicznosc, moc_emisji);
}

AABB SiatkaOBJ::oblicz_obwiednie_swiata() const {
    if (model_->wezly_bvh_.empty()) return AABB();
    AABB local = model_->wezly_bvh_[0].obwiednia;
    return AABB(
        local.min_ * skala_ + pozycja_,
        local.max_ * skala_ + pozycja_
    );
}

bool SiatkaOBJ::sprawdz_trafienie(const Promien & promien, WynikZdarzenia& wyniki, float t_min, float t_max) const {
    return false;
}