#version 330 core

uniform vec2 u_resolution;
uniform vec3 W;
uniform vec3 U;
uniform vec3 V;
uniform vec3 kamera_pos;
uniform float odleglosc_od_ekranu;

uniform vec3 skyColor = vec3(0.01, 0.01, 0.02);

struct Zrodlo_swiatla {
    vec3 srodek;
    vec3 kolor;
    float moc_emisji;
    vec3 kierunek_swiecenia;
    float kat_swiecenia;
};
uniform Zrodlo_swiatla swiatla[10];
uniform int ilosc_swiatel;

uniform sampler2D bvh_data;
uniform int tex_size;
uniform int inst_array_offset;

uniform sampler2D u_prev_frame;
uniform float u_frame_count;

float rand(inout float seed) {
    seed = fract(sin(seed * 78.233) * 43758.5453);
    return seed;
}

vec3 random_cosine_direction(vec3 normal, inout float seed) {
    float r1 = rand(seed);
    float r2 = rand(seed);
    float phi = 2.0 * 3.14159265 * r1;
    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);
    float z = sqrt(1.0 - r2);
    
    vec3 w = normal;
    vec3 a = abs(w.x) > 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 v = normalize(cross(w, a));
    vec3 u = cross(w, v);
    
    return normalize(x * u + y * v + z * w);
}

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitRecord {
    bool trafienie;
    float t;
    vec3 point;
    vec3 normal;
    vec3 kolor;
    float lustrzanosc;
    float metalicznosc;
    float moc_emisji;
};

float unpackFloat(vec4 color) {
    uvec4 bytes = uvec4(color * 255.0 + 0.5);
    uint packed_val = (bytes.a << 24u) | (bytes.b << 16u) | (bytes.g << 8u) | bytes.r;
    return uintBitsToFloat(packed_val);
}

float fetchFloat(int floatIndex) {
    int x = floatIndex % tex_size;
    int y = floatIndex / tex_size;
    vec4 p = texelFetch(bvh_data, ivec2(x, y), 0);
    return unpackFloat(p);
}

bool slabIntersection(vec3 min_box, vec3 max_box, Ray ray, float t_min, float t_max) {
    vec3 invD = 1.0 / ray.direction;
    vec3 t0 = (min_box - ray.origin) * invD;
    vec3 t1 = (max_box - ray.origin) * invD;
    
    vec3 tSmaller = min(t0, t1);
    vec3 tBigger = max(t0, t1);
    
    float tmin = max(t_min, max(tSmaller.x, max(tSmaller.y, tSmaller.z)));
    float tmax = min(t_max, min(tBigger.x, min(tBigger.y, tBigger.z)));
    
    return tmax > tmin;
}

bool rayTriangleIntersect(Ray ray, vec3 v0, vec3 v1, vec3 v2, vec3 n0, vec3 n1, vec3 n2, out float t, out vec3 normal) {
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 h = cross(ray.direction, e2);
    float a = dot(e1, h);
    if (a > -0.00001 && a < 0.00001) return false;
    
    float f = 1.0 / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;
    
    vec3 q = cross(s, e1);
    float v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0) return false;
    
    float t_hit = f * dot(e2, q);
    if (t_hit > 0.001) {
        t = t_hit;
        
        // Interpolacja normalnych wierzcholkow
        float w = 1.0 - u - v;
        normal = normalize(w * n0 + u * n1 + v * n2);
        
        // Upewnij sie ze normalny jest skierowany przeciwnie do kierunku promienia
        if (dot(ray.direction, normal) > 0.0) {
            normal = -normal;
        }
        return true;
    }
    return false;
}

HitRecord traverseBVH(Ray ray, float t_max) {
    HitRecord closest;
    closest.trafienie = false;
    closest.t = t_max;

    int tlasIdx = 0; 
    while (tlasIdx != -1) {
        int baseOffset = tlasIdx * 10;
        
        vec3 min_box = vec3(fetchFloat(baseOffset), fetchFloat(baseOffset+1), fetchFloat(baseOffset+2));
        int lewy = int(fetchFloat(baseOffset+3));
        vec3 max_box = vec3(fetchFloat(baseOffset+4), fetchFloat(baseOffset+5), fetchFloat(baseOffset+6));
        int miss_link = int(fetchFloat(baseOffset+7));
        int pierwszy_trojkat = int(fetchFloat(baseOffset+8)); 
        int ilosc_trojkatow = int(fetchFloat(baseOffset+9));

        if (!slabIntersection(min_box, max_box, ray, 0.001, closest.t)) {
            tlasIdx = miss_link;
            continue;
        }

        if (ilosc_trojkatow == -1) { 
            int inst_idx = pierwszy_trojkat;
            int instOffset = inst_array_offset + inst_idx * 12;
            
            vec3 inst_pos = vec3(fetchFloat(instOffset), fetchFloat(instOffset+1), fetchFloat(instOffset+2));
            float inst_skala = fetchFloat(instOffset+3);
            vec3 inst_kolor = vec3(fetchFloat(instOffset+4), fetchFloat(instOffset+5), fetchFloat(instOffset+6));
            int blas_root = int(fetchFloat(instOffset+7));
            float inst_lustr = fetchFloat(instOffset+8);
            float inst_metal = fetchFloat(instOffset+9);
            float inst_emis = fetchFloat(instOffset+10);
            int tri_offset = int(fetchFloat(instOffset+11));

            Ray local_ray;
            local_ray.origin = (ray.origin - inst_pos) / inst_skala;
            local_ray.direction = ray.direction / inst_skala; 

            int blasIdx = 0;
            while (blasIdx != -1) {
                int blasOffset = blas_root + blasIdx * 10;
                
                vec3 b_min = vec3(fetchFloat(blasOffset), fetchFloat(blasOffset+1), fetchFloat(blasOffset+2));
                int b_lewy = int(fetchFloat(blasOffset+3));
                vec3 b_max = vec3(fetchFloat(blasOffset+4), fetchFloat(blasOffset+5), fetchFloat(blasOffset+6));
                int b_miss = int(fetchFloat(blasOffset+7));
                int b_pierwszy = int(fetchFloat(blasOffset+8));
                int b_ilosc = int(fetchFloat(blasOffset+9));

                if (!slabIntersection(b_min, b_max, local_ray, 0.001, closest.t)) { 
                    blasIdx = b_miss;
                    continue;
                }

                if (b_ilosc > 0) { 
                    for (int i = 0; i < b_ilosc; i++) {
                        int triBase = tri_offset + (b_pierwszy + i) * 24;
                        vec3 v0 = vec3(fetchFloat(triBase), fetchFloat(triBase+1), fetchFloat(triBase+2));
                        float tri_lustr = fetchFloat(triBase+3);
                        vec3 v1 = vec3(fetchFloat(triBase+4), fetchFloat(triBase+5), fetchFloat(triBase+6));
                        float tri_metal = fetchFloat(triBase+7);
                        vec3 v2 = vec3(fetchFloat(triBase+8), fetchFloat(triBase+9), fetchFloat(triBase+10));
                        float tri_emis = fetchFloat(triBase+11);
                        
                        vec3 n0 = vec3(fetchFloat(triBase+12), fetchFloat(triBase+13), fetchFloat(triBase+14));
                        float tri_col_x = fetchFloat(triBase+15);
                        vec3 n1 = vec3(fetchFloat(triBase+16), fetchFloat(triBase+17), fetchFloat(triBase+18));
                        float tri_col_y = fetchFloat(triBase+19);
                        vec3 n2 = vec3(fetchFloat(triBase+20), fetchFloat(triBase+21), fetchFloat(triBase+22));
                        float tri_col_z = fetchFloat(triBase+23);
                        
                        float t_hit;
                        vec3 normal;
                        if (rayTriangleIntersect(local_ray, v0, v1, v2, n0, n1, n2, t_hit, normal)) {
                            if (t_hit < closest.t) { 
                                closest.trafienie = true;
                                closest.t = t_hit;
                                closest.point = ray.origin + ray.direction * t_hit; 
                                closest.normal = normal; 
                                closest.kolor = inst_kolor * vec3(tri_col_x, tri_col_y, tri_col_z);
                                closest.lustrzanosc = clamp(inst_lustr + tri_lustr, 0.0, 1.0);
                                closest.metalicznosc = clamp(inst_metal + tri_metal, 0.0, 1.0);
                                closest.moc_emisji = inst_emis + tri_emis;
                            }
                        }
                    }
                    blasIdx = b_miss;
                } else {
                    blasIdx = b_lewy;
                }
            }
            tlasIdx = miss_link;
        } else {
            tlasIdx = lewy;
        }
    }
    return closest;
}


vec3 calculateLighting(HitRecord hit, vec3 viewDir) {
    vec3 result = vec3(0.0);
    vec3 ambient = vec3(0.1) * hit.kolor;
    result += ambient;
    
    for (int i = 0; i < ilosc_swiatel; i++) {
        vec3 lightDir = normalize(swiatla[i].srodek - hit.point);
        float distance = length(swiatla[i].srodek - hit.point);
        float diff = max(dot(hit.normal, lightDir), 0.0);
        
        // Shadow ray
        Ray shadowRay = Ray(hit.point + hit.normal * 0.001, lightDir);
        HitRecord shadowHit = traverseBVH(shadowRay, distance);
        
        if (!shadowHit.trafienie) {
            vec3 diffuse = diff * hit.kolor * swiatla[i].kolor * (swiatla[i].moc_emisji / (distance * distance));
            result += diffuse;
        }
    }
    
    result += hit.kolor * hit.moc_emisji;
    return result;
}

void main() {
    float seed = dot(gl_FragCoord.xy, vec2(12.9898, 78.233)) + u_frame_count * 112.123;
    vec2 jitter = vec2(rand(seed), rand(seed)) - 0.5;
    
    vec2 fragCoord = gl_FragCoord.xy + jitter;
    vec2 uv = (fragCoord - 0.5 * u_resolution) / u_resolution.y;
    
    vec3 direction = normalize(odleglosc_od_ekranu * W + uv.x * U + uv.y * V);
    Ray ray = Ray(kamera_pos, direction);
    
    vec3 final_color = vec3(0.0);
    vec3 attenuation = vec3(1.0);
    float maska_blasku = 0.0;
    
    for (int bounce = 0; bounce < 3; bounce++) {
        HitRecord hit = traverseBVH(ray, 9999.0);
        
        if (!hit.trafienie) {
            final_color += attenuation * skyColor;
            break;
        }
        
        vec3 local_lighting = calculateLighting(hit, -ray.direction);
        final_color += attenuation * local_lighting * (1.0 - hit.lustrzanosc);
        
        if (bounce == 0 && hit.moc_emisji > 0.0) {
            maska_blasku += hit.moc_emisji * 0.3;
        }
        
        if (hit.lustrzanosc > 0.0) {
            attenuation *= hit.lustrzanosc * (hit.metalicznosc > 0.0 ? hit.kolor : vec3(1.0));
            ray.origin = hit.point + hit.normal * 0.001;
            
            vec3 ideal_reflect = reflect(ray.direction, hit.normal);
            float roughness = 1.0 - hit.lustrzanosc;
            vec3 rand_dir = normalize(vec3(rand(seed)-0.5, rand(seed)-0.5, rand(seed)-0.5));
            ray.direction = normalize(mix(ideal_reflect, ideal_reflect + rand_dir * roughness, roughness));
        } else {
            vec3 scatter_dir = random_cosine_direction(hit.normal, seed);
            attenuation *= hit.kolor;
            ray.origin = hit.point + hit.normal * 0.001;
            ray.direction = scatter_dir;
        }
    }
    
    for (int i = 0; i < ilosc_swiatel; i++) {
        vec3 do_swiatla = swiatla[i].srodek - kamera_pos;
        float odleglosc = length(do_swiatla);
        vec3 kierunek_swiatla = do_swiatla / odleglosc;
        
        float cos_kat = dot(direction, kierunek_swiatla);
        
        if (cos_kat > 0.95) {
            Ray test_ray = Ray(kamera_pos, kierunek_swiatla);
            HitRecord blokada = traverseBVH(test_ray, odleglosc - 0.01);
            if (blokada.trafienie) continue;
        }
        
        float tlumienie = swiatla[i].moc_emisji / (odleglosc * odleglosc);
        
        if (cos_kat > 0.9995) {
            float jadro = smoothstep(0.9995, 1.0, cos_kat);
            vec3 kolor_jadra = swiatla[i].kolor * jadro * tlumienie * 15.0;
            final_color += kolor_jadra;
            maska_blasku += jadro * tlumienie * 8.0;
        }
        
        if (cos_kat > 0.995) {
            float halo = smoothstep(0.995, 0.999, cos_kat);
            halo = halo * halo;
            vec3 kolor_halo = swiatla[i].kolor * halo * tlumienie * 3.0;
            final_color += kolor_halo;
            maska_blasku += halo * tlumienie * 2.0;
        }
        
        if (cos_kat > 0.98) {
            float poswiatla = smoothstep(0.98, 0.997, cos_kat);
            vec3 kolor_poswiatla = swiatla[i].kolor * poswiatla * tlumienie * 0.5;
            final_color += kolor_poswiatla;
            maska_blasku += poswiatla * tlumienie * 0.5;
        }
    }
    
    maska_blasku = clamp(maska_blasku, 0.0, 1.0);
    
    vec2 texCoord = gl_FragCoord.xy / u_resolution;
    vec4 prev_frame = texture(u_prev_frame, texCoord);
    
    if (u_frame_count > 1.0) {
        final_color = mix(prev_frame.rgb, final_color, 1.0 / u_frame_count);
        maska_blasku = mix(prev_frame.a, maska_blasku, 1.0 / u_frame_count);
    }
    
    gl_FragColor = vec4(final_color, maska_blasku);
}