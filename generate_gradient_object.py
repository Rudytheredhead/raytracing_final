import math
import colorsys

def generate_gradient_shape(filename, segments_u=150, segments_v=24):
    mtl_filename = filename.replace('.obj', '.mtl')
    
    # Sunset gradient
    def get_color(t):
        # t from 0 to 1
        # Hue: od 0.05 (Pomarancz) do 0.85 (Fioletowo-Niebieski)
        h = 0.05 + t * 0.8
        r, g, b = colorsys.hls_to_rgb(h, 0.5, 1.0)
        return (r, g, b)

    vertices = []
    normals = []
    faces = []
    face_materials = []
    
    # 3D Star-Knot
    p = 3
    q = 5
    r_main = 2.5
    r_tube = 0.6
    
    def knot_point(t):
        r = r_main + math.cos(q * t) * 0.5
        x = r * math.cos(p * t)
        y = r * math.sin(p * t)
        z = -math.sin(q * t) * 1.5
        return x, y, z
        
    def normalize(v):
        mag = math.sqrt(sum(c*c for c in v))
        if mag == 0: return (0,0,0)
        return tuple(c/mag for c in v)
        
    def cross(a, b):
        return (a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0])

    for i in range(segments_u):
        t = 2 * math.pi * i / segments_u
        
        p1 = knot_point(t)
        p2 = knot_point(t + 0.001)
        T = normalize((p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]))
        
        N_temp = (0, 0, 1)
        if abs(T[2]) > 0.9: N_temp = (1, 0, 0)
        B = normalize(cross(T, N_temp))
        N = normalize(cross(B, T))
        
        for j in range(segments_v):
            angle = 2 * math.pi * j / segments_v
            
            # Ripple profile (gwiazdkowy profil)
            r_profile = r_tube * (1.0 + 0.2 * math.cos(4 * angle))
            
            cx = math.cos(angle) * r_profile
            cy = math.sin(angle) * r_profile
            
            vx = p1[0] + N[0]*cx + B[0]*cy
            vy = p1[1] + N[1]*cx + B[1]*cy
            vz = p1[2] + N[2]*cx + B[2]*cy
            vertices.append((vx, vy, vz))
            
            nx = N[0]*math.cos(angle) + B[0]*math.sin(angle)
            ny = N[1]*math.cos(angle) + B[1]*math.sin(angle)
            nz = N[2]*math.cos(angle) + B[2]*math.sin(angle)
            normals.append(normalize((nx, ny, nz)))

    for i in range(segments_u):
        next_i = (i + 1) % segments_u
        mat_idx = i
        
        for j in range(segments_v):
            next_j = (j + 1) % segments_v
            
            v1 = i * segments_v + j
            v2 = next_i * segments_v + j
            v3 = next_i * segments_v + next_j
            v4 = i * segments_v + next_j
            
            faces.append((v1, v2, v3))
            face_materials.append(mat_idx)
            
            faces.append((v1, v3, v4))
            face_materials.append(mat_idx)
            
    with open(mtl_filename, 'w') as f:
        for i in range(segments_u):
            t = i / segments_u
            r, g, b = get_color(t)
            f.write(f"newmtl mat_{i}\n")
            f.write(f"Kd {r:.3f} {g:.3f} {b:.3f}\n")
            f.write(f"Ke 0 0 0\n")
            f.write(f"Ns 90\n") # lsniace
            f.write(f"Ni 1.0\n\n")

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
            f.write(f"f {v1+1}//{v1+1} {v2+1}//{v2+1} {v3+1}//{v3+1}\n")

if __name__ == "__main__":
    generate_gradient_shape("gradient_knot.obj")
    print("Wygenerowano gradient_knot.obj z plynym gradientem!")
