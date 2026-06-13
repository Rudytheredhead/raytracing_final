/**
 * @file Wektor3D.cpp
 * @brief Definicje operatorów strumieniowych dla klasy Wektor3D.
 */
#include "Wektor3D.h"
#include <iostream>




std::ostream& operator << (std::ostream& os, const Wektor3D& wektor) {
    os << "[" << wektor.x_ << ", " << wektor.y_ << ", " << wektor.z_ << "]";
    return os;
}

std::istream& operator>>(std::istream& is, Wektor3D& wektor) {
    is >> wektor.x_;
    is >> wektor.y_;
    is >> wektor.z_;
    return is;
}