import math

def generate_torus(filename, R, r, segments_u, segments_v):
    with open(filename, 'w') as f:
        # Generowanie wierzchołków
        for i in range(segments_u):
            u = (i / segments_u) * 2 * math.pi
            for j in range(segments_v):
                v = (j / segments_v) * 2 * math.pi
                
                x = (R + r * math.cos(v)) * math.cos(u)
                y = r * math.sin(v)
                z = (R + r * math.cos(v)) * math.sin(u)
                f.write(f"v {x} {y} {z}\n")
                
        # Generowanie ścianek (czworokątów -> trójkątów)
        for i in range(segments_u):
            for j in range(segments_v):
                next_i = (i + 1) % segments_u
                next_j = (j + 1) % segments_v
                
                # Indeksy w OBJ są od 1
                v0 = i * segments_v + j + 1
                v1 = next_i * segments_v + j + 1
                v2 = next_i * segments_v + next_j + 1
                v3 = i * segments_v + next_j + 1
                
                f.write(f"f {v0} {v1} {v2}\n")
                f.write(f"f {v0} {v2} {v3}\n")

def generate_grid(filename, size, segments):
    with open(filename, 'w') as f:
        half = size / 2.0
        step = size / segments
        
        for i in range(segments + 1):
            for j in range(segments + 1):
                x = -half + i * step
                z = -half + j * step
                # Wprowadzenie lekkiego falowania (noise)
                y = math.sin(x * 0.5) * math.cos(z * 0.5) * 0.5
                f.write(f"v {x} {y} {z}\n")
                
        for i in range(segments):
            for j in range(segments):
                v0 = i * (segments + 1) + j + 1
                v1 = (i + 1) * (segments + 1) + j + 1
                v2 = (i + 1) * (segments + 1) + (j + 1) + 1
                v3 = i * (segments + 1) + (j + 1) + 1
                
                f.write(f"f {v0} {v1} {v2}\n")
                f.write(f"f {v0} {v2} {v3}\n")

if __name__ == "__main__":
    generate_torus("torus.obj", R=3.0, r=0.8, segments_u=24, segments_v=12)
    generate_grid("teren.obj", size=30.0, segments=20)
    print("Wygenerowano modele torus.obj i teren.obj")
