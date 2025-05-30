#include <adwaita.h>
#include <ctime>
#include <format>

using i64 = long;

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

struct States
{
  const int file_version = 1.0;
  time_t timestamp = 0;
  explicit States ()
  {
    timestamp = get_current_time ();
  };
};

States states{};

std::string static GetTimeString (i64 v)
{
  constexpr i64 seconds_in_hour = 60 * 60;
  constexpr i64 seconds_in_min = 60;

  i64 n_hours = -1;
  i64 n_minutes = -1;
  i64 n_seconds = -1;

  auto hour_div = std::div (v, seconds_in_hour);
  n_hours = hour_div.quot;

  auto minutes_div = std::div (hour_div.rem, seconds_in_min);
  n_minutes = minutes_div.quot;
  n_seconds = minutes_div.rem;

  return std::format ("{}:{:02}:{:02}", n_hours, n_minutes, n_seconds);
};

struct UI
{
  GtkLabel *label;
  GtkWindow *window;
  AdwToolbarView *toolbar_view;
  GtkWidget *headbar;
  GtkBox *vbox;
  GtkButton *btn_reset;
};

UI ui{};

static gboolean
reset (gpointer user_data)
{
  states.timestamp = get_current_time ();

  return true;
}

void
init (GtkApplication *app)
{
  ui.window = (GtkWindow *) adw_application_window_new (app);
  gtk_window_set_resizable (ui.window, false);
  gtk_window_set_default_size (ui.window, 200, 0);

  ui.label = (GtkLabel *) gtk_label_new ("-:--:--");
  auto label_font_attr = pango_attr_list_new ();
  PangoFontDescription *label_font_desc = pango_font_description_new ();
  pango_font_description_set_size (label_font_desc, 36 * PANGO_SCALE);
  PangoAttribute *attr = pango_attr_font_desc_new (label_font_desc);
  pango_attr_list_insert (label_font_attr, attr);

  gtk_label_set_attributes (ui.label, label_font_attr);

  ui.toolbar_view = (AdwToolbarView *) adw_toolbar_view_new ();
  ui.headbar = adw_header_bar_new ();
  ui.vbox = (GtkBox *) gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  ui.btn_reset = (GtkButton *) gtk_button_new_with_label ("Reset");
  gtk_widget_add_css_class (GTK_WIDGET (ui.btn_reset), "pill");

  gtk_box_append (ui.vbox, GTK_WIDGET (ui.label));
  gtk_box_append (ui.vbox, GTK_WIDGET (ui.btn_reset));
  g_signal_connect (ui.btn_reset, "clicked", G_CALLBACK (reset), NULL);

  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (ui.toolbar_view), ui.headbar);

  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (ui.toolbar_view), GTK_WIDGET (ui.vbox));

  adw_application_window_set_content (ADW_APPLICATION_WINDOW (ui.window), GTK_WIDGET (ui.toolbar_view));

  gtk_window_present (GTK_WINDOW (ui.window));
}

static gboolean
updateTime (gpointer user_data)
{

  auto s = GetTimeString (get_current_time () - states.timestamp);

  gtk_label_set_text (ui.label, s.c_str ());
  return true;
}

int
main (int argc, char *argv[])
{
  g_autoptr (AdwApplication) app = adw_application_new ("org.slbtty.mStopWatch", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "activate", G_CALLBACK (init), NULL);

  g_timeout_add (1000, GSourceFunc (updateTime), nullptr);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
