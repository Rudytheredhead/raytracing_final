import math

def generate_colored_knot(filename, num_segments=150, num_sides=20):
    mtl_filename = filename.replace('.obj', '.mtl')
    
    # Parametry wezla (Torus Knot)
    p = 2
    q = 3
    r_tube = 0.5
    r_main = 2.0
    
    vertices = []
    normals = []
    faces = []
    face_materials = [] # index materialu dla kazdej sciany
    
    num_materials = 6
    
    # Funkcja parametryczna wezla
    def knot_point(t):
        r = r_main + math.cos(q * t)
        x = r * math.cos(p * t)
        y = r * math.sin(p * t)
        z = -math.sin(q * t)
        return x, y, z
        
    def normalize(v):
        mag = math.sqrt(sum(c*c for c in v))
        if mag == 0: return (0,0,0)
        return tuple(c/mag for c in v)
        
    def cross(a, b):
        return (a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0])
        
    # Generowanie wierzcholkow
    for i in range(num_segments):
        t = 2 * math.pi * i / num_segments
        
        # Obliczanie wektora stycznego, normalnego i binormalnego
        p1 = knot_point(t)
        p2 = knot_point(t + 0.01)
        T = normalize((p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]))
        
        N = normalize((p1[0]+p2[0], p1[1]+p2[1], p1[2]+p2[2])) 
        B = normalize(cross(T, N))
        N = normalize(cross(B, T))
        
        for j in range(num_sides):
            angle = 2 * math.pi * j / num_sides
            cx = math.cos(angle) * r_tube
            cy = math.sin(angle) * r_tube
            
            # Pozycja na powierzchni tuby
            vx = p1[0] + N[0]*cx + B[0]*cy
            vy = p1[1] + N[1]*cx + B[1]*cy
            vz = p1[2] + N[2]*cx + B[2]*cy
            vertices.append((vx, vy, vz))
            
            # Normalna wierzcholka
            nx = N[0]*cx + B[0]*cy
            ny = N[1]*cx + B[1]*cy
            nz = N[2]*cx + B[2]*cy
            normals.append(normalize((nx, ny, nz)))

    # Generowanie scian (Trojkaty) i przypisywanie materialow
    for i in range(num_segments):
        next_i = (i + 1) % num_segments
        
        # Paski kolorow na wezle
        mat_idx = (i // (num_segments // num_materials)) % num_materials
        
        for j in range(num_sides):
            next_j = (j + 1) % num_sides
            
            v1 = i * num_sides + j
            v2 = next_i * num_sides + j
            v3 = next_i * num_sides + next_j
            v4 = i * num_sides + next_j
            
            # Dwa trojkaty na czworokat (w formacie v//vn)
            faces.append((v1, v2, v3))
            face_materials.append(mat_idx)
            
            faces.append((v1, v3, v4))
            face_materials.append(mat_idx)

    # Zapis do pliku MTL
    with open(mtl_filename, 'w') as f:
        colors = [
            (1.0, 0.1, 0.1, 0),    # Red, dull
            (0.1, 1.0, 0.1, 50),   # Green, shiny
            (0.1, 0.1, 1.0, 100),  # Blue, very shiny
            (1.0, 1.0, 0.1, 0),    # Yellow
            (1.0, 0.1, 1.0, 80),   # Magenta, shiny
            (0.1, 1.0, 1.0, 0)     # Cyan
        ]
        
        for i, (r, g, b, ns) in enumerate(colors):
            f.write(f"newmtl mat_{i}\n")
            f.write(f"Kd {r:.2f} {g:.2f} {b:.2f}\n")
            f.write(f"Ke 0 0 0\n")
            f.write(f"Ns {ns}\n\n")

    # Zapis do pliku OBJ
    with open(filename, 'w') as f:
        f.write(f"mtllib {mtl_filename}\n\n")
        
        for v in vertices:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
            
        for vn in normals:
            f.write(f"vn {vn[0]:.4f} {vn[1]:.4f} {vn[2]:.4f}\n")
            
        current_mat = -1
        for i, face in enumerate(faces):
            if face_materials[i] != current_mat:
                current_mat = face_materials[i]
                f.write(f"\nusemtl mat_{current_mat}\n")
                
            v1, v2, v3 = face
            # Format v//vn
            f.write(f"f {v1+1}//{v1+1} {v2+1}//{v2+1} {v3+1}//{v3+1}\n")

    print(f"Wygenerowano {filename} i {mtl_filename} z {len(faces)} trojkatami!")

if __name__ == "__main__":
    generate_colored_knot("kolorowy_wezel.obj")
