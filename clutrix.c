#include <clutter/clutter.h>
#include <pango/pangocairo.h>

static gboolean windowed = 1;

static char *
set_pango_font(PangoLayout *layout)
{
	static char font[32];
	PangoFontDescription *desc;
	sprintf(font, "Monospace %dpx", g_random_int_range(6, 35));
	desc = pango_font_description_from_string(font);
	//pango_font_description_set_weight(desc, PANGO_WEIGHT_ULTRAHEAVY);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	return font;
}

static const char *
make_utf8(void)
{
#define NUM_CHARS 20
	static char buf[NUM_CHARS*7+1];
	unsigned i, pos;
	for (pos=0, i=0; i < NUM_CHARS; ++i) {
		gunichar u = g_random_int_range(0x30a1, 0x30fa);
		int len = g_unichar_to_utf8(u, &buf[pos]);
		buf[pos+len] = '\n';
		pos += len+1;
	}
	buf[pos-1] = 0;
	return buf;
}

static ClutterActor *
make_vertical_strip(void)
{
	fprintf(stderr, "%s()\n", __func__);
	static const ClutterColor last_color = { 255, 255, 255, 255 };
	const char *font;
	unsigned i, clones;
	int width, height;
	cairo_t *cr;
	cairo_surface_t *surface;
	PangoLayout *layout;
	ClutterActor *strip, *lastchar, *group;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 60, 2000);
	cr = cairo_create(surface);
	layout = pango_cairo_create_layout(cr);
	font = set_pango_font(layout);
	pango_layout_set_text(layout, make_utf8(), -1);
	cairo_set_source_rgb(cr, 0, 1.0, 0);
	pango_cairo_show_layout(cr, layout);
	pango_layout_get_size (layout, &width, &height);
	width = PANGO_PIXELS(width);
	height = PANGO_PIXELS(height);
	fprintf(stderr, " w=%d, h=%d\n", (width), (height));
	strip = clutter_texture_new();
	clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(strip),
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
	group = clutter_group_new();
	clutter_container_add_actor(CLUTTER_CONTAINER(group), strip);
	float ypos = clutter_actor_get_height(strip);
	clones = g_random_int_range(4, 8);
	for (i=0; i < clones; ++i) {
		ClutterActor *clone = clutter_clone_new(strip);
		clutter_container_add_actor(CLUTTER_CONTAINER(group), clone);
		clutter_actor_set_y(clone, ypos);
		ypos += clutter_actor_get_height(clone);
	}
	lastchar = clutter_text_new();
	clutter_text_set_font_name(CLUTTER_TEXT(lastchar), font);
	clutter_text_set_color(CLUTTER_TEXT(lastchar), &last_color);
	clutter_text_insert_unichar(CLUTTER_TEXT(lastchar),
			g_random_int_range(0x30a1, 0x30fa));
	clutter_container_add_actor(CLUTTER_CONTAINER(group), lastchar);
	clutter_actor_set_y(lastchar, ypos);
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
	fprintf(stderr, "*** %s(): entering main loop.\n", __func__);
	clutter_main();
	return 0;
}
