bl_info = {
    "name": "Map OBJ Exporter Final",
    "author": "Custom",
    "version": (1, 3),
    "blender": (3, 0, 0),
    "location": "View3D > Sidebar > Map Export",
    "description": "Экспорт OBJ с кастомными цветами и выбором пути",
    "category": "Import-Export",
}

#larf


import bpy
import os
import re
import subprocess

# --- ЛОГИКА ---

def linear_to_srgb(c):
    if c <= 0.0031308: return c * 12.92
    return 1.055 * (c ** (1.0 / 2.4)) - 0.055

def get_material_id(mat_name):
    match = re.search(r'Material\.(\d{3})', mat_name)
    return int(match.group(1)) if match else None

def get_used_materials_data():
    data = {}
    visible_meshes = [obj for obj in bpy.context.scene.objects 
                     if obj.type == 'MESH' and obj.visible_get()]
    for obj in visible_meshes:
        for slot in obj.material_slots:
            mat = slot.material
            if not mat: continue
            m_id = get_material_id(mat.name)
            if m_id is None or m_id in data: continue
            r, g, b, a = 255, 255, 255, 255
            try:
                if mat.use_nodes:
                    nodes = mat.node_tree.nodes
                    p_node = next((n for n in nodes if n.type == 'BSDF_PRINCIPLED'), None)
                    if p_node:
                        c = p_node.inputs['Base Color'].default_value
                        r = int(max(0, min(255, linear_to_srgb(c[0]) * 255)))
                        g = int(max(0, min(255, linear_to_srgb(c[1]) * 255)))
                        b = int(max(0, min(255, linear_to_srgb(c[2]) * 255)))
                        if 'Alpha' in p_node.inputs:
                            a = int(max(0, min(255, p_node.inputs['Alpha'].default_value * 255)))
                else:
                    c = mat.diffuse_color
                    r = int(max(0, min(255, linear_to_srgb(c[0]) * 255)))
                    g = int(max(0, min(255, linear_to_srgb(c[1]) * 255)))
                    b = int(max(0, min(255, linear_to_srgb(c[2]) * 255)))
                    a = int(max(0, min(255, c[3] * 255)))
            except: pass
            data[m_id] = (r, g, b, a)
    return data

def finalize_obj(obj_file, id_colors):
    if not os.path.exists(obj_file): return
    with open(obj_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    clean_content = [l for l in lines if not l.startswith('m ')]
    with open(obj_file, 'w', encoding='utf-8', newline='\n') as f:
        for m_id in sorted(id_colors.keys()):
            col = id_colors[m_id]
            f.write(f"m {m_id:03d} {col[0]:03d} {col[1]:03d} {col[2]:03d} {col[3]:03d}\n")
        if id_colors: f.write("\n")
        f.writelines(clean_content)

# --- ИНТЕРФЕЙС ---

class EXPORT_OT_custom_map(bpy.types.Operator):
    bl_idname = "export.custom_map_obj"
    bl_label = "Выполнить экспорт"
    
    def execute(self, context):
        scene = context.scene
        path = bpy.path.abspath(scene.map_export_path)
        
        if not os.path.isdir(path):
            self.report({'ERROR'}, "Выбранный путь не является папкой!")
            return {'CANCELLED'}

        full_path = os.path.join(path, scene.map_export_name)
        if not full_path.lower().endswith(".obj"):
            full_path += ".obj"

        id_colors_map = get_used_materials_data()
        
        params = {
            "filepath": full_path,
            "export_selected_objects": False,
            "apply_modifiers": True,
            "export_uv": False,
            "export_normals": False,
            "export_materials": True,
            "export_triangulated_mesh": True,
            "forward_axis": 'NEGATIVE_Z',
            "up_axis": 'Y',
            "export_object_groups": False,
            "export_material_groups": True
        }

        try:
            bpy.ops.wm.obj_export(**params)
        except:
            bpy.ops.export_scene.obj(filepath=full_path, use_selection=False, use_materials=True)

        finalize_obj(full_path, id_colors_map)
        self.report({'INFO'}, f"Успешно: {scene.map_export_name}")
        return {'FINISHED'}

class OPEN_OT_export_folder(bpy.types.Operator):
    bl_idname = "export.open_folder"
    bl_label = "Открыть папку"
    
    def execute(self, context):
        path = bpy.path.abspath(context.scene.map_export_path)
        if os.path.exists(path):
            if os.name == 'nt': os.startfile(path)
            else: subprocess.Popen(['xdg-open', path])
        return {'FINISHED'}

class VIEW3D_PT_map_export_panel(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Map Export'
    bl_label = 'Экспорт карты'

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        box = layout.box()
        box.label(text="Путь сохранения:", icon='FILE_FOLDER')
        box.prop(scene, "map_export_path", text="")
        
        box.label(text="Имя файла (.obj):", icon='EDITMODE_HLT')
        box.prop(scene, "map_export_name", text="")
        
        layout.separator()
        layout.operator("export.custom_map_obj", icon='EXPORT', text="ЭКСПОРТИРОВАТЬ")
        # Исправлено для Blender 4.4
        layout.operator("export.open_folder", icon='FILE_PARENT', text="ОТКРЫТЬ ПАПКУ")

# --- РЕГИСТРАЦИЯ ---

def register():
    bpy.types.Scene.map_export_path = bpy.props.StringProperty(
        name="Path", subtype='DIR_PATH', default="//")
    bpy.types.Scene.map_export_name = bpy.props.StringProperty(
        name="File Name", default="map1.obj")
    
    bpy.utils.register_class(EXPORT_OT_custom_map)
    bpy.utils.register_class(OPEN_OT_export_folder)
    bpy.utils.register_class(VIEW3D_PT_map_export_panel)

def unregister():
    bpy.utils.unregister_class(EXPORT_OT_custom_map)
    bpy.utils.unregister_class(OPEN_OT_export_folder)
    bpy.utils.unregister_class(VIEW3D_PT_map_export_panel)
    del bpy.types.Scene.map_export_path
    del bpy.types.Scene.map_export_name

if __name__ == "__main__":
    register()