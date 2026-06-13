import math
import colorsys

def generate_gradient_helix(filename, R=2.0, r=0.5, h=8.0, turns=3, segments_t=200, segments_theta=24):
    mtl_filename = filename.replace('.obj', '.mtl')
    
    # Cyberpunk gradient: od Błękitu do Różu/Fioletu
    def get_color(t):
        hue = 0.5 + t * 0.4
        red, green, blue = colorsys.hls_to_rgb(hue, 0.5, 1.0)
        return (red, green, blue)

    vertices = []
    normals = []
    faces = []
    face_materials = []

    for i in range(segments_t):
        t = (i / (segments_t - 1))
        angle_t = t * turns * 2 * math.pi
        
        # Srodek rurki
        cx = R * math.cos(angle_t)
        cz = R * math.sin(angle_t)
        cy = -h/2 + t * h
        
        # Wektor styczny
        tx = -R * math.sin(angle_t) * turns * 2 * math.pi
        tz = R * math.cos(angle_t) * turns * 2 * math.pi
        ty = h
        
        mag_t = math.sqrt(tx*tx + ty*ty + tz*tz)
        tx, ty, tz = tx/mag_t, ty/mag_t, tz/mag_t
        
        # Normalna glowna sciezki
        nx_path = math.cos(angle_t)
        nz_path = math.sin(angle_t)
        ny_path = 0
        
        # Binormalna
        bx = ty*nz_path - tz*ny_path
        by = tz*nx_path - tx*nz_path
        bz = tx*ny_path - ty*nx_path
        
        mag_b = math.sqrt(bx*bx + by*by + bz*bz)
        bx, by, bz = bx/mag_b, by/mag_b, bz/mag_b
        
        for j in range(segments_theta):
            theta = j * 2 * math.pi / segments_theta
            
            vx = cx + r * (nx_path * math.cos(theta) + bx * math.sin(theta))
            vy = cy + r * (ny_path * math.cos(theta) + by * math.sin(theta))
            vz = cz + r * (nz_path * math.cos(theta) + bz * math.sin(theta))
            vertices.append((vx, vy, vz))
            
            nx = nx_path * math.cos(theta) + bx * math.sin(theta)
            ny = ny_path * math.cos(theta) + by * math.sin(theta)
            nz = nz_path * math.cos(theta) + bz * math.sin(theta)
            
            mag_n = math.sqrt(nx*nx + ny*ny + nz*nz)
            if mag_n == 0:
                normals.append((0, 1, 0))
            else:
                normals.append((nx/mag_n, ny/mag_n, nz/mag_n))

    for i in range(segments_t - 1):
        mat_idx = i
        for j in range(segments_theta):
            next_j = (j + 1) % segments_theta
            
            v1 = i * segments_theta + j
            v2 = (i + 1) * segments_theta + j
            v3 = (i + 1) * segments_theta + next_j
            v4 = i * segments_theta + next_j
            
            faces.append((v1, v2, v3))
            face_materials.append(mat_idx)
            
            faces.append((v1, v3, v4))
            face_materials.append(mat_idx)

    with open(mtl_filename, 'w') as f:
        for i in range(segments_t - 1):
            t = i / (segments_t - 2) if segments_t > 2 else 0
            r_c, g_c, b_c = get_color(t)
            f.write(f"newmtl mat_{i}\n")
            f.write(f"Kd {r_c:.3f} {g_c:.3f} {b_c:.3f}\n")
            f.write(f"Ke 0 0 0\n")
            f.write(f"Ns 90\n")
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
    generate_gradient_helix("gradient_helix.obj")
    print("Wygenerowano gradient_helix.obj z plynym gradientem!")
