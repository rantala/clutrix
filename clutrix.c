#include <clutter/clutter.h>
#include <pango/pangocairo.h>

static gboolean windowed = 1;

static void
set_pango_font(PangoLayout *layout, unsigned min, unsigned max)
{
	static char font[32];
	PangoFontDescription *desc;
	sprintf(font, "Monospace %dpx", g_random_int_range(min, max));
	desc = pango_font_description_from_string(font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
}

static char *
make_utf8(unsigned chars)
{
	char *buf;
	unsigned i, pos;
	/* One unicode character takes max. 6 bytes in UTF-8. */
	buf = g_malloc(chars*7);
	for (pos=0, i=0; i < chars; ++i) {
		gunichar u = g_random_int_range(0x30a1, 0x30fa);
		int len = g_unichar_to_utf8(u, &buf[pos]);
		buf[pos+len] = '\n';
		pos += len+1;
	}
	buf[pos-1] = 0;
	return buf;
}

static ClutterActor *
make_vertical_actor(unsigned chars, const ClutterColor *text_color)
{
	static const unsigned font_min_px=6, font_max_px=20;
	const unsigned maxw=font_max_px, maxh=2*font_max_px*chars;
	char *text;
	int width, height;
	cairo_t *cr;
	cairo_surface_t *surface;
	PangoLayout *layout;
	ClutterActor *actor;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, maxw, maxh);
	cr = cairo_create(surface);
	layout = pango_cairo_create_layout(cr);
	set_pango_font(layout, font_min_px, font_max_px);
	text = make_utf8(chars);
	pango_layout_set_text(layout, text, -1);
	g_free(text);
	clutter_cairo_set_source_color(cr, text_color);
	pango_cairo_show_layout(cr, layout);
	pango_layout_get_size (layout, &width, &height);
	width = PANGO_PIXELS(width);
	height = PANGO_PIXELS(height);
	actor = clutter_texture_new();
	clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(actor),
			cairo_image_surface_get_data(surface),
			TRUE,
			width,
			height,
			cairo_image_surface_get_stride(surface),
			4,
			CLUTTER_TEXTURE_NONE,
			NULL);
	cairo_surface_destroy(surface);
	cairo_destroy(cr);
	g_object_unref(layout);
	return actor;
}

static ClutterActor *
make_vertical_strip(void)
{
	static const ClutterColor text_color = {   0, 255,   0, 255 };
	static const ClutterColor last_color = { 255, 255, 255, 255 };
	ClutterActor *group, *actor1, *actor2;
	unsigned chars = g_random_int_range(100, 200);
	group = clutter_group_new();
	actor1 = make_vertical_actor(chars, &text_color);
	actor2 = make_vertical_actor(1, &last_color);
	clutter_container_add_actor(CLUTTER_CONTAINER(group), actor1);
	clutter_container_add_actor(CLUTTER_CONTAINER(group), actor2);
	clutter_actor_set_y(actor2, clutter_actor_get_height(actor1));
	g_debug("%s(): %u characters, actor size: [%2g, %4g]",
			__func__, chars+1,
			clutter_actor_get_width(group),
			clutter_actor_get_height(group));
	return group;
}

int main(int argc, char **argv)
{
	static const ClutterColor stage_color = { 0, 0, 0, 255 };
	float xpos=0;
	clutter_init(&argc, &argv);
	ClutterActor *stage = clutter_stage_get_default();
	clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
	if (windowed) {
		clutter_actor_set_size(stage, 640, 480);
	} else {
		clutter_stage_set_fullscreen(CLUTTER_STAGE(stage), TRUE);
	}
	clutter_stage_hide_cursor(CLUTTER_STAGE(stage));
	clutter_stage_set_title(CLUTTER_STAGE(stage), "Clutrix");
	while (xpos < CLUTTER_STAGE_WIDTH()) {
		ClutterActor *actor = make_vertical_strip();
		clutter_stage_add(stage, actor);
		clutter_actor_set_position(actor, xpos, 0);
		ClutterAnimation *anim = clutter_actor_animate(actor,
				CLUTTER_EASE_IN_SINE,
				g_random_int_range(3000,9000),
				"fixed::opacity", (guchar)255,
				"fixed::x", xpos,
				"fixed::y",
				-clutter_actor_get_height(actor)-g_random_int_range(0, 200),
				"y", CLUTTER_STAGE_HEIGHT(),
				"opacity", (guchar)0,
				NULL);
		clutter_animation_set_loop(anim, TRUE);
		xpos += clutter_actor_get_width(actor);
	}
	clutter_actor_show(stage);
	g_debug("entering clutter main loop.");
	clutter_main();
	return 0;
}
