// #include "ezgl/application.hpp"
// #include "ezgl/callback.hpp"
// #include "ezgl/camera.hpp"
// #include "ezgl/canvas.hpp"
// #include "ezgl/color.hpp"
// #include "ezgl/control.hpp"
// #include "ezgl/graphics.hpp"
// #include "ezgl/point.hpp"
// #include "ezgl/rectangle.hpp"
// #include <iostream>
// #include <chrono>
// #include <thread>
// #include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "router.hpp"

/*
X 3$ X X $$ X X X X X X X X X X X
3$ X $$ X X 13 X X X X X X X X X X
X $$ X $$ X X $$ X X X X X X X X X
X X $$ X X X X $$ X X X X X X X X
$$ X X X X 2$ X X $$ X X X X X X X
X 13 X X 2$ X 2$ X X 13 X X X X X X
X X $$ X X 2$ X $$ X X 2$ X X X X X
X X X $$ X X $$ X X X X $$ X X X X
X X X X $$ X X X X $$ X X $$ X X X
X X X X X 13 X X $$ X 1$ X X 3$ X X
X X X X X X 2$ X X 1$ X 1$ X X $$ X
X X X X X X X $$ X X 1$ X X X X 1$
X X X X X X X X $$ X X X X $$ X X
X X X X X X X X X 3$ X X $$ X $$ X
X X X X X X X X X X $$ X X $$ X $$
X X X X X X X X X X X 1$ X X $$ X
*/
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
void draw_logic_blocks(ezgl::renderer *g, int numBlocks, std::vector<std::vector<int>> i_connections);
void draw_switch_blocks(ezgl::renderer *g, int numBlocks);
void draw_wires(ezgl::renderer *g, int W, int numBlocks, std::vector<std::vector<std::string>> adjacencyMatrix, std::vector<std::vector<int>> i_connections);

// draw route
void draw_connection(ezgl::renderer *g);

std::vector<std::vector<std::string>> gridAdjacencyMatrix;
std::vector<std::vector<int>> connectionlist;
int gridN;
int gridW;

/**
 * UI CALLBACK FUNCTIONS
 *
 * These are example callback functions for the UI elements
 */
void animate_button_cbk(GtkWidget *widget, ezgl::application *application);
void test_button_cbk(GtkWidget *widget, ezgl::application *application);
void combo_box_cbk(GtkComboBoxText *self, ezgl::application *app);
void delete_combo_box_cbk(GtkWidget *widget, ezgl::application *application);
void create_dialog_button_cbk(GtkWidget *widget, ezgl::application *application);
void create_mssg_button_cbk(GtkWidget *widget, ezgl::application *application);
void dialog_cbk(GtkDialog *self, gint response_id, ezgl::application *app);

static ezgl::rectangle initial_world{{0, 0}, 1100, 1150};

int main(int /*argc*/, char ** /*argv*/)
{
    ezgl::application::settings settings;

    // Path to the "main.ui" file that contains an XML description of the UI.
    // Edit this file with glade if you want to change the UI layout
    // settings.main_ui_resource = "main.ui";
    settings.main_ui_resource = "main.ui";

    // Note: the "main.ui" file has a GtkWindow called "MainWindow".
    settings.window_identifier = "MainWindow";

    // Note: the "main.ui" file has a GtkDrawingArea called "MainCanvas".
    settings.canvas_identifier = "MainCanvas";

    // Create our EZGL application.
    ezgl::application application(settings);

    // Set some parameters for the main sub-window (MainCanvas), where
    // visualization graphics are draw. Set the callback function that will be
    // called when the main window needs redrawing, and define the (world)
    // coordinate system we want to draw in.
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    Grid g;

    g.processFile("tests/cct1.txt");

    g.initializeGraph();
    g.mazeRouter();
    // g.si_mazeRouter();
    gridAdjacencyMatrix = g.adjacencyMatrix;
    connectionlist = g.i_connections;

    gridN = g.N;
    gridW = g.W;
    g.printMatrix();

    // Run the application until the user quits.
    // This hands over all control to the GTK runtime---after this point
    // you will only regain control based on callbacks you have setup.
    // Three callbacks can be provided to handle mouse button presses,
    // mouse movement and keyboard button presses in the graphics area,
    // respectively. Also, an initial_setup function can be passed that will
    // be called before the activation of the application and can be used
    // to create additional buttons, initialize the status message, or
    // connect added widgets to their callback functions.
    // Those callbacks are optional, so we can pass nullptr if
    // we don't need to take any action on those events
    return application.run(initial_setup, act_on_mouse_press, act_on_mouse_move, act_on_key_press);
}

void draw_main_canvas(ezgl::renderer *g)
{
    draw_wires(g, gridW, gridN, gridAdjacencyMatrix, connectionlist);
    draw_switch_blocks(g, gridN);
    draw_logic_blocks(g, gridN, connectionlist);
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
        std::cout << "left ";
    else if (event->button == 2)
        std::cout << "middle ";
    else if (event->button == 3)
        std::cout << "right ";

    std::cout << "mouse button at coordinates (" << x << "," << y << ") ";

    if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_SHIFT_MASK))
        std::cout << "with control and shift pressed ";
    else if (event->state & GDK_CONTROL_MASK)
        std::cout << "with control pressed ";
    else if (event->state & GDK_SHIFT_MASK)
        std::cout << "with shift pressed ";

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
void act_on_key_press(ezgl::application *application, GdkEventKey * /*event*/, char *key_name)
{
    application->update_message("Key Pressed");

    std::cout << key_name << " key is pressed" << std::endl;
}

/**
 * A callback function to the Animate button. Creates an Animation in the main wundow
 */
void animate_button_cbk(GtkWidget * /*widget*/, ezgl::application *application)
{
    // Get a renderer that can be used to draw on top of the main canvas
    ezgl::renderer *g = application->get_renderer();

    // Set line attributes
    g->set_line_width(2);
    g->set_color(ezgl::RED);
    g->set_line_dash(ezgl::line_dash::asymmetric_5_3);

    ezgl::point2d start_point = {100, 0};

    // Do some animation
    for (int i = 0; i < 50; i++)
    {
        // Draw a line
        g->draw_line(start_point, start_point + ezgl::point2d(500, 10));
        start_point += {10, 20};

        // Flush the drawings done by the renderer to the screen
        application->flush_drawing();

        // Add some delay
        std::chrono::milliseconds duration(50);
        std::this_thread::sleep_for(duration);
    }
}

/**
 * A callback function to test the Test button. Changes application message when button is pressed
 */
void test_button_cbk(GtkWidget * /*widget*/, ezgl::application *application)
{
    // Update the status bar message
    application->update_message("Test Button Pressed");

    // Redraw the main canvas
    application->refresh_drawing();
}

/**
 * Callback function for the example combo box. Sets message to currently active option.
 * Function trigerred when currently selected option changes.
 */
void combo_box_cbk(GtkComboBoxText *self, ezgl::application *app)
{
    // Getting text content of combo box. This call makes a copy that we must free
    auto text = gtk_combo_box_text_get_active_text(self);
    if (!text)
    { // Returning if the combo box is currently empty (Always check to avoid errors)
        return;
    }
    else
    { // Updating message to reflect new combo box value.
        app->update_message(text);
        g_free(text); // gtk made a copy that we own; need to free.
    }
}

/**
 * Callback function for the delete combo box button. Deletes combo box.
 */
void delete_combo_box_cbk(GtkWidget *widget, ezgl::application *app)
{
    // Destroying widget. If function fails (could not find widget), changing message to reflect failure.
    if (app->destroy_widget("TestComboBox"))
    {
        app->update_message("Successfully deleted");
    }
    else
    {
        app->update_message("Already deleted or not found");
    }
}

/**
 * Callback function for the create dialog button. Creates a dialog window and connects it to the dialog_cbk function
 */
void create_dialog_button_cbk(GtkWidget * /*widget*/, ezgl::application *application)
{
    application->create_dialog_window(dialog_cbk, "Title", "THIS IS SOME TEXT. HELLO!");
}

/**
 * Callback function for the create message button. Creates a popup message
 */
void create_mssg_button_cbk(GtkWidget * /*widget*/, ezgl::application *app)
{
    app->create_popup_message("My Message", "Hello, hit Done to Proceed");
}

/**
 * Callback function for dialog window created by "Create Dialog Window" button.
 * Updates application message to reflect user answer to dialog window.
 */
void dialog_cbk(GtkDialog *self, gint response_id, ezgl::application *app)
{
    // Response_id is an integer/enumeration, so we can use a switch to read its value and act accordingly
    switch (response_id)
    {
    case GTK_RESPONSE_ACCEPT:
        app->update_message("USER ACCEPTED");
        break;
    case GTK_RESPONSE_REJECT:
        app->update_message("USER REJECTED");
        break;
    case GTK_RESPONSE_DELETE_EVENT:
        app->update_message("USER CLOSED WINDOW");
        break;
    default:
        app->update_message("YOU SHOULD NOT SEE THIS");
    }

    // We always have to destroy the dialog window in the callback function or it will never close
    gtk_widget_destroy(GTK_WIDGET(self));
}

void draw_logic_blocks(ezgl::renderer *g, int numBlocks, std::vector<std::vector<int>> i_connections)
{
    g->set_font_size(10);
    g->set_line_width(2);
    ezgl::color color_indicies[] = {
        ezgl::RED,
        ezgl::YELLOW,
        ezgl::GREEN,
        ezgl::GREY_75,
        ezgl::CYAN,
        ezgl::BLUE,
        ezgl::PURPLE,
        ezgl::DARK_GREEN,
        ezgl::PINK,
        ezgl::BISQUE,
        ezgl::MAGENTA,
        ezgl::ORANGE,
        ezgl::LIGHT_PINK,
        ezgl::KHAKI,
        ezgl::FIRE_BRICK,
        ezgl::LIGHT_SKY_BLUE,
        ezgl::PLUM,
        ezgl::GREY_55,
        ezgl::CORAL,
        ezgl::LIGHT_MEDIUM_BLUE,
        ezgl::TURQUOISE,
        ezgl::DARK_KHAKI,
        ezgl::MEDIUM_PURPLE,
        ezgl::THISTLE,
        ezgl::DARK_SLATE_BLUE,
        ezgl::SADDLE_BROWN,
        ezgl::LIME_GREEN};

    for (int i = 0; i < numBlocks; i++)
    {
        for (int j = 0; j < numBlocks; j++)
        {
            g->set_color(ezgl::BLACK);
            const ezgl::point2d start_point{350 + (i * 400), 230 + (j * 400)};
            ezgl::rectangle switchblock = {start_point, 100, 100};
            g->fill_rectangle(switchblock);
        }
    }
    // all outputs
    for (int i = 0; i < numBlocks; i++)
    {
        for (int j = 0; j < numBlocks; j++)
        {
            g->set_color(ezgl::BLACK);
            g->draw_line({400 + ((i) * 400), 300 + ((j) * 400)}, {600 + ((i) * 400), 300 + ((j) * 400)});
            g->draw_line({400 + ((i) * 400), 260 + ((j) * 400)}, {200 + ((i) * 400), 260 + ((j) * 400)});
            g->draw_line({380 + ((i) * 400), 280 + ((j) * 400)}, {380 + ((i) * 400), 480 + ((j) * 400)});
            g->draw_line({420 + ((i) * 400), 280 + ((j) * 400)}, {420 + ((i) * 400), 80 + ((j) * 400)});
        }
    }
    for (int i = 0; i < i_connections.size(); i++)
    {
        std::vector<int> connection = i_connections[i];
        std::vector<int> block1 = {connection[0], connection[1], connection[2]};
        std::vector<int> block2 = {connection[3], connection[4], connection[5]};
        g->set_color(color_indicies[(i - 1) % (i_connections.size() - 1) + 1]);
        g->draw_line({400 + ((connection[0]) * 400), 300 + ((connection[1]) * 400)}, {600 + ((connection[0]) * 400), 300 + ((connection[1]) * 400)});
        if (connection[5] == 3)
        {
            g->draw_line({380 + ((connection[3]) * 400), 280 + ((connection[4]) * 400)}, {380 + ((connection[3]) * 400), 480 + ((connection[4]) * 400)});
        }
        else if (connection[5] == 2)
        {
            g->draw_line({400 + ((connection[3]) * 400), 260 + ((connection[4]) * 400)}, {200 + ((connection[3]) * 400), 260 + ((connection[4]) * 400)});
        }
        else if (connection[5] == 1)
        {
            g->draw_line({420 + ((connection[3]) * 400), 280 + ((connection[4]) * 400)}, {420 + ((connection[3]) * 400), 80 + ((connection[4]) * 400)});
        }
    }
}

void draw_switch_blocks(ezgl::renderer *g, int numBlocks)
{
    for (int i = 0; i <= numBlocks; i++)
    {
        for (int j = 0; j <= numBlocks; j++)
        {
            g->set_color(ezgl::BLACK);
            const ezgl::point2d start_point{150 + (i * 400), 30 + (j * 400)};
            ezgl::rectangle switchblock = {start_point, 100, 100};
            g->set_color(ezgl::LIGHT_SKY_BLUE);
            g->fill_rectangle(switchblock);
        }
    }
}

void draw_wires(ezgl::renderer *g, int W, int numBlocks, std::vector<std::vector<std::string>> adjacencyMatrix, std::vector<std::vector<int>> i_connections)
{
    g->set_font_size(10);
    g->set_line_width(2);

    ezgl::color color_indicies[] = {
        ezgl::RED,
        ezgl::YELLOW,
        ezgl::GREEN,
        ezgl::GREY_75,
        ezgl::CYAN,
        ezgl::BLUE,
        ezgl::PURPLE,
        ezgl::DARK_GREEN,
        ezgl::PINK,
        ezgl::BISQUE,
        ezgl::MAGENTA,
        ezgl::ORANGE,
        ezgl::LIGHT_PINK,
        ezgl::KHAKI,
        ezgl::FIRE_BRICK,
        ezgl::LIGHT_SKY_BLUE,
        ezgl::PLUM,
        ezgl::GREY_55,
        ezgl::CORAL,
        ezgl::LIGHT_MEDIUM_BLUE,
        ezgl::TURQUOISE,
        ezgl::DARK_KHAKI,
        ezgl::MEDIUM_PURPLE,
        ezgl::THISTLE,
        ezgl::DARK_SLATE_BLUE,
        ezgl::SADDLE_BROWN,
        ezgl::LIME_GREEN};

    float wireSeparation = 100 / W;
    // horizontal lines
    for (int i = 0; i <= numBlocks; i++)
    {
        for (int j = 0; j < numBlocks; j++)
        {
            int curr = ((i) * (numBlocks + 1)) + j;
            std::string temp;
            for (int k = 0; k < W; k++)
            {
                temp = adjacencyMatrix[curr][curr + 1];
                if (temp[k] == '$')
                {
                    g->set_color(ezgl::BLACK);
                }
                else
                {
                    g->set_color(color_indicies[((temp[k] - '0') - 1) % (i_connections.size() - 1) + 1]);
                }
                g->draw_line({250 + (j * 400), (30 + (i * 400)) + (k * wireSeparation)}, {550 + (j * 400), (30 + (i * 400)) + (k * wireSeparation)});
            }
        }
    }
    // vertical lines
    for (int i = 0; i <= numBlocks; i++)
    {
        for (int j = 0; j < numBlocks; j++)
        {
            int curr = ((j) * (numBlocks + 1)) + i;
            std::string temp;
            for (int k = 0; k < W; k++)
            {
                temp = adjacencyMatrix[curr][curr + numBlocks + 1];
                if (temp[k] == '$')
                {
                    g->set_color(ezgl::BLACK);
                }
                else
                {
                    g->set_color(color_indicies[((temp[k] - '0') - 1) % (i_connections.size() - 1) + 1]);
                }
                g->draw_line({(150 + (i * 400)) + (k * wireSeparation), 130 + (j * 400)}, {(150 + (i * 400)) + (k * wireSeparation), 430 + (j * 400)});
            }
        }
    }
}
