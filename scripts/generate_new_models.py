import math

def generate_mobius(filename, R=3.0, w=1.5, segments_u=120, segments_v=24):
    with open(filename, 'w') as f:
        f.write("# Mobius strip\n")
        
        # Wierzcholki
        for i in range(segments_u):
            u = (i / segments_u) * 2 * math.pi
            for j in range(segments_v + 1):
                v = -w/2.0 + (j / segments_v) * w
                
                x = (R + v * math.cos(u/2)) * math.cos(u)
                y = v * math.sin(u/2)
                z = (R + v * math.cos(u/2)) * math.sin(u)
                
                f.write(f"v {x} {y} {z}\n")
                
        # Sciany
        for i in range(segments_u):
            next_i = (i + 1) % segments_u
            for j in range(segments_v):
                if next_i == 0:
                    v0 = i * (segments_v + 1) + j + 1
                    v1 = next_i * (segments_v + 1) + (segments_v - j) + 1
                    v2 = next_i * (segments_v + 1) + (segments_v - (j + 1)) + 1
                    v3 = i * (segments_v + 1) + (j + 1) + 1
                else:
                    v0 = i * (segments_v + 1) + j + 1
                    v1 = next_i * (segments_v + 1) + j + 1
                    v2 = next_i * (segments_v + 1) + (j + 1) + 1
                    v3 = i * (segments_v + 1) + (j + 1) + 1
                
                f.write(f"f {v0} {v1} {v2}\n")
                f.write(f"f {v0} {v2} {v3}\n")

def generate_helix(filename, R=2.0, r=0.5, turns=4.0, height=8.0, segments_t=200, segments_theta=20):
    with open(filename, 'w') as f:
        f.write("mtllib helix.mtl\n")
        f.write("# Helix\n")
        
        # Wierzcholki (v), Tekstury (vt), Normalne (vn)
        for i in range(segments_t):
            t = (i / (segments_t - 1)) * turns * 2 * math.pi
            cx = R * math.cos(t)
            cz = R * math.sin(t)
            cy = (i / (segments_t - 1)) * height - height/2.0
            
            tx = -R * math.sin(t)
            ty = height / (turns * 2 * math.pi)
            tz = R * math.cos(t)
            t_len = math.sqrt(tx*tx + ty*ty + tz*tz)
            tx, ty, tz = tx/t_len, ty/t_len, tz/t_len
            
            nx = -math.cos(t)
            ny = 0.0
            nz = -math.sin(t)
            
            bx = ty * nz - tz * ny
            by = tz * nx - tx * nz
            bz = tx * ny - ty * nx
            
            for j in range(segments_theta + 1):
                theta = (j / segments_theta) * 2 * math.pi
                lx = r * math.cos(theta)
                ly = r * math.sin(theta)
                
                vx = cx + nx * lx + bx * ly
                vy = cy + ny * lx + by * ly
                vz = cz + nz * lx + bz * ly
                
                normal_x = nx * math.cos(theta) + bx * math.sin(theta)
                normal_y = ny * math.cos(theta) + by * math.sin(theta)
                normal_z = nz * math.cos(theta) + bz * math.sin(theta)
                n_len = math.sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)
                normal_x, normal_y, normal_z = normal_x/n_len, normal_y/n_len, normal_z/n_len
                
                uv_u = j / segments_theta
                uv_v = i / (segments_t - 1)
                
                f.write(f"v {vx} {vy} {vz}\n")
                f.write(f"vn {normal_x} {normal_y} {normal_z}\n")
                f.write(f"vt {uv_u} {uv_v}\n")
                
        f.write("usemtl mat0\n")
        # Sciany
        for i in range(segments_t - 1):
            for j in range(segments_theta):
                v0 = i * (segments_theta + 1) + j + 1
                v1 = (i + 1) * (segments_theta + 1) + j + 1
                v2 = (i + 1) * (segments_theta + 1) + j + 2
                v3 = i * (segments_theta + 1) + j + 2
                
                f.write(f"f {v0}/{v0}/{v0} {v1}/{v1}/{v1} {v2}/{v2}/{v2}\n")
                f.write(f"f {v0}/{v0}/{v0} {v2}/{v2}/{v2} {v3}/{v3}/{v3}\n")

if __name__ == "__main__":
    generate_mobius("mobius.obj", R=3.0, w=1.5, segments_u=120, segments_v=24) # Trojkatow: 120 * 24 * 2 = 5760
    generate_helix("helix.obj", R=2.0, r=0.5, turns=4.0, height=8.0, segments_t=200, segments_theta=20) # Trojkatow: 199 * 20 * 2 = 7960
    print("Wygenerowano mobius.obj i helix.obj")
