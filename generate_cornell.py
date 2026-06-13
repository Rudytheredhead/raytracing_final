import os

def create_cornell_box():
    # Vertices
    vertices = [
        # Floor (0 to 3)
        [-1, -1, -1], [1, -1, -1], [1, -1, 1], [-1, -1, 1],
        # Ceiling (4 to 7)
        [-1, 1, -1], [1, 1, -1], [1, 1, 1], [-1, 1, 1],
        # Back wall (8 to 11)
        [-1, -1, -1], [1, -1, -1], [1, 1, -1], [-1, 1, -1],
        # Left wall (12 to 15)
        [-1, -1, -1], [-1, 1, -1], [-1, 1, 1], [-1, -1, 1],
        # Right wall (16 to 19)
        [1, -1, -1], [1, -1, 1], [1, 1, 1], [1, 1, -1],
        # Light on ceiling (20 to 23)
        [-0.3, 0.99, -0.3], [0.3, 0.99, -0.3], [0.3, 0.99, 0.3], [-0.3, 0.99, 0.3],
        
        # Tall Box (24 to 31)
        [-0.5, -1, -0.1], [-0.1, -1, 0.3], [-0.5, 0.2, 0.3], [-0.9, -1, -0.1], # base ... actually let's just make axis aligned for simplicity
        [-0.5, -1, -0.5], [-0.1, -1, -0.5], [-0.1, -1, -0.1], [-0.5, -1, -0.1], # bottom
        [-0.5, 0.2, -0.5], [-0.1, 0.2, -0.5], [-0.1, 0.2, -0.1], [-0.5, 0.2, -0.1], # top
        
        # Short Box (32 to 39)
        [0.1, -1, 0.1], [0.5, -1, 0.1], [0.5, -1, 0.5], [0.1, -1, 0.5],
        [0.1, -0.4, 0.1], [0.5, -0.4, 0.1], [0.5, -0.4, 0.5], [0.1, -0.4, 0.5]
    ]

    mtl_content = """# Cornell Box Materials
newmtl red_wall
Kd 0.8 0.1 0.1
Ke 0 0 0
Ns 0

newmtl green_wall
Kd 0.1 0.8 0.1
Ke 0 0 0
Ns 0

newmtl white_wall
Kd 0.8 0.8 0.8
Ke 0 0 0
Ns 0

newmtl light
Kd 1.0 1.0 1.0
Ke 15.0 15.0 15.0
Ns 0

newmtl mirror_box
Kd 0.9 0.9 0.9
Ke 0 0 0
Ns 90

newmtl yellow_box
Kd 0.8 0.8 0.2
Ke 0 0 0
Ns 10
"""

    obj_content = "mtllib cornell_box.mtl\n\n"
    
    # Write vertices
    for v in vertices:
        obj_content += f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n"
    
    # Helper to write face
    def face(v1, v2, v3, v4=None):
        if v4:
            return f"f {v1+1} {v2+1} {v3+1}\nf {v1+1} {v3+1} {v4+1}\n"
        return f"f {v1+1} {v2+1} {v3+1}\n"

    obj_content += "\nusemtl white_wall\n"
    # Floor
    obj_content += face(0, 1, 2, 3)
    # Ceiling
    obj_content += face(7, 6, 5, 4)
    # Back
    obj_content += face(11, 10, 9, 8)

    obj_content += "\nusemtl red_wall\n"
    # Left
    obj_content += face(12, 13, 14, 15)

    obj_content += "\nusemtl green_wall\n"
    # Right
    obj_content += face(16, 17, 18, 19)

    obj_content += "\nusemtl light\n"
    # Light
    obj_content += face(20, 21, 22, 23)

    # Box helper
    def write_box(start_idx):
        res = ""
        # Front
        res += face(start_idx+4, start_idx+5, start_idx+1, start_idx+0)
        # Back
        res += face(start_idx+6, start_idx+7, start_idx+3, start_idx+2)
        # Left
        res += face(start_idx+7, start_idx+4, start_idx+0, start_idx+3)
        # Right
        res += face(start_idx+5, start_idx+6, start_idx+2, start_idx+1)
        # Top
        res += face(start_idx+7, start_idx+6, start_idx+5, start_idx+4)
        return res

    obj_content += "\nusemtl mirror_box\n"
    obj_content += write_box(24)

    obj_content += "\nusemtl yellow_box\n"
    obj_content += write_box(32)

    with open("cornell_box.mtl", "w") as f:
        f.write(mtl_content)
        
    with open("cornell_box.obj", "w") as f:
        f.write(obj_content)
        
    print("Wygenerowano cornell_box.obj oraz cornell_box.mtl!")

if __name__ == "__main__":
    create_cornell_box()
