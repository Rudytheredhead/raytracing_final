import json
import sys

def convert_obj_to_json(obj_path, json_path, scale=1.0, offset=(0.0, 0.0, 0.0), color=(0.8, 0.8, 0.8), lustrzanosc=0.0, metalicznosc=0.0, moc_emisji=0.0):
    vertices = []
    triangles = []
    
    with open(obj_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split()
            if parts[0] == 'v':
                # Vertex: v x y z
                x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                # Skalowanie i przesunięcie
                x = x * scale + offset[0]
                y = y * scale + offset[1]
                z = z * scale + offset[2]
                vertices.append([x, y, z])
            elif parts[0] == 'f':
                # Face: f v1/vt1/vn1 v2/vt2/vn2 ...
                face_vertices = []
                for part in parts[1:]:
                    idx = int(part.split('/')[0])
                    if idx < 0:
                        idx = len(vertices) + idx + 1
                    face_vertices.append(idx - 1)
                
                # Triangulacja wielokątów (np. czworokątów) na trójkąty
                for i in range(1, len(face_vertices) - 1):
                    v0 = vertices[face_vertices[0]]
                    v1 = vertices[face_vertices[i]]
                    v2 = vertices[face_vertices[i+1]]
                    triangles.append((v0, v1, v2))
                    
    # Generowanie obiektów JSON
    objects = []
    for t in triangles:
        objects.append({
            "typ": "Trojkat",
            "kolor": color,
            "v0": t[0],
            "v1": t[1],
            "v2": t[2],
            "lustrzanosc": lustrzanosc,
            "moc_emisji": moc_emisji,
            "metalicznosc": metalicznosc
        })
        
    # Odczytaj istniejący plik JSON, aby zachować ustawienia rdzeni i światła
    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except Exception:
        data = {
            "ustawienia": {"liczba_rdzeni": 4},
            "swiatla": []
        }
        
    data["obiekty"] = objects
    
    with open(json_path, 'w') as f:
        json.dump(data, f, indent=2)
        
    print(f"Pomyślnie skonwertowano {len(triangles)} trójkątów z {obj_path} do {json_path}.")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Użycie: python obj_to_json.py <plik.obj> <plik.json> [skala] [offsetX offsetY offsetZ] [R G B] [lustrzanosc] [metalicznosc] [moc_emisji]")
        sys.exit(1)
        
    obj_p = sys.argv[1]
    json_p = sys.argv[2]
    
    scale = float(sys.argv[3]) if len(sys.argv) > 3 else 1.0
    
    offset = (0.0, 0.0, 0.0)
    if len(sys.argv) > 6:
        offset = (float(sys.argv[4]), float(sys.argv[5]), float(sys.argv[6]))
        
    color = (0.8, 0.8, 0.8)
    if len(sys.argv) > 9:
        color = (float(sys.argv[7]), float(sys.argv[8]), float(sys.argv[9]))
        
    lustrzanosc = float(sys.argv[10]) if len(sys.argv) > 10 else 0.0
    metalicznosc = float(sys.argv[11]) if len(sys.argv) > 11 else 0.0
    moc_emisji = float(sys.argv[12]) if len(sys.argv) > 12 else 0.0
    
    convert_obj_to_json(obj_p, json_p, scale, offset, color, lustrzanosc, metalicznosc, moc_emisji)
