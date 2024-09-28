#include "ezgl/application.hpp"
#include "ezgl/callback.hpp"
#include "ezgl/camera.hpp"
#include "ezgl/canvas.hpp"
#include "ezgl/color.hpp"
#include "ezgl/control.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

/**
 * Draw to the main canvas using the provided graphics object. Runs every time graphics are refreshed/image zooms or pans
 *
 * The graphics object expects that x and y values will be in the main canvas' world coordinate system.
 */
void draw_main_canvas(ezgl::renderer *g);
/**
 * Initial Setup is a mandatory function for any EZGL application, and is run whenever a window is opened.
 */
void initial_setup(ezgl::application *application, bool new_window);

// mouse click functions
void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application * /*application*/, GdkEventButton * /*event*/, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey * /*event*/, char *key_name);

// main drawing functions
void draw_logic_block(ezgl::renderer *g);
void draw_switch_block(ezgl::renderer *g);
void draw_wire(ezgl::renderer *g);

// draw route
void draw_connection(ezgl::renderer *g);

int main(int /*argc*/, char ** /*argv*/)
{
    ezgl::application::settings settings;

    settings.main
}

void draw_main_canvas(ezgl::renderer *g)
{
}

/**
 * Function called before the activation of the application
 * Can be used to create additional buttons, initialize the status message,
 * or connect added widgets to their callback functions
 */
void initial_setup(ezgl::application *application, bool /*new_window*/)
{
    // Update the status bar message
    application->update_message("EZGL Application");

    // Setting our starting row for insertion at 6 (Default zoom/pan buttons created by EZGL take up first five rows);
    // We will increment row each time we insert a new element.
    int row = 6;

    application->create_label(row++, "EXAMPLE UI ELEMENTS: ");

    // Create the Animate button and link it with animate_button callback fn.
    application->create_button("Animate", row++, animate_button_cbk);

    // Create a Test button and link it with test_button callback fn.
    application->create_button("Test", row++, test_button_cbk);

    application->create_label(row++, "Test Combo Box:");

    // Creating example combo box with options Yes, No, Maybe, connected to combo_box_cbk
    application->create_combo_box_text(
        "TestComboBox",
        row++,
        combo_box_cbk,
        {"YES", "NO", "MAYBE"});

    application->create_button("Delete Combo Box", row++, delete_combo_box_cbk);

    application->create_button("Create Dialog", row++, create_dialog_button_cbk);

    application->create_button("Create Popup Mssg", row++, create_mssg_button_cbk);
}

void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y)
{
    application->update_message("Mouse Clicked");

    std::cout << "User clicked the ";

    if (event->button == 1)
    {
        std::cout << "Left "
    };
    else if (event->button == 2)
    {
        std::cout << "Middle ";
    }
    else if (event->button == 3)
    {
        std::cout << "Right ";
    }
    std::cout << "mouse button at coordinates (" << x << "," << y << ") ";

    if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_SHIFT_MASK))
    {
        std::cout << "with control and shift pressed";
    }
    else if (event->state & GDK_CONTROL_MASK)
    {
        std::cout << "with control pressed";
    }
    else if (event->state & GDK_SHIFT_MASK)
    {
        std::cout << "with shift pressed";
    }
    std::cout << std::endl;
}

/**
 * Function to handle mouse move event
 * The current mouse position in the main canvas' world coordinate system is returned
 * A pointer to the application and the entire GDK event are also returned
 */
void act_on_mouse_move(ezgl::application * /*application*/, GdkEventButton * /*event*/, double x, double y)
{
    std::cout << "Mouse move at coordinates (" << x << "," << y << ") " << std::endl;
}

/**
 * Function to handle keyboard press event
 * The name of the key pressed is returned (0-9, a-z, A-Z, Up, Down, Left, Right, Shift_R, Control_L, space, Tab, ...)
 * A pointer to the application and the entire GDK event are also returned
 */
void act_on_mouse_move(ezgl::application * /*application*/, GdkEventButton * /*event*/, double x, double y) void act_on_key_press(ezgl::application *application, GdkEventKey * /*event*/, char *key_name)
{
    application->update_message("Key Pressed");

    std::cout << key_name << " key is pressed" << std::endl;
}
