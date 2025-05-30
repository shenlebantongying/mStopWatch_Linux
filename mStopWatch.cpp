#include <adwaita.h>
#include <cstddef>
#include <ctime>
#include <format>

namespace
{
using i64 = int64_t;

struct UI
{
  GtkLabel_autoptr label = nullptr;
  GtkWindow_autoptr window = nullptr;
  AdwToolbarView_autoptr toolbar_view = nullptr;
  GtkWidget_autoptr headbar = nullptr;
  GtkButton_autoptr btn_reset = nullptr;

  GtkGesture_autoptr ges_shortclick = nullptr;
  GtkGesture_autoptr ges_longpress = nullptr;
};

struct States
{
  time_t current_timestamp = 0;
  i64 pause_duration = 0;
  bool paused = false;
};

UI ui{};
States states{};
guint gsource_id = 0;

time_t
get_current_time ()
{
  time_t t = time (nullptr);
  if (t == ((time_t) -1))
    {
      throw std::runtime_error ("Failed to get unix epoch");
    }

  return t;
}

std::string
GetTimeString (i64 v)
{
  constexpr i64 seconds_in_hour = 60 * 60;
  constexpr i64 seconds_in_min = 60;

  auto hour_div = std::div (v, seconds_in_hour);
  i64 n_hours = hour_div.quot;

  auto minutes_div = std::div (hour_div.rem, seconds_in_min);
  i64 n_minutes = minutes_div.quot;
  i64 n_seconds = minutes_div.rem;

  return std::format ("{}:{:02}:{:02}", n_hours, n_minutes, n_seconds);
}

gboolean
inc_paused_duration ()
{
  states.pause_duration += 1;
  return TRUE;
}

gboolean
ticking ()
{
  auto s = GetTimeString (get_current_time () - states.current_timestamp - states.pause_duration);
  gtk_label_set_text (ui.label, s.c_str ());
  return TRUE;
}

gboolean
start_reset ()
{

  if (gsource_id > 0)
    {
      g_source_destroy (g_main_context_find_source_by_id (g_main_context_default (), gsource_id));
    }
  states.current_timestamp = get_current_time ();
  states.pause_duration = 0;

  gsource_id = g_timeout_add (1000, GSourceFunc (ticking), nullptr);
  return TRUE;
}

gboolean
pause_unpause ()
{
  g_source_destroy (g_main_context_find_source_by_id (g_main_context_default (), gsource_id));
  if (states.paused)
    {
      states.paused = false;
      gsource_id = g_timeout_add (1000, GSourceFunc (ticking), nullptr);
    }
  else
    {
      states.paused = true;
      gsource_id = g_timeout_add (1000, GSourceFunc (inc_paused_duration), nullptr);
    }

  return TRUE;
}

// GUI

gboolean
reset (gpointer /*user_data*/)
{
  states.current_timestamp = get_current_time ();

  return TRUE;
}

void
init (GtkApplication *app)
{
  ui.window = (GtkWindow *) adw_application_window_new (app);
  gtk_window_set_resizable (ui.window, FALSE);
  gtk_window_set_default_size (ui.window, 200, 0);

  ui.label = (GtkLabel *) gtk_label_new ("-:--:--");

  ui.ges_shortclick = gtk_gesture_click_new ();
  gtk_widget_add_controller (GTK_WIDGET (ui.label), GTK_EVENT_CONTROLLER (ui.ges_shortclick));
  g_signal_connect (ui.ges_shortclick, "pressed", G_CALLBACK (pause_unpause), NULL);

  ui.ges_longpress = gtk_gesture_long_press_new ();
  gtk_widget_add_controller (GTK_WIDGET (ui.label), GTK_EVENT_CONTROLLER (ui.ges_longpress));
  g_signal_connect (ui.ges_longpress, "pressed", G_CALLBACK (start_reset), NULL);

  PangoAttrList *label_font_attr = pango_attr_list_new ();
  PangoFontDescription *label_font_desc = pango_font_description_new ();
  pango_font_description_set_size (label_font_desc, 36 * PANGO_SCALE);
  PangoAttribute *attr = pango_attr_font_desc_new (label_font_desc);
  pango_attr_list_insert (label_font_attr, attr);

  gtk_label_set_attributes (ui.label, label_font_attr);

  ui.toolbar_view = (AdwToolbarView *) adw_toolbar_view_new ();
  ui.headbar = adw_header_bar_new ();
  ui.btn_reset = (GtkButton *) gtk_button_new_with_label ("Reset");
  gtk_widget_add_css_class (GTK_WIDGET (ui.btn_reset), "pill");

  g_signal_connect (ui.btn_reset, "clicked", G_CALLBACK (reset), NULL);

  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (ui.toolbar_view), ui.headbar);

  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (ui.toolbar_view), GTK_WIDGET (ui.label));

  adw_application_window_set_content (ADW_APPLICATION_WINDOW (ui.window), GTK_WIDGET (ui.toolbar_view));

  gtk_window_present (GTK_WINDOW (ui.window));

  start_reset ();
}

}

int
main (int argc, char *argv[])
{
  g_autoptr (AdwApplication) app = adw_application_new ("org.slbtty.mStopWatch", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "activate", G_CALLBACK (init), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
