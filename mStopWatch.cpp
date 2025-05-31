#include <adwaita.h>
#include <cstddef>
#include <ctime>
#include <format>

namespace {
using i64 = int64_t;

struct UI {
    GtkLabel_autoptr label = nullptr;
    GtkWindow_autoptr window = nullptr;
    AdwToolbarView_autoptr toolbar_view = nullptr;
    AdwHeaderBar_autoptr headbar = nullptr;

    GtkGesture_autoptr ges_shortclick = nullptr;
    GtkGesture_autoptr ges_longpress = nullptr;
};

constexpr int state_file_version = 1;

struct States {
    time_t current_timestamp = 0;
    i64 pause_duration = 0;
    bool paused = false;
};

UI ui {};
States states {};
guint gsource_id = 0;

PangoAttrList* font_attrs_default;
PangoAttrList* font_attrs_red;

gchar* user_state_file = NULL;

time_t get_current_time()
{
    time_t t = time(nullptr);
    if (t == ((time_t)-1)) {
        throw std::runtime_error("Failed to get unix epoch");
    }

    return t;
}

std::string
get_time_string(i64 v)
{
    constexpr i64 seconds_in_hour = 60 * 60;
    constexpr i64 seconds_in_min = 60;

    auto hour_div = std::div(v, seconds_in_hour);
    i64 n_hours = hour_div.quot;

    auto minutes_div = std::div(hour_div.rem, seconds_in_min);
    i64 n_minutes = minutes_div.quot;
    i64 n_seconds = minutes_div.rem;

    return std::format("{}:{:02}:{:02}", n_hours, n_minutes, n_seconds);
}

std::string states_to_str()
{
    return std::format("{}\n{}\n{}\n{}\n", state_file_version, states.current_timestamp, states.pause_duration, states.paused);
}

void states_write()
{
    g_autofree GError* err = nullptr;
    g_file_set_contents(user_state_file, states_to_str().c_str(), (gssize)states_to_str().length(), &err);
    if (err != nullptr) {
        throw std::runtime_error(std::format("Cannot write file {}", err->message));
    }
}

bool states_read()
{
    g_autofree GError* err = nullptr;
    g_autofree gchar* content = nullptr;
    gsize length = 0;
    g_file_get_contents(user_state_file, &content, &length, &err);
    if (err != nullptr) {
        g_info("Cannot read file %s", err->message);
        return false;
    }

    gchar** string_segs = g_strsplit(content, "\n", 0);

    if (g_strv_length(string_segs) != 5 || g_ascii_strtoll(string_segs[0], NULL, 10) != state_file_version) {
        g_strfreev(string_segs);
        return false;
    } else {
        states.current_timestamp = g_ascii_strtoll(string_segs[1], NULL, 10);
        states.pause_duration = g_ascii_strtoll(string_segs[2], NULL, 10);
        const gchar* true_str = "true";
        states.paused = (string_segs[3] == true_str);

        g_strfreev(string_segs);
        return true;
    }
}

gboolean inc_paused_duration(void*)
{
    states.pause_duration += 1;
    return TRUE;
}

gboolean ticking(void*)
{
    auto s = get_time_string(get_current_time() - states.current_timestamp - states.pause_duration);
    gtk_label_set_text(ui.label, s.c_str());
    return TRUE;
}

gboolean start(void*)
{
    gtk_label_set_attributes(ui.label, font_attrs_default);
    if (gsource_id > 0) {
        g_source_destroy(g_main_context_find_source_by_id(g_main_context_default(), gsource_id));
    }

    if (!states_read()) {
        states.current_timestamp = get_current_time();
        states.pause_duration = 0;
        states.paused = false;
    }

    ticking(nullptr);
    gsource_id = g_timeout_add_seconds(1, GSourceFunc(ticking), nullptr);
    states_write();
    return TRUE;
}

gboolean reset()
{
    gtk_label_set_attributes(ui.label, font_attrs_default);
    if (gsource_id > 0) {
        g_source_destroy(g_main_context_find_source_by_id(g_main_context_default(), gsource_id));
    }

    states.current_timestamp = get_current_time();
    states.pause_duration = 0;
    states.paused = false;

    ticking(nullptr);
    gsource_id = g_timeout_add_seconds(1, GSourceFunc(ticking), nullptr);
    states_write();
    return TRUE;
}

gboolean pause_unpause()
{
    g_source_destroy(g_main_context_find_source_by_id(g_main_context_default(), gsource_id));
    if (states.paused) {
        gtk_label_set_attributes(ui.label, font_attrs_default);
        states.paused = false;

        ticking(nullptr);
        gsource_id = g_timeout_add_seconds(1, GSourceFunc(ticking), nullptr);
    } else {
        gtk_label_set_attributes(ui.label, font_attrs_red);
        states.paused = true;

        inc_paused_duration(nullptr);
        gsource_id = g_timeout_add_seconds(1, GSourceFunc(inc_paused_duration), nullptr);
    }

    states_write();

    return TRUE;
}

void cb_startup(GtkApplication* app)
{

    ui.window = (GtkWindow*)adw_application_window_new(app);
    gtk_window_set_resizable(ui.window, FALSE);

    ui.label = (GtkLabel*)gtk_label_new("-:--:--");

    ui.ges_shortclick = gtk_gesture_click_new();
    gtk_widget_add_controller(GTK_WIDGET(ui.label), GTK_EVENT_CONTROLLER(ui.ges_shortclick));
    g_signal_connect(ui.ges_shortclick, "pressed", G_CALLBACK(pause_unpause), NULL);

    ui.ges_longpress = gtk_gesture_long_press_new();
    gtk_widget_add_controller(GTK_WIDGET(ui.label), GTK_EVENT_CONTROLLER(ui.ges_longpress));
    g_signal_connect(ui.ges_longpress, "pressed", G_CALLBACK(reset), NULL);

    ui.toolbar_view = (AdwToolbarView*)adw_toolbar_view_new();
    ui.headbar = ADW_HEADER_BAR(adw_header_bar_new());

    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(ui.toolbar_view), GTK_WIDGET(ui.headbar));

    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(ui.toolbar_view), GTK_WIDGET(ui.label));

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(ui.window), GTK_WIDGET(ui.toolbar_view));

    start(nullptr);
    gtk_window_present(GTK_WINDOW(ui.window));
}

void cb_activate()
{
    gtk_window_present(GTK_WINDOW(ui.window));
}

void init_font_attrs()
{

    PangoFontDescription* fond_desc = pango_font_description_new();
    pango_font_description_set_size(fond_desc, 36 * PANGO_SCALE);

    font_attrs_default = pango_attr_list_new();
    pango_attr_list_insert(font_attrs_default, pango_attr_font_desc_new(fond_desc));

    font_attrs_red = pango_attr_list_new();

    pango_attr_list_insert(font_attrs_red, pango_attr_font_desc_new(fond_desc));
    pango_attr_list_insert(font_attrs_red, pango_attr_foreground_new(65535, 0, 0));

    pango_font_description_free(fond_desc);
}

void init_user_state_dir()
{
    g_autofree const gchar* usr_state_dir = g_get_user_state_dir();
    user_state_file = g_build_path("/", usr_state_dir, "org.slbtty.mstopwatch.txt", NULL);
    g_info("State File -> %s", user_state_file);
}

}

int main(int argc, char* argv[])
{

    init_user_state_dir();

    init_font_attrs();

    g_autoptr(AdwApplication) app = adw_application_new("org.slbtty.mstopwatch", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "startup", G_CALLBACK(cb_startup), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(cb_activate), NULL);

    return g_application_run(G_APPLICATION(app), argc, argv);
}
