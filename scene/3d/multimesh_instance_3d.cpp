/**************************************************************************/
/*  multimesh_instance_3d.cpp                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "multimesh_instance_3d.h"

void MultiMeshInstance3D::_refresh_interpolated() {
	if (is_inside_tree() && multimesh.is_valid()) {
		bool interpolated = is_physics_interpolated_and_enabled();
		multimesh->set_physics_interpolated(interpolated);
	}
}

void MultiMeshInstance3D::_physics_interpolated_changed() {
	VisualInstance3D::_physics_interpolated_changed();
	_refresh_interpolated();
}

bool MultiMeshInstance3D::_set(const StringName &p_name, const Variant &p_value) {
	//this is not _too_ bad performance wise, really. it only arrives here if the property was not set anywhere else.
	//add to it that it's probably found on first call to _set anyway.

	if (!get_instance().is_valid()) {
		return false;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();

		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}

		set_surface_override_material(idx, p_value);
		return true;
	}

	return false;
}

bool MultiMeshInstance3D::_get(const StringName &p_name, Variant &r_ret) const {
	if (!get_instance().is_valid()) {
		return false;
	}

	if (p_name.operator String().begins_with("surface_material_override/")) {
		int idx = p_name.operator String().get_slicec('/', 1).to_int();
		if (idx >= surface_override_materials.size() || idx < 0) {
			return false;
		}
		r_ret = surface_override_materials[idx];
		return true;
	}
	return false;
}

void MultiMeshInstance3D::_get_property_list(List<PropertyInfo> *p_list) const {	
	if (multimesh.is_valid()) {
		Ref<Mesh> mesh = multimesh->get_mesh();
		if (mesh.is_valid()) {
			for (int i = 0; i < mesh->get_surface_count(); i++) {
				p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("%s/%d", PNAME("surface_material_override"), i), PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial", PROPERTY_USAGE_DEFAULT));
			}
		}
	}

	
}

void MultiMeshInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_multimesh", "multimesh"), &MultiMeshInstance3D::set_multimesh);
	ClassDB::bind_method(D_METHOD("get_multimesh"), &MultiMeshInstance3D::get_multimesh);

	ClassDB::bind_method(D_METHOD("get_surface_override_material_count"), &MultiMeshInstance3D::get_surface_override_material_count);
	ClassDB::bind_method(D_METHOD("set_surface_override_material", "surface", "material"), &MultiMeshInstance3D::set_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_surface_override_material", "surface"), &MultiMeshInstance3D::get_surface_override_material);
	ClassDB::bind_method(D_METHOD("get_active_material", "surface"), &MultiMeshInstance3D::get_active_material);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "multimesh", PROPERTY_HINT_RESOURCE_TYPE, "MultiMesh"), "set_multimesh", "get_multimesh");
}

void MultiMeshInstance3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		_refresh_interpolated();
	}
}

void MultiMeshInstance3D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "multimesh") {
		WARN_PRINT_ED("Multimesh validated.");
	}
}

void MultiMeshInstance3D::_multimesh_changed() {

	ERR_FAIL_COND(multimesh.is_null());

	Ref<Mesh> mesh = multimesh->get_mesh();
	WARN_PRINT_ED("Multimesh changed.");

	if (mesh.is_valid()) {
		surface_override_materials.resize(mesh->get_surface_count());

		int surface_count = mesh->get_surface_count();
		for (int surface_index = 0; surface_index < surface_count; ++surface_index) {
			if (surface_override_materials[surface_index].is_valid()) {
				RS::get_singleton()->multimesh_set_surface_override_material(get_instance(), surface_index, surface_override_materials[surface_index]->get_rid());
			}
		}
	} else {
		surface_override_materials.resize(0);
	}

	//notify_property_list_changed();
}

void MultiMeshInstance3D::set_multimesh(const Ref<MultiMesh> &p_multimesh) {
	if (multimesh == p_multimesh) {
		return;
	}

	if (multimesh.is_valid()) {
		multimesh->disconnect_changed(callable_mp(this, &MultiMeshInstance3D::_multimesh_changed));
	}

	multimesh = p_multimesh;

	if (multimesh.is_valid()) {	
		set_base(multimesh->get_rid());

		multimesh->connect_changed(callable_mp(this, &MultiMeshInstance3D::_multimesh_changed));
		_multimesh_changed();

		_refresh_interpolated();

	} else {
		set_base(RID());
	}

	notify_property_list_changed();

	// multimesh = p_multimesh;

	// if (multimesh.is_valid()) {
	// 	set_base(multimesh->get_rid());

	// 	// Adapted from MeshInstance3D::_mesh_changed() and inlined.
	// 	Ref<Mesh> mesh = multimesh->get_mesh();
	// 	if (mesh.is_valid()) {
	// 		surface_override_materials.resize(mesh->get_surface_count());

	// 		int surface_count = mesh->get_surface_count();
	// 		for (int surface_index = 0; surface_index < surface_count; ++surface_index) {
	// 			if (surface_override_materials[surface_index].is_valid()) {
	// 				RS::get_singleton()->multimesh_set_surface_override_material(get_instance(), surface_index, surface_override_materials[surface_index]->get_rid());
	// 			}
	// 		}
	// 	} else {
	// 		surface_override_materials.resize(0);
	// 	}

	// 	_refresh_interpolated();
	// } else {
	// 	set_base(RID());
	// }

	// notify_property_list_changed();
}

Ref<MultiMesh> MultiMeshInstance3D::get_multimesh() const {
	return multimesh;
}

Array MultiMeshInstance3D::get_meshes() const {
	if (multimesh.is_null() || multimesh->get_mesh().is_null() || multimesh->get_transform_format() != MultiMesh::TransformFormat::TRANSFORM_3D) {
		return Array();
	}

	int count = multimesh->get_visible_instance_count();
	if (count == -1) {
		count = multimesh->get_instance_count();
	}

	Ref<Mesh> mesh = multimesh->get_mesh();

	Array results;
	for (int i = 0; i < count; i++) {
		results.push_back(multimesh->get_instance_transform(i));
		results.push_back(mesh);
	}
	return results;
}

int MultiMeshInstance3D::get_surface_override_material_count() const {
	return surface_override_materials.size();
}

void MultiMeshInstance3D::set_surface_override_material(int p_surface, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_surface, surface_override_materials.size());

	surface_override_materials.write[p_surface] = p_material;

	if (surface_override_materials[p_surface].is_valid()) {
		RS::get_singleton()->multimesh_set_surface_override_material(get_instance(), p_surface, surface_override_materials[p_surface]->get_rid());
	} else {
		RS::get_singleton()->multimesh_set_surface_override_material(get_instance(), p_surface, RID());
	}
}

Ref<Material> MultiMeshInstance3D::get_surface_override_material(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, surface_override_materials.size(), Ref<Material>());

	return surface_override_materials[p_surface];
}

Ref<Material> MultiMeshInstance3D::get_active_material(int p_surface) const {
	Ref<Material> mat_override = get_material_override();
	if (mat_override.is_valid()) {
		return mat_override;
	}

	Ref<Material> surface_material = get_surface_override_material(p_surface);
	if (surface_material.is_valid()) {
		return surface_material;
	}

	// Ref<Mesh> m = get_mesh();
	Ref<Mesh> m = multimesh.is_valid() ? multimesh->get_mesh() : Ref<Mesh>();
	if (m.is_valid()) {
		return m->surface_get_material(p_surface);
	}

	return Ref<Material>();
}

AABB MultiMeshInstance3D::get_aabb() const {
	if (multimesh.is_null()) {
		return AABB();
	} else {
		return multimesh->get_aabb();
	}
}

MultiMeshInstance3D::MultiMeshInstance3D() {
}

MultiMeshInstance3D::~MultiMeshInstance3D() {
}
