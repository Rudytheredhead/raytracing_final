import math

def generate_trefoil_knot(filename, N_u=100, N_v=25, r_tube=0.8):
    vertices = []
    normals = []
    faces = []
    
    # 1. Generowanie wierzchołków i normalnych
    for i in range(N_u):
        phi = (i / N_u) * 2 * math.pi
        
        # Współrzędne krzywej bazowej węzła trójlistnego
        x = 2.0 * (math.sin(phi) + 2.0 * math.sin(2.0 * phi))
        y = 2.0 * (math.cos(phi) - 2.0 * math.cos(2.0 * phi))
        z = 2.0 * (-math.sin(3.0 * phi))
        
        # Pochodna (wektor styczny)
        dx = 2.0 * (math.cos(phi) + 4.0 * math.cos(2.0 * phi))
        dy = 2.0 * (-math.sin(phi) + 4.0 * math.sin(2.0 * phi))
        dz = 2.0 * (-3.0 * math.cos(3.0 * phi))
        
        T = [dx, dy, dz]
        len_T = math.sqrt(T[0]**2 + T[1]**2 + T[2]**2)
        T = [T[0]/len_T, T[1]/len_T, T[2]/len_T]
        
        # Wybór wektora pomocniczego do wyznaczenia układu odniesienia
        if abs(T[1]) < 0.9:
            up = [0.0, 1.0, 0.0]
        else:
            up = [0.0, 0.0, 1.0]
            
        # Normalna = up x T
        N = [
            up[1]*T[2] - up[2]*T[1],
            up[2]*T[0] - up[0]*T[2],
            up[0]*T[1] - up[1]*T[0]
        ]
        len_N = math.sqrt(N[0]**2 + N[1]**2 + N[2]**2)
        N = [N[0]/len_N, N[1]/len_N, N[2]/len_N]
        
        # Binormalna = T x N
        B = [
            T[1]*N[2] - T[2]*N[1],
            T[2]*N[0] - T[0]*N[2],
            T[0]*N[1] - T[1]*N[0]
        ]
        
        # Generowanie okręgu wokół punktu na krzywej
        for j in range(N_v):
            theta = (j / N_v) * 2 * math.pi
            
            # Wektor kierunkowy od środka rury do powierzchni (czyli wektor normalny powierzchni)
            nx = math.cos(theta) * N[0] + math.sin(theta) * B[0]
            ny = math.cos(theta) * N[1] + math.sin(theta) * B[1]
            nz = math.cos(theta) * N[2] + math.sin(theta) * B[2]
            
            # Normalizujemy na wszelki wypadek
            len_n = math.sqrt(nx**2 + ny**2 + nz**2)
            nx, ny, nz = nx/len_n, ny/len_n, nz/len_n
            
            # Wierzchołek na rurce
            vx = x + r_tube * nx
            vy = y + r_tube * ny
            vz = z + r_tube * nz
            
            vertices.append((vx, vy, vz))
            normals.append((nx, ny, nz))
            
    # 2. Generowanie ścianek (trójkątów)
    for i in range(N_u):
        next_i = (i + 1) % N_u
        for j in range(N_v):
            next_j = (j + 1) % N_v
            
            # Indeksy wierzchołków (1-based w formacie OBJ)
            v0 = i * N_v + j + 1
            v1 = next_i * N_v + j + 1
            v2 = next_i * N_v + next_j + 1
            v3 = i * N_v + next_j + 1
            
            # Zapisujemy jako 2 trójkąty z indeksami wierzchołków i normalnych (v//vn)
            faces.append((v0, v1, v2))
            faces.append((v0, v2, v3))
            
    # Zapis do pliku .obj
    with open(filename, 'w') as f:
        f.write(f"# Wezel trojlistny (Trefoil Knot) z normalnymi wierzcholkow - {len(faces)} trojkatow\n")
        # Zapisz wierzchołki
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        # Zapisz wektory normalne
        for n in normals:
            f.write(f"vn {n[0]:.6f} {n[1]:.6f} {n[2]:.6f}\n")
        # Zapisz ścianki w formacie v//vn
        for t in faces:
            f.write(f"f {t[0]}//{t[0]} {t[1]}//{t[1]} {t[2]}//{t[2]}\n")

if __name__ == "__main__":
    generate_trefoil_knot("wezel.obj", N_u=100, N_v=25, r_tube=0.8)
    print("Wygenerowano wezel.obj z normalnymi wierzcholkow (5000 trojkatow)")
