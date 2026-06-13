import os

def create_simple_scene():
    mtl_content = """# Proste materialy
newmtl red_mat
Kd 1.0 0.2 0.2
Ke 0 0 0
Ns 0

newmtl green_mat
Kd 0.2 1.0 0.2
Ke 0 0 0
Ns 50

newmtl blue_mat
Kd 0.2 0.2 1.0
Ke 0 0 0
Ns 100
"""

    # 3 cubes: Left (red), Center (green, somewhat shiny), Right (blue, very shiny)
    # Centers: (-2, 0, 0), (0, 0, 0), (2, 0, 0)
    
    def get_cube_verts(cx, cy, cz, size=0.8):
        s = size / 2.0
        return [
            [cx-s, cy-s, cz-s], [cx+s, cy-s, cz-s], [cx+s, cy-s, cz+s], [cx-s, cy-s, cz+s],
            [cx-s, cy+s, cz-s], [cx+s, cy+s, cz-s], [cx+s, cy+s, cz+s], [cx-s, cy+s, cz+s],
        ]

    vertices = []
    vertices.extend(get_cube_verts(-2, 0, 0)) # Red cube
    vertices.extend(get_cube_verts(0, 0, 0))  # Green cube
    vertices.extend(get_cube_verts(2, 0, 0))  # Blue cube

    obj_content = "mtllib simple_cubes.mtl\n\n"
    
    for v in vertices:
        obj_content += f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n"

    def write_box(start_idx):
        res = ""
        def face(v1, v2, v3, v4=None):
            return f"f {v1+1} {v2+1} {v3+1}\nf {v1+1} {v3+1} {v4+1}\n"
        res += face(start_idx+4, start_idx+5, start_idx+1, start_idx+0)
        res += face(start_idx+6, start_idx+7, start_idx+3, start_idx+2)
        res += face(start_idx+7, start_idx+4, start_idx+0, start_idx+3)
        res += face(start_idx+5, start_idx+6, start_idx+2, start_idx+1)
        res += face(start_idx+7, start_idx+6, start_idx+5, start_idx+4)
        res += face(start_idx+0, start_idx+1, start_idx+2, start_idx+3)
        return res

    obj_content += "\nusemtl red_mat\n"
    obj_content += write_box(0)

    obj_content += "\nusemtl green_mat\n"
    obj_content += write_box(8)

    obj_content += "\nusemtl blue_mat\n"
    obj_content += write_box(16)

    with open("simple_cubes.mtl", "w") as f:
        f.write(mtl_content)
        
    with open("simple_cubes.obj", "w") as f:
        f.write(obj_content)
        
    print("Wygenerowano simple_cubes.obj oraz simple_cubes.mtl!")

if __name__ == "__main__":
    create_simple_scene()
