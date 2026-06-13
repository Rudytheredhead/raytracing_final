#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "Wczytywanie.h"
#include "Matematyka.h"
#include "Bryly.h"
#include "Wektor3D.h"
#include <vector>
#include <memory>
#include <iostream>
#include <chrono>

using namespace std;

// Float packing utility
void packFloat(float val, sf::Uint8* dest) {
    union { float f; sf::Uint8 b[4]; } u;
    u.f = val;
    dest[0] = u.b[0];
    dest[1] = u.b[1];
    dest[2] = u.b[2];
    dest[3] = u.b[3];
}

// Global BVH builder helpers
void aktualizuj_obwiednie_wezla(int nodeIdx, std::vector<BVHWezel>& wezly, const std::vector<Trojkat>& trojkaty) {
    BVHWezel& wezel = wezly[nodeIdx];
    wezel.obwiednia.min_ = Wektor3D(1e9f, 1e9f, 1e9f);
    wezel.obwiednia.max_ = Wektor3D(-1e9f, -1e9f, -1e9f);
    for (int i = 0; i < wezel.ilosc_trojkatow; i++) {
        wezel.obwiednia.rozszerz(trojkaty[wezel.pierwszy_trojkat + i].oblicz_obwiednie());
    }
}

void podzial_wezla(int nodeIdx, std::vector<BVHWezel>& wezly, std::vector<Trojkat>& trojkaty) {
    BVHWezel& wezel = wezly[nodeIdx];
    if (wezel.ilosc_trojkatow <= 4) return;
    
    Wektor3D rozmiar = wezel.obwiednia.max_ - wezel.obwiednia.min_;
    int os = 0;
    if (rozmiar.y() > rozmiar.x()) os = 1;
    if (rozmiar.z() > rozmiar[os]) os = 2;
    
    float split_pos = wezel.obwiednia.min_[os] + rozmiar[os] * 0.5f;
    
    int i = wezel.pierwszy_trojkat;
    int j = i + wezel.ilosc_trojkatow - 1;
    while (i <= j) {
        if (trojkaty[i].srodek_geometryczny()[os] < split_pos) i++;
        else std::swap(trojkaty[i], trojkaty[j--]);
    }
    
    int left_count = i - wezel.pierwszy_trojkat;
    if (left_count == 0 || left_count == wezel.ilosc_trojkatow) return;
    
    int left_child_idx = wezly.size();
    wezly.push_back(BVHWezel());
    int right_child_idx = wezly.size();
    wezly.push_back(BVHWezel());
    
    wezly[left_child_idx].pierwszy_trojkat = wezly[nodeIdx].pierwszy_trojkat;
    wezly[left_child_idx].ilosc_trojkatow = left_count;
    wezly[right_child_idx].pierwszy_trojkat = i;
    wezly[right_child_idx].ilosc_trojkatow = wezly[nodeIdx].ilosc_trojkatow - left_count;
    
    wezly[nodeIdx].lewy = left_child_idx;
    wezly[nodeIdx].prawy = right_child_idx;
    wezly[nodeIdx].ilosc_trojkatow = 0;
    
    aktualizuj_obwiednie_wezla(left_child_idx, wezly, trojkaty);
    aktualizuj_obwiednie_wezla(right_child_idx, wezly, trojkaty);
    
    podzial_wezla(left_child_idx, wezly, trojkaty);
    podzial_wezla(right_child_idx, wezly, trojkaty);
}

void build_skip_pointers(int nodeIdx, int next_skip_idx, std::vector<BVHWezel>& wezly) {
    wezly[nodeIdx].miss_link = next_skip_idx;
    if (wezly[nodeIdx].ilosc_trojkatow == 0) { // wewnątrz (internal node)
        // Dla lewego dziecka, następnym węzłem po odwiedzeniu/pominięciu jest prawe dziecko
        build_skip_pointers(wezly[nodeIdx].lewy, wezly[nodeIdx].prawy, wezly);
        // Dla prawego dziecka, następnym węzłem jest next_skip_idx ojca
        build_skip_pointers(wezly[nodeIdx].prawy, next_skip_idx, wezly);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(DLUGOSC, DLUGOSC), "GPU Raytracer z Obiektami OBJ");
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    if (!sf::Shader::isAvailable()) {
        cerr << "Shadery nie są wspierane!" << endl;
        return -1;
    }

    sf::Shader shader_obliczenia;
    if (!shader_obliczenia.loadFromFile("fizyka.frag", sf::Shader::Fragment)) {
        cerr << "Nie udalo sie zaladowac shadera fizyka.frag!" << endl;
        return -1;
    }

    sf::Shader shader_post;
    if (!shader_post.loadFromFile("post_process.frag", sf::Shader::Fragment)) {
        cerr << "Nie udalo sie zaladowac shadera post_process.frag!" << endl;
        return -1;
    }

    // Render textures do akumulacji klatek (Ping-Pong) i post-processingu
    sf::RenderTexture accumTex[2];
    if (!accumTex[0].create(DLUGOSC, DLUGOSC) || !accumTex[1].create(DLUGOSC, DLUGOSC)) {
        cerr << "Nie udalo sie utworzyc RenderTexture!" << endl;
        return -1;
    }
    int currentAccum = 0;
    int frameCount = 1;

    FabrykaObiektow::rejestruj(&SiatkaOBJ::kreator, "SiatkaOBJ");
    FabrykaObiektow::rejestruj(&Kula::kreator, "Kula");

    std::vector<std::unique_ptr<Obiekt3D>> obiekty;
    std::vector<Zrodlo_swiatla> zrodla_swiatla;
    int liczba_rdzeni = 1;

    if (!wczytaj_scene("parametry_wejsciowe.json", obiekty, zrodla_swiatla, liczba_rdzeni)) {
        cerr << "Blad wczytywania json!" << endl;
        return -1;
    }

    std::vector<SiatkaOBJ*> instancje_sceny;
    for (auto& ob : obiekty) {
        if (SiatkaOBJ* s = dynamic_cast<SiatkaOBJ*>(ob.get())) {
            instancje_sceny.push_back(s);
        }
    }

    struct InstanceData {
        int id;
        AABB obwiednia;
        Wektor3D srodek() const { return (obwiednia.min_ + obwiednia.max_) * 0.5f; }
    };
    std::vector<InstanceData> inst_data;
    for(int i=0; i<instancje_sceny.size(); ++i) {
        InstanceData idata;
        idata.id = i;
        idata.obwiednia = instancje_sceny[i]->oblicz_obwiednie_swiata();
        inst_data.push_back(idata);
    }

    std::vector<BVHWezel> tlas_wezly;
    auto aktualizuj_obwiednie_tlas = [&](int nodeIdx) {
        tlas_wezly[nodeIdx].obwiednia = AABB();
        for (int i = 0; i < tlas_wezly[nodeIdx].ilosc_trojkatow; i++) {
            tlas_wezly[nodeIdx].obwiednia.rozszerz(inst_data[tlas_wezly[nodeIdx].pierwszy_trojkat + i].obwiednia);
        }
    };

    std::function<void(int)> podzial_tlas = [&](int nodeIdx) {
        if (tlas_wezly[nodeIdx].ilosc_trojkatow <= 1) return;
        
        Wektor3D rozmiar = tlas_wezly[nodeIdx].obwiednia.max_ - tlas_wezly[nodeIdx].obwiednia.min_;
        int os = 0;
        if (rozmiar.y() > rozmiar.x()) os = 1;
        if (rozmiar.z() > rozmiar[os]) os = 2;
        
        float split_pos = tlas_wezly[nodeIdx].obwiednia.min_[os] + rozmiar[os] * 0.5f;
        
        int i = tlas_wezly[nodeIdx].pierwszy_trojkat;
        int j = i + tlas_wezly[nodeIdx].ilosc_trojkatow - 1;
        while (i <= j) {
            if (inst_data[i].srodek()[os] < split_pos) {
                i++;
            } else {
                std::swap(inst_data[i], inst_data[j--]);
            }
        }
        
        int left_count = i - tlas_wezly[nodeIdx].pierwszy_trojkat;
        if (left_count == 0 || left_count == tlas_wezly[nodeIdx].ilosc_trojkatow) return;
        
        int left_child_idx = tlas_wezly.size();
        tlas_wezly.push_back(BVHWezel());
        int right_child_idx = tlas_wezly.size();
        tlas_wezly.push_back(BVHWezel());
        
        tlas_wezly[left_child_idx].pierwszy_trojkat = tlas_wezly[nodeIdx].pierwszy_trojkat;
        tlas_wezly[left_child_idx].ilosc_trojkatow = left_count;
        
        tlas_wezly[right_child_idx].pierwszy_trojkat = i;
        tlas_wezly[right_child_idx].ilosc_trojkatow = tlas_wezly[nodeIdx].ilosc_trojkatow - left_count;
        
        tlas_wezly[nodeIdx].lewy = left_child_idx;
        tlas_wezly[nodeIdx].prawy = right_child_idx;
        tlas_wezly[nodeIdx].ilosc_trojkatow = 0; 
        
        aktualizuj_obwiednie_tlas(left_child_idx);
        aktualizuj_obwiednie_tlas(right_child_idx);
        
        podzial_tlas(left_child_idx);
        podzial_tlas(right_child_idx);
    };

    if(!inst_data.empty()) {
        BVHWezel root;
        root.pierwszy_trojkat = 0;
        root.ilosc_trojkatow = inst_data.size();
        tlas_wezly.push_back(root);
        aktualizuj_obwiednie_tlas(0);
        podzial_tlas(0);
        build_skip_pointers(0, -1, tlas_wezly);
    }

    // Oznaczanie lisci w TLAS: ilosc_trojkatow = -1, pierwszy_trojkat = id instancji
    for (auto& w : tlas_wezly) {
        if (w.ilosc_trojkatow > 0) {
            w.pierwszy_trojkat = inst_data[w.pierwszy_trojkat].id;
            w.ilosc_trojkatow = -1;
        }
    }

    int tlas_float_offset = 0;
    int inst_float_offset = tlas_wezly.size() * 10;
    int blas_float_offset = inst_float_offset + instancje_sceny.size() * 12;

    std::unordered_map<Model3D*, int> model_to_blas_offset;
    int current_blas = blas_float_offset;
    for(auto inst : instancje_sceny) {
        Model3D* m = inst->get_model().get();
        if (model_to_blas_offset.find(m) == model_to_blas_offset.end()) {
            model_to_blas_offset[m] = current_blas;
            current_blas += m->wezly_bvh_.size() * 10;
        }
    }

    int tri_float_offset = current_blas;
    std::unordered_map<Model3D*, int> model_to_tri_offset;
    int current_tri = tri_float_offset;
    for(auto inst : instancje_sceny) {
        Model3D* m = inst->get_model().get();
        if (model_to_tri_offset.find(m) == model_to_tri_offset.end()) {
            model_to_tri_offset[m] = current_tri;
            current_tri += m->trojkaty_.size() * 24;
        }
    }

    int floats_needed = current_tri;
    int tex_size = std::ceil(std::sqrt(floats_needed));
    if (tex_size < 1) tex_size = 1;

    sf::Image bvh_image;
    bvh_image.create(tex_size, tex_size, sf::Color::Black);

    int current_float_index = 0;
    auto push_float = [&](float val) {
        int x = current_float_index % tex_size;
        int y = current_float_index / tex_size;
        sf::Color c;
        packFloat(val, (sf::Uint8*)&c);
        bvh_image.setPixel(x, y, c);
        current_float_index++;
    };

    // 1. Pack TLAS
    for(const auto& w : tlas_wezly) {
        push_float(w.obwiednia.min_.x()); push_float(w.obwiednia.min_.y()); push_float(w.obwiednia.min_.z());
        push_float((float)w.lewy);
        push_float(w.obwiednia.max_.x()); push_float(w.obwiednia.max_.y()); push_float(w.obwiednia.max_.z());
        push_float((float)w.miss_link);
        push_float((float)w.pierwszy_trojkat); push_float((float)w.ilosc_trojkatow);
    }

    // 2. Pack Instances
    for(auto inst : instancje_sceny) {
        Model3D* m = inst->get_model().get();
        Wektor3D pos = inst->get_pozycja();
        Wektor3D col = inst->get_kolor();
        push_float(pos.x()); push_float(pos.y()); push_float(pos.z());
        push_float(inst->get_skala());
        push_float(col.x()); push_float(col.y()); push_float(col.z());
        push_float((float)model_to_blas_offset[m]);
        push_float(inst->get_lustrzanosc());
        push_float(inst->get_metalicznosc());
        push_float(inst->get_moc_emisji());
        push_float((float)model_to_tri_offset[m]);
    }

    // 3. Pack BLAS
    std::unordered_map<Model3D*, bool> packed_blas;
    for(auto inst : instancje_sceny) {
        Model3D* m = inst->get_model().get();
        if (!packed_blas[m]) {
            packed_blas[m] = true;
            for(const auto& w : m->wezly_bvh_) {
                push_float(w.obwiednia.min_.x()); push_float(w.obwiednia.min_.y()); push_float(w.obwiednia.min_.z());
                push_float((float)w.lewy);
                push_float(w.obwiednia.max_.x()); push_float(w.obwiednia.max_.y()); push_float(w.obwiednia.max_.z());
                push_float((float)w.miss_link);
                push_float((float)w.pierwszy_trojkat); push_float((float)w.ilosc_trojkatow);
            }
        }
    }

    // 4. Pack Triangles
    std::unordered_map<Model3D*, bool> packed_tri;
    for(auto inst : instancje_sceny) {
        Model3D* m = inst->get_model().get();
        if (!packed_tri[m]) {
            packed_tri[m] = true;
            for(const auto& t : m->trojkaty_) {
                push_float(t.get_v0().x()); push_float(t.get_v0().y()); push_float(t.get_v0().z()); push_float(t.get_lustrzanosc());
                push_float(t.get_v1().x()); push_float(t.get_v1().y()); push_float(t.get_v1().z()); push_float(t.get_metalicznosc());
                push_float(t.get_v2().x()); push_float(t.get_v2().y()); push_float(t.get_v2().z()); push_float(t.get_moc_emisji());
                push_float(t.get_n0().x()); push_float(t.get_n0().y()); push_float(t.get_n0().z()); push_float(t.get_kolor().x());
                push_float(t.get_n1().x()); push_float(t.get_n1().y()); push_float(t.get_n1().z()); push_float(t.get_kolor().y());
                push_float(t.get_n2().x()); push_float(t.get_n2().y()); push_float(t.get_n2().z()); push_float(t.get_kolor().z());
            }
        }
    }

    sf::Texture bvh_texture;
    bvh_texture.loadFromImage(bvh_image);
    bvh_texture.setSmooth(false);

    Wektor3D kamera(0.0f, 0.0f, 0.0f);
    Wektor3D gora(0.0f, 1.0f, 0.0f);
    float odleglosc_od_ekranu = 1.0f;
    float sens_myszki = 0.003f;
    float obrot_na_boki = 0.0f;
    float obrot_gora_dol = 0.0f;

    sf::Mouse::setPosition(sf::Vector2i(DLUGOSC/2, DLUGOSC/2), window);

    sf::RectangleShape fullscreen_rect(sf::Vector2f(DLUGOSC, DLUGOSC));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                sf::Texture screen_texture;
                screen_texture.create(window.getSize().x, window.getSize().y);
                screen_texture.update(window);
                sf::Image screenshot = screen_texture.copyToImage();
                if (screenshot.saveToFile("render.png")) {
                    std::cout << "Zapisano render do pliku render.png" << std::endl;
                }
            }
        }

        bool kamera_poruszona = false;
        // --- Mysz: obracanie kamery ---
        if (window.hasFocus()){
            sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
            int dx = mouse_pos.x - DLUGOSC/2;
            int dy = mouse_pos.y - DLUGOSC/2;
            if (dx != 0 || dy != 0) {
                sf::Mouse::setPosition(sf::Vector2i(DLUGOSC/2, DLUGOSC/2), window);
                obrot_na_boki -= dx * sens_myszki;
                obrot_gora_dol = std::clamp(obrot_gora_dol - dy * sens_myszki, -1.55f, 1.55f);
                kamera_poruszona = true;
            }
        }

        if (kamera_poruszona) {
            frameCount = 1;
        }

        // --- Wektor kierunku patrzenia (z uwzglednieniem pitch) ---
        Wektor3D kierunek_kamery(
            -std::cos(obrot_gora_dol)*std::sin(obrot_na_boki),
            std::sin(obrot_gora_dol),
            -std::cos(obrot_gora_dol)*std::cos(obrot_na_boki)
        );
        kierunek_kamery.normalizuj();

        // --- Uklad wspolrzednych kamery (recznie, W = kierunek patrzenia) ---
        Wektor3D W = kierunek_kamery;
        Wektor3D U = W % gora;
        U.normalizuj();
        Wektor3D V = U % W;
        V.normalizuj();

        shader_obliczenia.setUniform("u_resolution", sf::Vector2f(window.getSize()));
        shader_obliczenia.setUniform("kamera_pos", sf::Vector3f(kamera.x(), kamera.y(), kamera.z()));
        shader_obliczenia.setUniform("odleglosc_od_ekranu", odleglosc_od_ekranu);
        shader_obliczenia.setUniform("W", sf::Vector3f(W.x(), W.y(), W.z()));
        shader_obliczenia.setUniform("U", sf::Vector3f(U.x(), U.y(), U.z()));
        shader_obliczenia.setUniform("V", sf::Vector3f(V.x(), V.y(), V.z()));
        
        shader_obliczenia.setUniform("ilosc_swiatel", (int)zrodla_swiatla.size());
        for(size_t i = 0; i < zrodla_swiatla.size(); i++){
            std::string prefix = "swiatla["+std::to_string(i)+"].";
            shader_obliczenia.setUniform(prefix+"srodek", sf::Vector3f(zrodla_swiatla[i].srodek.x(), zrodla_swiatla[i].srodek.y(), zrodla_swiatla[i].srodek.z()));
            shader_obliczenia.setUniform(prefix+"kolor", sf::Vector3f(zrodla_swiatla[i].kolor.x(), zrodla_swiatla[i].kolor.y(), zrodla_swiatla[i].kolor.z()));
            shader_obliczenia.setUniform(prefix+"moc_emisji", zrodla_swiatla[i].moc_emisji);
        }

        shader_obliczenia.setUniform("bvh_data", bvh_texture);
        shader_obliczenia.setUniform("tex_size", tex_size);
        shader_obliczenia.setUniform("inst_array_offset", inst_float_offset);
        
        shader_obliczenia.setUniform("u_prev_frame", accumTex[1 - currentAccum].getTexture());
        shader_obliczenia.setUniform("u_frame_count", (float)frameCount);

        // --- PRZEBIEG 1: Renderowanie sceny do RenderTexture (akumulacja) ---
        sf::RenderStates states_pass1;
        states_pass1.shader = &shader_obliczenia;
        states_pass1.blendMode = sf::BlendNone;

        accumTex[currentAccum].clear(sf::Color::Black);
        accumTex[currentAccum].draw(fullscreen_rect, states_pass1);
        accumTex[currentAccum].display();

        // --- PRZEBIEG 2: Post-processing (bloom/flara) na ekran ---
        shader_post.setUniform("u_tekstura", accumTex[currentAccum].getTexture());
        shader_post.setUniform("u_resolution", sf::Vector2f(window.getSize()));

        sf::RenderStates states_pass2;
        states_pass2.shader = &shader_post;
        states_pass2.blendMode = sf::BlendNone;

        sf::RectangleShape post_rect(sf::Vector2f(DLUGOSC, DLUGOSC));

        window.clear(sf::Color::Black);
        window.draw(post_rect, states_pass2);
        window.display();

        currentAccum = 1 - currentAccum;
        frameCount++;
    }

    return 0;
}