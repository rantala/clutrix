/* Clutrix
 *
 * Copyright (C) 2010 by Tommi Rantala <tt.rantala@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <clutter/clutter.h>
#include <pango/pangocairo.h>

static gboolean windowed = 0;

static char *
make_utf8(unsigned chars, unsigned font_size)
{
	unsigned i;
	GString *buf;
	buf = g_string_new("");
	g_string_printf(buf, "<span font=\"%upx\">", font_size);
	for (i=0; i < chars; ++i) {
		g_string_append_unichar(buf,
				g_random_int_range(0x30a1, 0x30df));
		g_string_append_c(buf, '\n');
	}
	g_string_erase(buf, buf->len-1, -1);
	g_string_append(buf, "</span>");
	return g_string_free(buf, FALSE);
}

static ClutterActor *
make_vertical_actor(unsigned chars, unsigned font_size, const ClutterColor *text_color)
{
	char *text;
	int width, height;
	cairo_t *cr;
	cairo_surface_t *surface;
	PangoLayout *layout;
	ClutterActor *actor;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			2*font_size, 3*font_size*chars);
	cr = cairo_create(surface);
	layout = pango_cairo_create_layout(cr);
	text = make_utf8(chars, font_size);
	pango_layout_set_markup(layout, text, -1);
	g_free(text);
	cairo_set_source_rgba(cr,
			text_color->red   / 255.0,
			text_color->green / 255.0,
			text_color->blue  / 255.0,
			text_color->alpha / 255.0);
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
			0,
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
	unsigned chars, font_size;
	chars = g_random_int_range(20, 60);
	font_size = g_random_int_range(20, 30);
	group = clutter_group_new();
	actor1 = make_vertical_actor(chars, font_size, &text_color);
	actor2 = make_vertical_actor(1, font_size, &last_color);
#if CLUTTER_CHECK_VERSION(1,0,0)
	clutter_texture_set_repeat(CLUTTER_TEXTURE(actor1), FALSE, TRUE);
	clutter_actor_set_size(actor1,
			clutter_actor_get_width(actor1),
			5*clutter_actor_get_height(actor1));
#endif
	clutter_container_add_actor(CLUTTER_CONTAINER(group), actor1);
	clutter_container_add_actor(CLUTTER_CONTAINER(group), actor2);
	clutter_actor_set_y(actor2, clutter_actor_get_height(actor1));
	g_debug("%s(): %u characters, actor size: [%2u, %4u]",
			__func__, chars+1,
			(unsigned)clutter_actor_get_width(group),
			(unsigned)clutter_actor_get_height(group));
	return group;
}

static gboolean
keypress_cb(ClutterActor *actor, ClutterEvent *event, void *data)
{
	(void)data; (void)actor;
	unsigned keysym = 0;
#if CLUTTER_CHECK_VERSION(1,0,0)
	keysym = clutter_event_get_key_symbol(event);
#elif CLUTTER_CHECK_VERSION(0,8,0)
	keysym = clutter_key_event_symbol((ClutterKeyEvent *)event);
#endif
	switch (keysym) {
	case CLUTTER_Escape: /* fall through */
	case CLUTTER_q:      /* fall through */
	case CLUTTER_Q:
		clutter_main_quit();
		break;
	default:
		break;
	}
	return TRUE;
}

int main(int argc, char **argv)
{
	static const ClutterColor stage_color = { 0, 0, 0, 255 };
	float xpos=0;
	if (clutter_init(&argc, &argv) < 0) {
		fprintf(stderr, "ERROR: clutter_init() failure.\n");
		return 1;
	}
	ClutterActor *stage = clutter_stage_get_default();
	if (!stage) {
		fprintf(stderr, "ERROR: unable to get clutter default stage.\n");
		return 1;
	}
	clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);
	if (windowed) {
		clutter_actor_set_size(stage, 640, 480);
	} else {
#if CLUTTER_CHECK_VERSION(1,0,0)
		clutter_stage_set_fullscreen(CLUTTER_STAGE(stage), TRUE);
#else
		clutter_stage_fullscreen(CLUTTER_STAGE(stage));
#endif
	}
	clutter_stage_hide_cursor(CLUTTER_STAGE(stage));
	clutter_stage_set_title(CLUTTER_STAGE(stage), "Clutrix");
	while (xpos < CLUTTER_STAGE_WIDTH()) {
		ClutterActor *actor = make_vertical_strip();
		clutter_stage_add(stage, actor);
		clutter_actor_set_position(actor, xpos, 0);
#if CLUTTER_CHECK_VERSION(1,0,0)
		ClutterAnimation *anim = clutter_actor_animate(actor,
				CLUTTER_EASE_IN_SINE,
				g_random_int_range(3000, 19000),
				"fixed::opacity", (guchar)255,
				"fixed::y",
				-clutter_actor_get_height(actor)-g_random_int_range(0, 400),
				"y", CLUTTER_STAGE_HEIGHT(),
				"opacity", (guchar)0,
				NULL);
		clutter_animation_set_loop(anim, TRUE);
#elif CLUTTER_CHECK_VERSION(0,8,0)
		ClutterTimeline *timeline;
		ClutterBehaviour *beh_opacity, *beh_path;
		ClutterAlpha *alpha;
		ClutterKnot knots[] = {
			{xpos, -clutter_actor_get_height(actor)-g_random_int_range(0, 400)},
			{xpos, CLUTTER_STAGE_HEIGHT()},
		};
		timeline = clutter_timeline_new_for_duration(g_random_int_range(3000, 19000));
		alpha = clutter_alpha_new_full(timeline,
				clutter_sine_inc_func, NULL, NULL);
		beh_opacity = clutter_behaviour_opacity_new(alpha, 255, 0);
		beh_path = clutter_behaviour_path_new(alpha, knots, 2);
		clutter_behaviour_apply(beh_opacity, actor);
		clutter_behaviour_apply(beh_path, actor);
		clutter_timeline_set_loop(timeline, TRUE);
		clutter_timeline_start(timeline);
#endif
		xpos += clutter_actor_get_width(actor);
	}
	g_signal_connect(stage, "key-press-event", G_CALLBACK(keypress_cb), NULL);
	clutter_actor_show(stage);
	g_debug("entering clutter main loop.");
	/* Lowering the frame rate seems to give somewhat smoother animation on
	 * N900. */
	clutter_set_default_frame_rate(30);
	clutter_main();
	return 0;
}
