#include <glib.h>
#include <gtk/gtk.h>
#include <marco-private/preview-widget.h>
#include <marco-private/theme-parser.h>
#include <marco-private/theme.h>
#include <marco-private/util.h>
#include <stdint.h>

// 定义命令行选项的结构体
typedef struct
{
    gboolean show_help;
    gchar **theme_names;
    gchar *thumbnail_directory;
    gint thumbnail_width;
    gint thumbnail_height;
} Options;

static Options options;

// example: thumbnailer -n Kiran-dark -n Kiran-white -d /tmp/thumbnail -w 128 -h 128
// 定义命令行选项的条目
static GOptionEntry entries[] = {
    {"help", 'h', 0, G_OPTION_ARG_NONE, &options.show_help, "Show help options", NULL},
    {"theme-name", 'n', 0, G_OPTION_ARG_STRING_ARRAY, &options.theme_names, "thumbnail theme", "theme1"},
    {"thumbnail-directory", 'd', 0, G_OPTION_ARG_STRING, &options.thumbnail_directory, "Directory containing themes thumbnail", "DIRECTORY"},
    {"thumbnail-width", 'w', 0, G_OPTION_ARG_INT, &options.thumbnail_width, "Width of the thumbnail", "WIDTH"},
    {"thumbnail-height", 't', 0, G_OPTION_ARG_INT, &options.thumbnail_height, "Height of the thumbnail", "HEIGHT"},
    {NULL}};

// 透明pixbuf中不在region中的部分
static void pixbuf_apply_mask_region(GdkPixbuf *pixbuf, cairo_region_t *region)
{
    gint nchannels, rowstride, w, h;
    guchar *pixels, *p;

    g_return_if_fail(pixbuf);
    g_return_if_fail(region);

    nchannels = gdk_pixbuf_get_n_channels(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    /* we need an alpha channel ... */
    if (!gdk_pixbuf_get_has_alpha(pixbuf) || nchannels != 4)
        return;

    for (w = 0; w < gdk_pixbuf_get_width(pixbuf); ++w)
        for (h = 0; h < gdk_pixbuf_get_height(pixbuf); ++h)
        {
            if (!cairo_region_contains_point(region, w, h))
            {
                p = pixels + h * rowstride + w * nchannels;
                if (G_BYTE_ORDER == G_BIG_ENDIAN)
                    p[0] = 0x0;
                else
                    p[3] = 0x0;
            }
        }
}

#if 0
// 列出主题目录
GList *list_marco_theme_directory(const gchar *directory)
{
    GDir *dir;
    const gchar *name;
    GList *theme_list = NULL;

    dir = g_dir_open(directory, 0, NULL);
    if (dir == NULL)
    {
        g_printerr("Cannot open directory '%s'\n", directory);
        return NULL;
    }

    while ((name = g_dir_read_name(dir)) != NULL)
    {
        g_autofree gchar *full_path = NULL;
        g_autofree gchar *theme_1_path, *theme_2_path, *theme_3_path;

        full_path = g_build_filename(directory, name, NULL);
        theme_1_path = g_build_filename(full_path, "metacity-1", "metacity-theme-1.xml", NULL);
        theme_2_path = g_build_filename(full_path, "metacity-1", "metacity-theme-2.xml", NULL);
        theme_3_path = g_build_filename(full_path, "metacity-1", "metacity-theme-3.xml", NULL);

        if (!(g_file_test(theme_1_path, G_FILE_TEST_EXISTS) ||
              g_file_test(theme_2_path, G_FILE_TEST_EXISTS) ||
              g_file_test(theme_3_path, G_FILE_TEST_EXISTS)))
        {
            continue;
        }

        theme_list = g_list_append(theme_list, g_strdup(name));
    }

    g_dir_close(dir);
    return theme_list;
}
#endif

// 生成窗口缩略图
// see mate-control-center create_marco_theme_pixbuf
GdkPixbuf *generator_thumbnail(const gchar *theme_name, const uint32_t width, const uint32_t height)
{
    MetaTheme *theme = meta_theme_load((char *)theme_name, NULL);
    if (!theme)
    {
        return NULL;
    }

    MetaFrameFlags flags;
    flags = META_FRAME_ALLOWS_DELETE |
            META_FRAME_ALLOWS_MENU |
            META_FRAME_ALLOWS_MINIMIZE |
            META_FRAME_ALLOWS_MAXIMIZE |
            META_FRAME_ALLOWS_VERTICAL_RESIZE |
            META_FRAME_ALLOWS_HORIZONTAL_RESIZE |
            META_FRAME_HAS_FOCUS |
            META_FRAME_ALLOWS_SHADE |
            META_FRAME_ALLOWS_MOVE;

    GtkWidget *offscreen_window = gtk_offscreen_window_new();
    gtk_window_set_default_size(GTK_WINDOW(offscreen_window),
                                (int)width * 1.2,
                                (int)height * 1.2);
    // 设置离屏窗口背景透明
    gtk_widget_set_app_paintable(offscreen_window, TRUE);

    GtkWidget *preview = meta_preview_new();
    meta_preview_set_frame_flags(META_PREVIEW(preview), flags);
    meta_preview_set_theme(META_PREVIEW(preview), theme);
    meta_preview_set_title(META_PREVIEW(preview), "");
    gtk_container_add(GTK_CONTAINER(offscreen_window), preview);

    GtkWidget *dummy = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(preview), dummy);

    gtk_widget_show_all(offscreen_window);

    GtkRequisition requisition;
    GtkAllocation allocation;

    // 更新大小
    gtk_widget_get_preferred_size(offscreen_window, &requisition, NULL);
    allocation.x = 0;
    allocation.y = 0;
    allocation.width = (int)width * 1.2;
    allocation.height = (int)height * 1.2;
    gtk_widget_size_allocate(offscreen_window, &allocation);
    gtk_widget_get_preferred_size(offscreen_window, &requisition, NULL);

    // 处理pending事件，处理大小处理绘制
    gtk_widget_queue_draw(offscreen_window);
    while (gtk_events_pending())
        gtk_main_iteration();

    // 拿取窗口图片
    GdkPixbuf *pixbuf = gtk_offscreen_window_get_pixbuf(GTK_OFFSCREEN_WINDOW(offscreen_window));
    cairo_region_t *region = meta_preview_get_clip_region(META_PREVIEW(preview),
                                                          width * 1.2, height * 1.2);

    pixbuf_apply_mask_region(pixbuf, region);
    cairo_region_destroy(region);

    GdkPixbuf *retval = gdk_pixbuf_scale_simple(pixbuf,
                                                width,
                                                height,
                                                GDK_INTERP_BILINEAR);

    g_object_unref(pixbuf);
    gtk_widget_destroy(offscreen_window);
    meta_theme_free(theme);
    return retval;
}

gboolean check_wm_theme(const gchar *theme_name)
{
    static const char *theme_dir = "/usr/share/themes/";
    g_autofree gchar *theme_1_path = g_build_filename(theme_dir, theme_name, "metacity-1", "metacity-theme-1.xml", NULL);
    g_autofree gchar *theme_2_path = g_build_filename(theme_dir, theme_name, "metacity-1", "metacity-theme-2.xml", NULL);
    g_autofree gchar *theme_3_path = g_build_filename(theme_dir, theme_name, "metacity-1", "metacity-theme-3.xml", NULL);
    return (g_file_test(theme_1_path, G_FILE_TEST_EXISTS) ||
            g_file_test(theme_2_path, G_FILE_TEST_EXISTS) ||
            g_file_test(theme_3_path, G_FILE_TEST_EXISTS));
}

gboolean check_thumbnail_directory(const gchar *dir)
{
    if (!g_file_test(dir, G_FILE_TEST_IS_DIR))
    {
        g_warning("Directory '%s' does not exist\n", dir);
        return FALSE;
    }

    if (access(dir, W_OK) != 0)
    {
        g_warning("Directory '%s' is not writable\n", dir);
        return FALSE;
    }

    return TRUE;
}

gboolean check_old_thumbnail_valid(const gchar *path, const uint32_t width, const uint32_t height)
{
    if (!g_file_test(path, G_FILE_TEST_EXISTS))
    {
        return FALSE;
    }

    g_autoptr(GdkPixbuf) pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (pixbuf == NULL)
    {
        return FALSE;
    }

    if (gdk_pixbuf_get_width(pixbuf) != width ||
        gdk_pixbuf_get_height(pixbuf) != height)
    {
        return FALSE;
    }

    return TRUE;
}

int main(int argc, char *argv[])
{
    GError *error = NULL;
    gboolean bRes = FALSE;

    g_autoptr(GOptionContext) context = g_option_context_new("- GTK Theme Thumbnailer");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_add_group(context, gtk_get_option_group(TRUE));

    // 解析命令行选项
    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_print("option parsing failed: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    gboolean valid = TRUE;
    if (!options.thumbnail_directory || !options.theme_names ||
        !options.thumbnail_height || !options.thumbnail_width)
    {
        g_warning("--theme-directory,--theme-name,--thumbnail-height,--thumbnail-width are mutually exclusive\n");
        valid = FALSE;
    }

    // 显示帮助信息
    if (options.show_help || !valid)
    {
        g_print("%s\n", g_option_context_get_help(context, TRUE, NULL));
        return 0;
    }

    // 检查缩略图目录是否有效
    if (!check_thumbnail_directory(options.thumbnail_directory))
    {
        g_warning("Invalid directory %s\n", options.thumbnail_directory);
        return EXIT_FAILURE;
    }

    const gchar *directory = options.thumbnail_directory;
    const uint32_t width = options.thumbnail_width;
    const uint32_t height = options.thumbnail_height;
    int i;
    for (i = 0; options.theme_names[i]; i++)
    {
        const gchar *name = options.theme_names[i];
        g_print("Processing theme %s\n", name);
        if (!check_wm_theme(name))
        {
            g_warning("Invalid theme name %s\n", name);
            continue;
        }

        g_autofree gchar *thumbnailer_path = g_strdup_printf("%s/%s.png", directory, name);
        if (check_old_thumbnail_valid(thumbnailer_path, width, height))
        {
            g_print("%s thumbnailer is valid, skip generate\n", thumbnailer_path);
            continue;
        }

        g_autoptr(GdkPixbuf) pixbuf = generator_thumbnail(name, width, height);
        if (!pixbuf)
        {
            g_warning("Failed to generate thumbnail of %s\n", name);
            continue;
        }

        bRes = gdk_pixbuf_save(pixbuf, thumbnailer_path, "png", &error, NULL);
        if (!bRes)
        {
            g_printerr("Generate thumbnail %s failed,%s\n", thumbnailer_path, error->message);
            g_error_free(error);
            continue;
        }
        g_print("Generate thumbnail for %s success: %s\n", name, thumbnailer_path);
    }
    return 0;
}