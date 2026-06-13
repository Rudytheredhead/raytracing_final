#include "Bryly.h"
#include <iostream>

int max_depth = 0;
int leaf_count = 0;
int max_leaf_triangles = 0;

void analyze_bvh(Model3D* m, int nodeIdx, int depth) {
    if (depth > max_depth) max_depth = depth;
    if (m->wezly_bvh_[nodeIdx].ilosc_trojkatow > 0) {
        leaf_count++;
        if (m->wezly_bvh_[nodeIdx].ilosc_trojkatow > max_leaf_triangles) {
            max_leaf_triangles = m->wezly_bvh_[nodeIdx].ilosc_trojkatow;
        }
        if (m->wezly_bvh_[nodeIdx].ilosc_trojkatow > 100) {
            std::cout << "Duzy lisc na glebokosci " << depth << " z trojkatami: " << m->wezly_bvh_[nodeIdx].ilosc_trojkatow << std::endl;
        }
    } else {
        analyze_bvh(m, m->wezly_bvh_[nodeIdx].lewy, depth + 1);
        analyze_bvh(m, m->wezly_bvh_[nodeIdx].prawy, depth + 1);
    }
}

int main() {
    std::cout << "Wczytywanie modelu..." << std::endl;
    Model3D m("plastic_crate_01_1k.obj");
    std::cout << "Liczba trojkatow: " << m.trojkaty_.size() << std::endl;
    std::cout << "Liczba wezlow BVH: " << m.wezly_bvh_.size() << std::endl;
    if (!m.wezly_bvh_.empty()) {
        analyze_bvh(&m, 0, 1);
        std::cout << "Max depth: " << max_depth << std::endl;
        std::cout << "Leaves: " << leaf_count << std::endl;
        std::cout << "Max triangles in leaf: " << max_leaf_triangles << std::endl;
    }
    return 0;
}
