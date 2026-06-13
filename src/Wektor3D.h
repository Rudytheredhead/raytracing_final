/**
 * @file Wektor3D.h
 * @brief Deklaracja i implementacja wektora w przestrzeni trójwymiarowej.
 * @details Zawiera klasę Wektor3D dostarczającą podstawowe operacje algebry liniowej 
 * wymagane do obliczeń geometrii i oświetlenia w raytracingu (iloczyn skalarny, wektorowy, normalizacja, odbicie).
 */
#pragma once
#include <iostream>
#include <cmath> 

/**
 * @class Wektor3D
 * @brief Klasa reprezentująca wektor lub punkt w przestrzeni 3D.
 * @details Klasa implementuje szereg operatorów matematycznych ułatwiających manipulację wektorami 
 * (dodawanie, odejmowanie, mnożenie przez skalar, iloczyn skalarny i wektorowy).
 */
class Wektor3D {
private:
    float x_, y_, z_;
public:
    /**
     * @brief Konstruktor wektora 3D.
     * @param x Składowa x (domyślnie 0).
     * @param y Składowa y (domyślnie 0).
     * @param z Składowa z (domyślnie 0).
     */
    explicit Wektor3D(float x = 0, float y = 0, float z = 0);
    
    /**
     * @brief Operator porównania.
     */
    bool operator == (const Wektor3D& wektor) const;
    
    /**
     * @brief Iloczyn skalarny (dot product) dwóch wektorów.
     */
    float operator * (const Wektor3D& wektor) const; 
    
    /**
     * @brief Iloczyn wektorowy (cross product) dwóch wektorów.
     */
    Wektor3D operator % (const Wektor3D& wektor) const;
    
    /**
     * @brief Dodawanie wektorów.
     */
    Wektor3D operator +(const Wektor3D& wektor) const;
    
    /**
     * @brief Odejmowanie wektorów.
     */
    Wektor3D operator -(const Wektor3D& wektor) const;
    
    /**
     * @brief Mnożenie składowych wektora (Hadamard product).
     */
    Wektor3D przemnoz(const Wektor3D& wektor) const;
    
    /**
     * @brief Dzielenie składowych wektora przez składowe drugiego wektora.
     */
    Wektor3D operator /(const Wektor3D& wektor) const;
    
    /**
     * @brief Dzielenie wektora przez skalar.
     */
    Wektor3D operator /(float scalar) const;
    
    /**
     * @brief Dodanie skalara do każdej ze składowych wektora.
     */
    Wektor3D operator +(float scalar) const;
    
    /**
     * @brief Oblicza wektor odbity względem wektora normalnego.
     * @param normalna Wektor normalny powierzchni.
     * @return Wektor po odbiciu (idealne odbicie lustrzane).
     */
    Wektor3D odbij(const Wektor3D& normalna) const;
    
    /**
     * @brief Oblicza długość (moduł) wektora.
     */
    float modul() const;
    
    /**
     * @brief Oblicza kwadrat długości wektora (szybsze niż modul() gdy nie trzeba pierwiastkować).
     */
    float modul2() const;
    
    /** @brief Zwraca składową x. */
    float x() const;
    /** @brief Zwraca składową y. */
    float y() const;
    /** @brief Zwraca składową z. */
    float z() const;
    
    /** @brief Dostęp do składowych po indeksie (0=x, 1=y, 2=z). */
    float operator[](int i) const;
    
    friend std::ostream& operator << (std::ostream& os, const Wektor3D& wektor);
    friend std::istream& operator >> (std::istream& is, Wektor3D& wektor);
    friend Wektor3D operator * (float scalar, const Wektor3D& wektor);
    
    /**
     * @brief Normalizuje wektor (zmienia jego długość na 1, zachowując kierunek).
     */
    void normalizuj();
    
    /** @brief Ustawia składową x. */
    void set_x(float x){x_ = x;};
    /** @brief Ustawia składową y. */
    void set_y(float y){y_ = y;};
    /** @brief Ustawia składową z. */
    void set_z(float z){z_ = z;};

};

/**
 * @brief Dokonuje liniowej interpolacji (mieszania) pomiędzy dwoma wektorami.
 * @param x Pierwszy wektor.
 * @param y Drugi wektor.
 * @param a Współczynnik mieszania z zakresu [0, 1].
 * @return Wektor wynikowy interpolacji.
 */
Wektor3D mieszaj(const Wektor3D& x, const Wektor3D& y, float a);


inline Wektor3D::Wektor3D(float x, float y, float z) : x_(x), y_(y), z_(z) {}

inline bool Wektor3D::operator==(const Wektor3D& wektor) const {
    return (x_ == wektor.x_) && (y_ == wektor.y_) && (z_ == wektor.z_);
}

inline float Wektor3D::operator*(const Wektor3D& wektor) const {
    return x_ * wektor.x_ + y_ * wektor.y_ + z_ * wektor.z_;
}

inline Wektor3D Wektor3D::operator%(const Wektor3D& wektor) const {
    return Wektor3D(y_ * wektor.z_ - z_ * wektor.y_, z_ * wektor.x_ - x_ * wektor.z_, x_ * wektor.y_ - y_ * wektor.x_);
}

inline Wektor3D Wektor3D::operator+(const Wektor3D& wektor) const {
    return Wektor3D(x_ + wektor.x_, y_ + wektor.y_, z_ + wektor.z_);
}

inline Wektor3D Wektor3D::operator-(const Wektor3D& wektor) const {
    return Wektor3D(x_ - wektor.x_, y_ - wektor.y_, z_ - wektor.z_);
}

inline Wektor3D Wektor3D::przemnoz(const Wektor3D& wektor) const {
    return Wektor3D(x_ * wektor.x_, y_ * wektor.y_, z_ * wektor.z_);
}

inline Wektor3D Wektor3D::operator/(const Wektor3D& wektor) const {
    return Wektor3D(x_ / wektor.x_, y_ / wektor.y_, z_ / wektor.z_);
}

inline Wektor3D Wektor3D::operator/(float scalar) const {
    return Wektor3D(x_ / scalar, y_ / scalar, z_ / scalar);
}

inline Wektor3D Wektor3D::operator+(float scalar) const {
    return Wektor3D(x_ + scalar, y_ + scalar, z_ + scalar);
}

inline Wektor3D Wektor3D::odbij(const Wektor3D& normalna) const {
    float dot_val = (*this) * normalna;
    return (*this) - (2.0f * dot_val) * normalna;
}

inline float Wektor3D::modul() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

inline float Wektor3D::modul2() const {
    return x_ * x_ + y_ * y_ + z_ * z_;
}

inline Wektor3D operator*(float scalar, const Wektor3D& wektor) {
    return Wektor3D(scalar * wektor.x(), scalar * wektor.y(), scalar * wektor.z());
}

inline Wektor3D operator*(const Wektor3D& wektor, float scalar) {
    return Wektor3D(scalar * wektor.x(), scalar * wektor.y(), scalar * wektor.z());
}

inline float Wektor3D::x() const { return x_; }
inline float Wektor3D::y() const { return y_; }
inline float Wektor3D::z() const { return z_; }

inline void Wektor3D::normalizuj(){
    float dl = modul();
    x_ /= dl;
    y_ /= dl;
    z_ /= dl;
}

inline Wektor3D mieszaj(const Wektor3D& x, const Wektor3D& y, float a) {
    return Wektor3D(
        x.x() * (1.0f - a) + y.x() * a,
        x.y() * (1.0f - a) + y.y() * a,
        x.z() * (1.0f - a) + y.z() * a
    );
    
}

inline float Wektor3D::operator[](int i) const {
    if (i == 0) return x_;
    if (i == 1) return y_;
    return z_;
}
