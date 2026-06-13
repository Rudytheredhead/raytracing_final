#version 130

uniform sampler2D u_tekstura;
uniform vec2 u_resolution;

out vec4 outColor;

void main(){
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    vec2 rozmiar_pixela = 1.0/u_resolution.xy;

    vec4 pixel_bazowy = texture(u_tekstura,st);
    vec3 kolor_ostateczny = pixel_bazowy.rgb;
    float rozmyty_blask = 0.0;
    int promien_rozmycia = 4;
    float ilosc_probek = 0.0;
    for (int x = -promien_rozmycia; x< promien_rozmycia;x++){
        for (int y = -promien_rozmycia; y< promien_rozmycia;y++){
            vec2 offset = vec2(float(x),float(y))*rozmiar_pixela*1.5;

            float probka_blasku = texture(u_tekstura,st + offset).a;
            rozmyty_blask += probka_blasku;
            ilosc_probek +=1.0;
        }

    }
    rozmyty_blask /= ilosc_probek;
    vec3 kolor_swiatla = vec3(1.0, 0.95, 0.8);
    kolor_ostateczny += rozmyty_blask*kolor_swiatla;

    // Reinhard tone mapping
    kolor_ostateczny = kolor_ostateczny / (kolor_ostateczny + vec3(1.0));
    // Gamma correction
    kolor_ostateczny = pow(kolor_ostateczny, vec3(1.0/2.2));

    outColor = vec4(kolor_ostateczny, 1.0);
}