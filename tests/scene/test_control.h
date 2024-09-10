/**************************************************************************/
/*  test_control.h                                                        */
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

#ifndef TEST_CONTROL_H
#define TEST_CONTROL_H

#include "scene/gui/control.h"

#include "tests/test_macros.h"

namespace TestControl {

TEST_CASE("[SceneTree][Control]") {
	SUBCASE("[Control][Global Transform] Global Transform should be accessible while not in SceneTree.") { // GH-79453
		Control *test_node = memnew(Control);
		Control *test_child = memnew(Control);
		test_node->add_child(test_child);

		test_node->set_global_position(Point2(1, 1));
		CHECK_EQ(test_node->get_global_position(), Point2(1, 1));
		CHECK_EQ(test_child->get_global_position(), Point2(1, 1));
		test_node->set_global_position(Point2(2, 2));
		CHECK_EQ(test_node->get_global_position(), Point2(2, 2));
		test_node->set_scale(Vector2(4, 4));
		CHECK_EQ(test_node->get_global_transform(), Transform2D(0, Size2(4, 4), 0, Vector2(2, 2)));
		test_node->set_scale(Vector2(1, 1));
		test_node->set_rotation_degrees(90);
		CHECK_EQ(test_node->get_global_transform(), Transform2D(Math_PI / 2, Vector2(2, 2)));
		test_node->set_pivot_offset(Vector2(1, 0));
		CHECK_EQ(test_node->get_global_transform(), Transform2D(Math_PI / 2, Vector2(3, 1)));

		memdelete(test_child);
		memdelete(test_node);
	}
}

TEST_CASE("[Control][SceneTree] Focus behavior.") {
	Window *root = SceneTree::get_singleton()->get_root();
	int size = 100;
	int margin = 5;

	SUBCASE("[Control][SceneTree] Navigation of four controls in a grid, with separation between controls.") {
		Control *control_up_left = memnew(Control);
		Control *control_up_right = memnew(Control);
		Control *control_down_left = memnew(Control);
		Control *control_down_right = memnew(Control);

		root->add_child(control_up_left);
		root->add_child(control_up_right);
		root->add_child(control_down_left);
		root->add_child(control_down_right);

		// Arrange four controls in a grid.
		control_up_left->set_focus_mode(Control::FOCUS_ALL);
		control_up_left->set_size(Size2(size, size));
		control_up_left->set_global_position(Point2(0, 0));

		control_up_right->set_focus_mode(Control::FOCUS_ALL);
		control_up_right->set_size(Size2(size, size));
		control_up_right->set_global_position(Point2(size + margin, 0));

		control_down_left->set_focus_mode(Control::FOCUS_ALL);
		control_down_left->set_size(Size2(size, size));
		control_down_left->set_global_position(Point2(0, size + margin));

		control_down_right->set_focus_mode(Control::FOCUS_ALL);
		control_down_right->set_size(Size2(size, size));
		control_down_right->set_global_position(Point2(size + margin, size + margin));

		// Focus the upper-left control, then visit the four controls in clockwise order.
		control_up_left->grab_focus();
		CHECK(root->gui_get_focus_owner() == control_up_left);
		SEND_GUI_ACTION("ui_right");
		CHECK(root->gui_get_focus_owner() == control_up_right);
		SEND_GUI_ACTION("ui_down");
		CHECK(root->gui_get_focus_owner() == control_down_right);
		SEND_GUI_ACTION("ui_left");
		CHECK(root->gui_get_focus_owner() == control_down_left);
		SEND_GUI_ACTION("ui_up");
		CHECK(root->gui_get_focus_owner() == control_up_left);

		// The upper-left control has no up or left neighbors, so focus should not change.
		SEND_GUI_ACTION("ui_left");
		CHECK(root->gui_get_focus_owner() == control_up_left);
		SEND_GUI_ACTION("ui_up");
		CHECK(root->gui_get_focus_owner() == control_up_left);

		// Manually set the focus neighbors, so attempting to navigate clockwise instead navigates counterclockwise.
		control_up_left->set_focus_neighbor(SIDE_RIGHT, control_up_left->get_path_to(control_down_left));
		CHECK(control_up_left->get_focus_neighbor(SIDE_RIGHT) == control_up_left->get_path_to(control_down_left));

		control_down_left->set_focus_neighbor(SIDE_TOP, control_down_left->get_path_to(control_down_right));
		CHECK(control_down_left->get_focus_neighbor(SIDE_TOP) == control_down_left->get_path_to(control_down_right));

		control_down_right->set_focus_neighbor(SIDE_LEFT, control_down_right->get_path_to(control_up_right));
		CHECK(control_down_right->get_focus_neighbor(SIDE_LEFT) == control_down_right->get_path_to(control_up_right));

		control_up_right->set_focus_neighbor(SIDE_BOTTOM, control_up_right->get_path_to(control_up_left));
		CHECK(control_up_right->get_focus_neighbor(SIDE_BOTTOM) == control_up_right->get_path_to(control_up_left));

		control_up_left->grab_focus();
		CHECK(root->gui_get_focus_owner() == control_up_left);
		SEND_GUI_ACTION("ui_right");
		CHECK(root->gui_get_focus_owner() == control_down_left);
		SEND_GUI_ACTION("ui_up");
		CHECK(root->gui_get_focus_owner() == control_down_right);
		SEND_GUI_ACTION("ui_left");
		CHECK(root->gui_get_focus_owner() == control_up_right);
		SEND_GUI_ACTION("ui_down");
		CHECK(root->gui_get_focus_owner() == control_up_left);

		memdelete(control_up_left);
		memdelete(control_up_right);
		memdelete(control_down_left);
		memdelete(control_down_right);
	}

	SUBCASE("[Control][SceneTree] Navigation of three square controls above one wide control.") {
		Control *control_up_left = memnew(Control);
		Control *control_up_middle = memnew(Control);
		Control *control_up_right = memnew(Control);
		Control *control_down = memnew(Control);

		root->add_child(control_up_left);
		root->add_child(control_up_right);
		root->add_child(control_up_middle);
		root->add_child(control_down);

		// Arrange three small square controls above one wide control.
		control_up_left->set_focus_mode(Control::FOCUS_ALL);
		control_up_left->set_size(Size2(size, size));
		control_up_left->set_global_position(Point2(0, 0));

		control_up_middle->set_focus_mode(Control::FOCUS_ALL);
		control_up_middle->set_size(Size2(size, size));
		control_up_middle->set_global_position(Point2(size + margin, 0));

		control_up_right->set_focus_mode(Control::FOCUS_ALL);
		control_up_right->set_size(Size2(size, size));
		control_up_right->set_global_position(Point2(2 * size + 2 * margin, 0));

		control_down->set_focus_mode(Control::FOCUS_ALL);
		control_down->set_size(Size2(3 * size + 2 * margin, size));
		control_down->set_global_position(Point2(0, size + margin));

		// Focus the each upper control, then navigate down.
		control_up_left->grab_focus();
		SEND_GUI_ACTION("ui_down");
		CHECK(root->gui_get_focus_owner() == control_down);

		control_up_middle->grab_focus();
		SEND_GUI_ACTION("ui_down");
		CHECK(root->gui_get_focus_owner() == control_down);

		control_up_right->grab_focus();
		SEND_GUI_ACTION("ui_down");
		CHECK(root->gui_get_focus_owner() == control_down);

		memdelete(control_up_left);
		memdelete(control_up_middle);
		memdelete(control_up_right);
		memdelete(control_down);
	}
}

} // namespace TestControl

#endif // TEST_CONTROL_H
