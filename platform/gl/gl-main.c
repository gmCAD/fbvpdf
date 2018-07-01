#include "gl-app.h"

#include <SDL2/SDL.h>

#include "mupdf/pdf.h" /* for pdf specifics and forms */
#include "mupdf/ddi.h"

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define FL __FILE__,__LINE__
#ifndef _WIN32
#include <unistd.h> /* for fork and exec */
#endif

int windowx, windowy;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Surface *sdlSurface;
SDL_Texture *sdlTexture;
SDL_Event sdlEvent;
SDL_GLContext glcontext;

/* set the timer cycle rate, 50ms is more than fast enough! */
#define GLUT_TIMER_DURATION 50 
#ifndef FREEGLUT
/* freeglut extension no-ops */
//void glutExit(void) {}
//void glutMouseWheelFunc(void *fn) {}
//v/oid glutInitErrorFunc(void *fn) {}
//void glutInitWarningFunc(void *fn) {}
#endif

enum
{
	/* Screen furniture: aggregate size of unusable space from title bars, task bars, window borders, etc */
	SCREEN_FURNITURE_W = 20,
	SCREEN_FURNITURE_H = 40,

	/* Default EPUB/HTML layout dimensions */
	DEFAULT_LAYOUT_W = 450,
	DEFAULT_LAYOUT_H = 600,
	DEFAULT_LAYOUT_EM = 12,

	/* Default UI sizes */
	DEFAULT_UI_FONTSIZE = 15,
	DEFAULT_UI_BASELINE = 14,
	DEFAULT_UI_LINEHEIGHT = 18,
};

struct ui ui;
fz_context *ctx = NULL;
struct ddi_s ddi;

/* OpenGL capabilities */
static int has_ARB_texture_non_power_of_two = 1;
static GLint max_texture_size = 8192;

// Menu handling function declaration
void menucb(int mitem) {
}


static void ui_begin(void)
{
	ui.hot = NULL;
}

static void ui_end(void)
{
	if (!ui.down && !ui.middle && !ui.right)
		ui.active = NULL;
}

static void open_browser(const char *uri)
{
#ifdef _WIN32
	ShellExecuteA(NULL, "open", uri, 0, 0, SW_SHOWNORMAL);
#else
	const char *browser = getenv("BROWSER");
	if (!browser)
	{
#ifdef __APPLE__
		browser = "open";
#else
		browser = "xdg-open";
#endif
	}
	if (fork() == 0)
	{
		execlp(browser, browser, uri, (char*)0);
		fprintf(stderr, "cannot exec '%s'\n", browser);
		exit(0);
	}
#endif
}

const char *ogl_error_string(GLenum code)
{
#define CASE(E) case E: return #E; break
	switch (code)
	{
		/* glGetError */
		CASE(GL_NO_ERROR);
		CASE(GL_INVALID_ENUM);
		CASE(GL_INVALID_VALUE);
		CASE(GL_INVALID_OPERATION);
		CASE(GL_OUT_OF_MEMORY);
		CASE(GL_STACK_UNDERFLOW);
		CASE(GL_STACK_OVERFLOW);
		default: return "(unknown)";
	}
#undef CASE
}

void ogl_assert(fz_context *ctx, const char *msg)
{
	int code = glGetError();
	if (code != GL_NO_ERROR) {
		fz_warn(ctx, "glGetError(%s): %s", msg, ogl_error_string(code));
	}
}

void ui_draw_image(struct texture *tex, float x, float y)
{
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glColor4f(1, 1, 1, 1);
		glTexCoord2f(0, tex->t);
		glVertex2f(x + tex->x, y + tex->y + tex->h);
		glTexCoord2f(0, 0);
		glVertex2f(x + tex->x, y + tex->y);
		glTexCoord2f(tex->s, tex->t);
		glVertex2f(x + tex->x + tex->w, y + tex->y + tex->h);
		glTexCoord2f(tex->s, 0);
		glVertex2f(x + tex->x + tex->w, y + tex->y);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

static const int zoom_list[] = { 18, 24, 36, 54, 72, 96, 120, 144, 180, 216, 288, 350 };

/*
	static int zoom_in(int oldres)
	{
	int i;
	for (i = 0; i < nelem(zoom_list) - 1; ++i)
	if (zoom_list[i] <= oldres && zoom_list[i+1] > oldres)
	return zoom_list[i+1];
	return zoom_list[i];
	}

	static int zoom_out(int oldres)
	{
	int i;
	for (i = 0; i < nelem(zoom_list) - 1; ++i)
	if (zoom_list[i] < oldres && zoom_list[i+1] >= oldres)
	return zoom_list[i];
	return zoom_list[0];
	}
	*/

#define MINRES (zoom_list[0])
#define MAXRES (zoom_list[nelem(zoom_list)-1])
#define DEFRES 96

#define SEARCH_STATUS_NONE 0
#define SEARCH_STATUS_INPAGE 1
#define SEARCH_STATUS_SEEKING 2

#define DDI_SIMULATE_OPTION_NONE 0
#define DDI_SIMULATE_OPTION_SEARCH_NEXT 1
#define DDI_SIMULATE_OPTION_SEARCH_PREV 2

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define SEARCH_TYPE_NONE 0
#define SEARCH_TYPE_DDI_SEQUENCE 1
#define SEARCH_TYPE_TEXT_ONLY 2

static char filename[PATH_MAX];
static char *password = "";
static int raise_on_search = 0;
static int scroll_wheel_swap = 0;
//static int search_ended_flash_page = 0;
static int search_heuristics = 1;
static int search_in_page_only = 0;
static char *ddiprefix = "mupdf";
static char prior_search[1024] = "";
static int search_current_page = 1;
static int search_inpage_index = -1;
static int ddi_simulate_option = DDI_SIMULATE_OPTION_NONE;
static int search_type = SEARCH_TYPE_NONE;
static int document_has_hits = 0;
//static int search_status = SEARCH_STATUS_NONE;
static int am_dragging = 0;
static fz_point dragging_start;


static char *anchor = NULL;
static float layout_w = DEFAULT_LAYOUT_W;
static float layout_h = DEFAULT_LAYOUT_H;
static float layout_em = DEFAULT_LAYOUT_EM;
static char *layout_css = NULL;
static int layout_use_doc_css = 1;

static const char *title = "FlexBV MuPDF/GL";
static int search_not_found = 0;
static char last_search_string[256] = "";
static fz_document *doc = NULL;
static fz_page *page = NULL;
static fz_stext_page *text = NULL;
static pdf_document *pdf = NULL;
static fz_outline *outline = NULL;
static fz_link *links = NULL;

static int number = 0;
static int show_help = 0;

static struct texture page_tex = { 0 };
static int scroll_x = 0, scroll_y = 0;
static int canvas_x = 0, canvas_w = 100;
static int canvas_y = 0, canvas_h = 100;

static struct texture annot_tex[256];
static int annot_count = 0;

//static int window_w = 1, window_h = 1;
static int window_w = 1, window_h = 1;

static int debug = 0;
static time_t process_start_time;
static int oldinvert = 0, currentinvert = 0;
static int oldpage = 0, currentpage = 0;
static float oldzoom = DEFRES, currentzoom = DEFRES;
static float oldrotate = 0, currentrotate = 0;
static fz_matrix page_ctm, page_inv_ctm;
static int loaded = 0;
//static int window = 0;
#ifdef __WIN32__
//HWND hwnd;
#endif

static int isfullscreen = 0;
static int showoutline = 0;
static int showlinks = 0;
static int showsearch = 0;
static int showinfo = 0;
static int showhelp = 0;
static int doquit = 0;

struct mark
{
	int page;
	fz_point scroll;
};

static int history_count = 0;
static struct mark history[256];
static int future_count = 0;
static struct mark future[256];
static struct mark marks[10];

static int search_active = 0;
static struct input search_input = { { 0 }, 0 };
static char search_string[10240];
static char *search_needle = 0;
static int needle_has_hits = 0;
static int search_dir = 1;
static int search_page = 0;
static int search_hit_page = -1;
static int search_hit_count = 0;
static int search_compound = 0;
static fz_rect search_hit_bbox[5000];

static unsigned int next_power_of_two(unsigned int n)
{
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return ++n;
}

static void update_title(void)
{
	static char buf[256];
	size_t n = strlen(title);

	if (search_not_found) {
		if (n > 50) snprintf(buf, sizeof(buf),"'%s' not found - ...%s (R%d)", last_search_string, title +n -50, GIT_BUILD);
		else snprintf(buf, sizeof(buf),"'%s' not found - %s (R%d)", last_search_string, title, GIT_BUILD);
		if (debug) fprintf(stderr,"%s:%d: Search not found: '%s'\r\n", FL, last_search_string);
	} else {
		if (n > 50) {
			sprintf(buf, "...%s - %d / %d (R%d)", title + n - 50, currentpage + 1, fz_count_pages(ctx, doc), GIT_BUILD);
		} else{
			sprintf(buf, "%s - %d / %d (R%d)", title, currentpage + 1, fz_count_pages(ctx, doc), GIT_BUILD);
		}
	}
	if (debug) fprintf(stderr,"%s:%d: Setting window title '%s'\r\n", FL, buf);
	SDL_SetWindowTitle( sdlWindow, buf );
}

void texture_from_pixmap(struct texture *tex, fz_pixmap *pix)
{
	if (!tex->id) glGenTextures(1, &tex->id);

	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	tex->x = pix->x;
	tex->y = pix->y;
	tex->w = pix->w;
	tex->h = pix->h;

	if (has_ARB_texture_non_power_of_two)
	{
		if (tex->w > max_texture_size || tex->h > max_texture_size)
			fz_warn(ctx, "texture size (%d x %d) exceeds implementation limit (%d)", tex->w, tex->h, max_texture_size);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, pix->n == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pix->samples);
		tex->s = 1;
		tex->t = 1;
	}
	else
	{
		int w2 = next_power_of_two(tex->w);
		int h2 = next_power_of_two(tex->h);
		if (w2 > max_texture_size || h2 > max_texture_size)
			fz_warn(ctx, "texture size (%d x %d) exceeds implementation limit (%d)", w2, h2, max_texture_size);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w, tex->h, pix->n == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pix->samples);
		tex->s = (float) tex->w / w2;
		tex->t = (float) tex->h / h2;
	}
}

void load_page(void)
{
	fz_rect rect;
	fz_irect irect;

	fz_scale(&page_ctm, currentzoom / 72, currentzoom / 72);
	fz_pre_rotate(&page_ctm, -currentrotate);
	fz_invert_matrix(&page_inv_ctm, &page_ctm);

	fz_drop_stext_page(ctx, text);
	text = NULL;
	fz_drop_link(ctx, links);
	links = NULL;
	fz_drop_page(ctx, page);
	page = NULL;

	page = fz_load_page(ctx, doc, currentpage);
	links = fz_load_links(ctx, page);
	text = fz_new_stext_page_from_page(ctx, page, NULL);

	/* compute bounds here for initial window size */
	fz_bound_page(ctx, page, &rect);
	fz_transform_rect(&rect, &page_ctm);
	fz_round_rect(&irect, &rect);
	page_tex.w = irect.x1 - irect.x0;
	page_tex.h = irect.y1 - irect.y0;

	loaded = 1;
}

void render_page(void)
{
	fz_annot *annot;
	fz_pixmap *pix;

	if (!loaded)
		load_page();

	pix = fz_new_pixmap_from_page_contents(ctx, page, &page_ctm, fz_device_rgb(ctx), 0);
	if (currentinvert)
	{
		fz_invert_pixmap(ctx, pix);
		fz_gamma_pixmap(ctx, pix, 1 / 1.4f);
	}

	texture_from_pixmap(&page_tex, pix);
	fz_drop_pixmap(ctx, pix);

	/*
		annot_count = 0;
		for (annot = fz_first_annot(ctx, page); annot; annot = fz_next_annot(ctx, annot))
		{
		pix = fz_new_pixmap_from_annot(ctx, annot, &page_ctm, fz_device_rgb(ctx), 1);
		texture_from_pixmap(&annot_tex[annot_count++], pix);
		fz_drop_pixmap(ctx, pix);
		if (annot_count >= nelem(annot_tex))
		{
		fz_warn(ctx, "too many annotations to display!");
		break;
		}
		}
		*/

	loaded = 0;
}

static struct mark save_mark()
{
	struct mark mark;
	mark.page = currentpage;
	mark.scroll.x = scroll_x;
	mark.scroll.y = scroll_y;
	fz_transform_point(&mark.scroll, &page_inv_ctm);
	return mark;
}

static void restore_mark(struct mark mark)
{
	currentpage = mark.page;
	fz_transform_point(&mark.scroll, &page_ctm);
	scroll_x = mark.scroll.x;
	scroll_y = mark.scroll.y;
}

static void push_history(void)
{
	if (history_count + 1 >= nelem(history))
	{
		memmove(history, history + 1, sizeof *history * (nelem(history) - 1));
		history[history_count] = save_mark();
	}
	else
	{
		history[history_count++] = save_mark();
	}
}

static void push_future(void)
{
	if (future_count + 1 >= nelem(future))
	{
		memmove(future, future + 1, sizeof *future * (nelem(future) - 1));
		future[future_count] = save_mark();
	}
	else
	{
		future[future_count++] = save_mark();
	}
}

static void clear_future(void)
{
	future_count = 0;
}

static void jump_to_page(int newpage)
{
	newpage = fz_clampi(newpage, 0, fz_count_pages(ctx, doc) - 1);
	clear_future();
	push_history();
	currentpage = newpage;
	push_history();
}

static void jump_to_page_xy(int newpage, float x, float y)
{
	fz_point p = { x, y };
	newpage = fz_clampi(newpage, 0, fz_count_pages(ctx, doc) - 1);
	fz_transform_point(&p, &page_ctm);
	clear_future();
	push_history();
	currentpage = newpage;
	scroll_x = p.x;
	scroll_y = p.y;
	push_history();
}

static void pop_history(void)
{
	int here = currentpage;
	push_future();
	while (history_count > 0 && currentpage == here)
		restore_mark(history[--history_count]);
}

static void pop_future(void)
{
	int here = currentpage;
	push_history();
	while (future_count > 0 && currentpage == here)
		restore_mark(future[--future_count]);
	push_history();
}

static void ui_label_draw(int x0, int y0, int x1, int y1, const char *text)
{
	glColor4f(1, 1, 1, 1);
	glRectf(x0, y0, x1, y1);
	glColor4f(0, 0, 0, 1);
	ui_draw_string(ctx, x0 + 2, y0 + 2 + ui.baseline, text);
}

static void ui_scrollbar(int x0, int y0, int x1, int y1, int *value, int page_size, int max)
{
	static float saved_top = 0;
	static int saved_ui_y = 0;
	float top;

	int total_h = y1 - y0;
	int thumb_h = fz_maxi(x1 - x0, total_h * page_size / max);
	int avail_h = total_h - thumb_h;

	max -= page_size;

	if (max <= 0)
	{
		*value = 0;
		glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
		glRectf(x0, y0, x1, y1);
		return;
	}

	top = (float) *value * avail_h / max;

	if (ui.down && !ui.active)
	{
		if (ui.x >= x0 && ui.x < x1 && ui.y >= y0 && ui.y < y1)
		{
			if (ui.y < top)
			{
				ui.active = "pgdn";
				*value -= page_size;
			}
			else if (ui.y >= top + thumb_h)
			{
				ui.active = "pgup";
				*value += page_size;
			}
			else
			{
				ui.hot = value;
				ui.active = value;
				saved_top = top;
				saved_ui_y = ui.y;
			}
		}
	}

	if (ui.active == value)
	{
		*value = (saved_top + ui.y - saved_ui_y) * max / avail_h;
	}

	if (*value < 0)
		*value = 0;
	else if (*value > max)
		*value = max;

	top = (float) *value * avail_h / max;

	glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	glRectf(x0, y0, x1, y1);
	glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	glRectf(x0, top, x1, top + thumb_h);
}

static int measure_outline_height(fz_outline *node)
{
	int h = 0;
	while (node)
	{
		h += ui.lineheight;
		if (node->down)
			h += measure_outline_height(node->down);
		node = node->next;
	}
	return h;
}

static int do_outline_imp(fz_outline *node, int end, int x0, int x1, int x, int y)
{
	int h = 0;
	int p = currentpage;
	int n = end;

	while (node)
	{
		p = node->page;
		if (p >= 0)
		{
			if (ui.x >= x0 && ui.x < x1 && ui.y >= y + h && ui.y < y + h + ui.lineheight)
			{
				ui.hot = node;
				if (!ui.active && ui.down)
				{
					ui.active = node;
					jump_to_page_xy(p, node->x, node->y);
					//glutPostRedisplay(); /* we changed the current page, so force a redraw */
				}
			}

			n = end;
			if (node->next && node->next->page >= 0)
			{
				n = node->next->page;
			}
			if (currentpage == p || (currentpage > p && currentpage < n))
			{
				glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
				glRectf(x0, y + h, x1, y + h + ui.lineheight);
			}
		}

		glColor4f(0, 0, 0, 1);
		ui_draw_string(ctx, x, y + h + ui.baseline, node->title);
		h += ui.lineheight;
		if (node->down)
			h += do_outline_imp(node->down, n, x0, x1, x + ui.lineheight, y + h);

		node = node->next;
	}
	return h;
}

static void do_outline(fz_outline *node, int outline_w)
{
	static char *id = "outline";
	static int outline_scroll_y = 0;
	static int saved_outline_scroll_y = 0;
	static int saved_ui_y = 0;

	int outline_h;
	int total_h;

	outline_w -= ui.lineheight;
	outline_h = window_h;
	total_h = measure_outline_height(outline);

	if (ui.x >= 0 && ui.x < outline_w && ui.y >= 0 && ui.y < outline_h)
	{
		ui.hot = id;
		if (!ui.active && ui.middle)
		{
			ui.active = id;
			saved_ui_y = ui.y;
			saved_outline_scroll_y = outline_scroll_y;
		}
	}

	if (ui.active == id)
		outline_scroll_y = saved_outline_scroll_y + (saved_ui_y - ui.y) * 5;

	if (ui.hot == id)
		outline_scroll_y -= ui.scroll_y * ui.lineheight * 3;

	ui_scrollbar(outline_w, 0, outline_w+ui.lineheight, outline_h, &outline_scroll_y, outline_h, total_h);

	glScissor(0, 0, outline_w, outline_h);
	glEnable(GL_SCISSOR_TEST);

	glColor4f(1, 1, 1, 1);
	glRectf(0, 0, outline_w, outline_h);

	do_outline_imp(outline, fz_count_pages(ctx, doc), 0, outline_w, 10, -outline_scroll_y);

	glDisable(GL_SCISSOR_TEST);
}

static void do_links(fz_link *link, int xofs, int yofs)
{
	fz_rect r;
	float x, y;
	float link_x, link_y;

	x = ui.x;
	y = ui.y;

	xofs -= page_tex.x;
	yofs -= page_tex.y;

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	while (link)
	{
		r = link->rect;
		fz_transform_rect(&r, &page_ctm);

		if (x >= xofs + r.x0 && x < xofs + r.x1 && y >= yofs + r.y0 && y < yofs + r.y1)
		{
			ui.hot = link;
			if (!ui.active && ui.down)
				ui.active = link;
		}

		if (ui.hot == link || showlinks)
		{
			if (ui.active == link && ui.hot == link)
				glColor4f(0, 0, 1, 0.4f);
			else if (ui.hot == link)
				glColor4f(0, 0, 1, 0.2f);
			else
				glColor4f(0, 0, 1, 0.1f);
			glRectf(xofs + r.x0, yofs + r.y0, xofs + r.x1, yofs + r.y1);
		}

		if (ui.active == link && !ui.down)
		{
			if (ui.hot == link)
			{
				if (fz_is_external_link(ctx, link->uri))
					open_browser(link->uri);
				else
				{
					int p = fz_resolve_link(ctx, doc, link->uri, &link_x, &link_y);
					if (p >= 0)
						jump_to_page_xy(p, link_x, link_y);
					else
						fz_warn(ctx, "cannot find link destination '%s'", link->uri);
					//glutPostRedisplay(); /* we changed the current page, so force a redraw */
				}
			}
		}

		link = link->next;
	}

	glDisable(GL_BLEND);
}

static void do_page_selection(int x0, int y0, int x1, int y1)
{
	static fz_point pt = { 0, 0 };
	fz_rect hits[1000];
	int i, n;

	if (ui.x >= x0 && ui.x < x1 && ui.y >= y0 && ui.y < y1)
	{
		ui.hot = &pt;
		if (!ui.active && ui.right)
		{
			ui.active = &pt;
			pt.x = ui.x;
			pt.y = ui.y;
		}
	}

	if (ui.active == &pt)
	{
		int xofs = x0 - page_tex.x;
		int yofs = y0 - page_tex.y;

		fz_point page_a = { pt.x - xofs, pt.y - yofs };
		fz_point page_b = { ui.x - xofs, ui.y - yofs };

		fz_transform_point(&page_a, &page_inv_ctm);
		fz_transform_point(&page_b, &page_inv_ctm);

		n = fz_highlight_selection(ctx, text, page_a, page_b, hits, nelem(hits));

		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO); /* invert destination color */
		glEnable(GL_BLEND);

		glColor4f(1, 1, 1, 1);
		for (i = 0; i < n; ++i)
		{
			fz_transform_rect(&hits[i], &page_ctm);
			glRectf(hits[i].x0+xofs, hits[i].y0+yofs, hits[i].x1 + 1 + xofs, hits[i].y1 + 1 + yofs);
		}

		glDisable(GL_BLEND);

		if (!ui.right)
		{
			char *s;
#ifdef _WIN32
			s = fz_copy_selection(ctx, text, page_a, page_b, 1);
#else
			s = fz_copy_selection(ctx, text, page_a, page_b, 0);
#endif
			ui_set_clipboard(s);
			if (debug) fprintf(stderr,"%s:%d: Dispatching request '%s'\n", FL, s);
			DDI_dispatch( &ddi, s );
			fz_free(ctx, s);
			//glutPostRedisplay();
		}
	}
}

static void do_search_hits(int xofs, int yofs)
{
	fz_rect r;
	int i;

	xofs -= page_tex.x;
	yofs -= page_tex.y;

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	for (i = 0; i < search_hit_count; ++i)
	{
		r = search_hit_bbox[i];

		fz_transform_rect(&r, &page_ctm);

		if (i == search_inpage_index) {
			glColor4f(1, 0, 0, 0.4f);
		} else {
			glColor4f(1, 0, 0, 0.1f);
		}
		glRectf(xofs + r.x0, yofs + r.y0, xofs + r.x1, yofs + r.y1);
	}

	glDisable(GL_BLEND);
}

static void do_forms(float xofs, float yofs)
{
	static int do_forms_tag = 0;
	pdf_ui_event event;
	fz_point p;
	int i;

	for (i = 0; i < annot_count; ++i)
		ui_draw_image(&annot_tex[i], xofs - page_tex.x, yofs - page_tex.y);

	if (!pdf || search_active)
		return;

	p.x = xofs - page_tex.x + ui.x;
	p.y = xofs - page_tex.x + ui.y;
	fz_transform_point(&p, &page_inv_ctm);

	if (ui.down && !ui.active)
	{
		event.etype = PDF_EVENT_TYPE_POINTER;
		event.event.pointer.pt = p;
		event.event.pointer.ptype = PDF_POINTER_DOWN;
		if (pdf_pass_event(ctx, pdf, (pdf_page*)page, &event))
		{
			if (pdf->focus)
				ui.active = &do_forms_tag;
			pdf_update_page(ctx, (pdf_page*)page);
			render_page();
			//glutPostRedisplay();
		}
	}
	else if (ui.active == &do_forms_tag && !ui.down)
	{
		ui.active = NULL;
		event.etype = PDF_EVENT_TYPE_POINTER;
		event.event.pointer.pt = p;
		event.event.pointer.ptype = PDF_POINTER_UP;
		if (pdf_pass_event(ctx, pdf, (pdf_page*)page, &event))
		{
			pdf_update_page(ctx, (pdf_page*)page);
			render_page();
			//glutPostRedisplay();
		}
	}
}

static void toggle_fullscreen(void)
{
	static int win_x = 0, win_y = 0;
	static int win_w = 100, win_h = 100;
	if (!isfullscreen)
	{
		SDL_GetWindowSize( sdlWindow, &win_w, &win_h );
		SDL_GetWindowPosition( sdlWindow, &win_x, &win_y );
		SDL_SetWindowFullscreen( sdlWindow, 0 );
		//		win_w = glutGet(GLUT_WINDOW_WIDTH);
		//		win_h = glutGet(GLUT_WINDOW_HEIGHT);
		//		win_x = glutGet(GLUT_WINDOW_X);
		//		win_y = glutGet(GLUT_WINDOW_Y);
		//		glutFullScreen();
		isfullscreen = 1;
	} else {
		SDL_SetWindowPosition( sdlWindow, win_x, win_y );
		SDL_SetWindowSize( sdlWindow, win_w, win_h );
		//		glutPositionWindow(win_x, win_y);
		//		glutReshapeWindow(win_w, win_h);
		isfullscreen = 0;
	}
}

static void shrinkwrap(void)
{
	int x, y;
	int screen_w, screen_h;
	int w, h;

	SDL_GetWindowSize( sdlWindow, &x, &y );
	screen_w = x - SCREEN_FURNITURE_W;
	screen_h = y - SCREEN_FURNITURE_H;
	w = page_tex.w + canvas_x;
	h = page_tex.h + canvas_y;

	if (screen_w > 0 && w > screen_w) w = screen_w;
	if (screen_h > 0 && h > screen_h) h = screen_h;
	if (isfullscreen) toggle_fullscreen();
	//glutReshapeWindow(w, h);
	SDL_SetWindowSize( sdlWindow, w, h);
}

static void load_document(void)
{
	fz_drop_outline(ctx, outline);
	fz_drop_document(ctx, doc);

	doc = fz_open_document(ctx, filename);
	if (fz_needs_password(ctx, doc))
	{
		if (!fz_authenticate_password(ctx, doc, password))
		{
			fprintf(stderr, "Invalid password.\n");
			exit(1);
		}
	}

	fz_layout_document(ctx, doc, layout_w, layout_h, layout_em);

	fz_try(ctx)
		outline = fz_load_outline(ctx, doc);
	fz_catch(ctx)
		outline = NULL;

	pdf = pdf_specifics(ctx, doc);
	if (pdf)
	{
		pdf_enable_js(ctx, pdf);
		if (anchor)
			currentpage = pdf_lookup_anchor(ctx, pdf, anchor, NULL, NULL);
	}
	else
	{
		if (anchor)
			currentpage = fz_atoi(anchor) - 1;
	}
	anchor = NULL;

	currentpage = fz_clampi(currentpage, 0, fz_count_pages(ctx, doc) - 1);
}

static void reload(void)
{
	load_document();
	render_page();
	update_title();
}

static void toggle_outline(void)
{
	if (outline)
	{
		showoutline = !showoutline;
		if (showoutline)
			canvas_x = ui.lineheight * 16;
		else
			canvas_x = 0;
		if (canvas_w == page_tex.w && canvas_h == page_tex.h)
			shrinkwrap();
	}
}

static void auto_zoom_w(void)
{
	scroll_x = scroll_y = 0;
	currentzoom = fz_clamp(currentzoom * canvas_w / page_tex.w, MINRES, MAXRES);
}

static void auto_zoom_h(void)
{
	scroll_x = scroll_y = 0;
	currentzoom = fz_clamp(currentzoom * canvas_h / page_tex.h, MINRES, MAXRES);
}

static void auto_zoom(void)
{
	float page_a = (float) page_tex.w / page_tex.h;
	float screen_a = (float) canvas_w / canvas_h;
	if (page_a > screen_a)
		auto_zoom_w();
	else
		auto_zoom_h();
}

static void smart_move_backward(void)
{
	if (scroll_y <= 0)
	{
		if (scroll_x <= 0)
		{
			if (currentpage - 1 >= 0)
			{
				scroll_x = page_tex.w;
				scroll_y = page_tex.h;
				currentpage -= 1;
			}
		}
		else
		{
			scroll_y = page_tex.h;
			scroll_x -= canvas_w * 9 / 10;
		}
	}
	else
	{
		scroll_y -= canvas_h * 9 / 10;
	}
}

static void smart_move_forward(void)
{
	if (scroll_y + canvas_h >= page_tex.h)
	{
		if (scroll_x + canvas_w >= page_tex.w)
		{
			if (currentpage + 1 < fz_count_pages(ctx, doc))
			{
				scroll_x = 0;
				scroll_y = 0;
				currentpage += 1;
			}
		}
		else
		{
			scroll_y = 0;
			scroll_x += canvas_w * 9 / 10;
		}
	}
	else
	{
		scroll_y += canvas_h * 9 / 10;
	}
}

static void quit(void)
{
	doquit = 1;
}

static void clear_search(void)
{
	search_hit_page = -1;
	search_hit_count = 0;
}

static void do_keypress(void)
{

	ui.plain = 1;
	if (debug) fprintf(stderr,"%s:%d key = %02x '%c' [ focus:%d, plain:%d]\r\n",FL, ui.key, ui.key, ui.focus, ui.plain );

	/*
	 * close the help/info if we've pressed something else 
	 *
	 */
	if (ui.down || ui.middle || ui.right || ui.key)
		showinfo = showhelp = 0;

	if (!ui.focus && ui.key && ui.plain)
	{
		if (debug) fprintf(stderr,"%s:%d: Acting on key '%c'\r\n", FL, ui.key );
		switch (ui.key)
		{
			case SDLK_ESCAPE: clear_search(); break;
			case SDLK_F1: showhelp = !showhelp; break;
			case 'o': toggle_outline(); break;
			case 'L': showlinks = !showlinks; break;
			case 'i': showinfo = !showinfo; break;
			case 'r': reload(); break;
			case 'q': quit(); break;

			case 'I': currentinvert = !currentinvert; break;
			case 'f':
						 fprintf(stderr,"%s:%d: Full screen\r\n",FL);
						 toggle_fullscreen(); break;
			case 'W': 
						 fprintf(stderr,"%s:%d: Shrinkwrapping\r\n",FL);
						 shrinkwrap(); break;
			case 'w': auto_zoom_w(); break;
			case 'h': auto_zoom_h(); break;
			case 'z': currentzoom = number > 0 ? number : DEFRES; break;
						 //			case '+': currentzoom = zoom_in(currentzoom); break;
						 //`			case '-': currentzoom = zoom_out(currentzoom); break;
			case '=': currentzoom *= 1.25; break;
			case '-': currentzoom /= 1.25; break;
			case '[': currentrotate += 90; break;
			case ']': currentrotate -= 90; break;
						 //			case 'k': case KEY_UP: scroll_y -= 10; break;
						 //			case 'j': case KEY_DOWN: scroll_y += 10; break;
						 //			case 'h': case KEY_LEFT: scroll_x -= 10; break;
						 //			case 'l': case KEY_RIGHT: scroll_x += 10; break;

			case 'b': number = fz_maxi(number, 1); while (number--) smart_move_backward(); break;
			case ' ': number = fz_maxi(number, 1); while (number--) smart_move_forward(); break;
			case ',': case KEY_PAGE_UP: case SDLK_PAGEUP: currentpage -= fz_maxi(number, 1); break;
			case '.': case KEY_PAGE_DOWN: case SDLK_PAGEDOWN: currentpage += fz_maxi(number, 1); break;
			case '<': currentpage -= 10 * fz_maxi(number, 1); break;
			case '>': currentpage += 10 * fz_maxi(number, 1); break;
			case 'g': jump_to_page(number - 1); break;
			case 'G': jump_to_page(fz_count_pages(ctx, doc) - 1); break;

			case 'm':
						 if (number == 0)
							 push_history();
						 else if (number > 0 && number < nelem(marks))
							 marks[number] = save_mark();
						 break;
			case 't':
						 if (number == 0)
						 {
							 if (history_count > 0)
								 pop_history();
						 }
						 else if (number > 0 && number < nelem(marks))
						 {
							 struct mark mark = marks[number];
							 restore_mark(mark);
							 jump_to_page(mark.page);
						 }
						 break;
			case 'T':
						 if (number == 0)
						 {
							 if (future_count > 0)
								 pop_future();
						 }
						 break;

			case '/':
						 clear_search();
						 search_dir = 1;
						 showsearch = 1;
						 search_input.p = search_input.text;
						 search_input.q = search_input.end;
						 break;
			case '?':
						 clear_search();
						 search_dir = -1;
						 showsearch = 1;
						 search_input.p = search_input.text;
						 search_input.q = search_input.end;
						 break;
			case 'N':
						 search_dir = -1;
						 if (search_hit_page == currentpage)
							 search_page = currentpage + search_dir;
						 else
							 search_page = currentpage;
						 if (search_page >= 0 && search_page < fz_count_pages(ctx, doc))
						 {
							 search_hit_page = -1;
							 if (search_needle)
								 search_active = 1;
						 }
						 //glutPostRedisplay();
						 break;
			case 'n':
						 if (debug) fprintf(stderr,"%s:%d: NEXT search pressed\r\n",FL);
						 if (!search_needle) {
							 if (debug) fprintf(stderr,"%s:%d: Needle is NULL\r\n",FL);

							 if (strlen(prior_search)) {
								 if (debug) fprintf(stderr,"%s:%d: Prior exists, setting  needle to prior\r\n",FL);
								 search_needle = prior_search;
							 } else {
								 if (debug) fprintf(stderr,"%s:%d: Search needle is NULL, and prior search is empty. Ignoring next search request\r\n", FL);
							 }

							 break;
						 }  else {
							 if (debug) fprintf(stderr,"%s:%d: Needle is '%s'\r\n",FL, search_needle);
						 }


						 //						 if ((search_current_page >= 0)&&(search_needle)) {
						 //							 ddi_simulate_option = DDI_SIMULATE_OPTION_SEARCH_NEXT;
						 //						 } else {

						 if (strlen(search_needle)) {
							 search_dir = 1;
							 if (search_hit_page == currentpage)
								 search_page = currentpage + search_dir;
							 else
								 search_page = currentpage;
							 if (search_page >= 0 && search_page < fz_count_pages(ctx, doc))
							 {
								 search_hit_page = -1;
								 if (search_needle)
									 search_active = 1;
							 }
							 //glutPostRedisplay();
						 }
						 //						 }
						 break;
		}

		if (ui.key >= '0' && ui.key <= '9')
			number = number * 10 + ui.key - '0';
		else
			number = 0;

		currentpage = fz_clampi(currentpage, 0, fz_count_pages(ctx, doc) - 1);
		currentzoom = fz_clamp(currentzoom, MINRES, MAXRES);
		while (currentrotate < 0) currentrotate += 360;
		while (currentrotate >= 360) currentrotate -= 360;

		if (search_hit_page != currentpage)
			search_hit_page = -1; /* clear highlights when navigating */

		ui.key = 0; /* we ate the key event, so zap it */
	}
}

static int do_info_line(int x, int y, char *label, char *text)
{
	char buf[512];
	fz_snprintf(buf, sizeof buf, "%s: %s", label, text);
	ui_draw_string(ctx, x, y, buf);
	return y + ui.lineheight;
}

static void do_info(void)
{
	char buf[256];

	int x = canvas_x + 4 * ui.lineheight;
	int y = canvas_y + 4 * ui.lineheight;
	int w = canvas_w - 8 * ui.lineheight;
	int h = 9 * ui.lineheight;

	glBegin(GL_TRIANGLE_STRIP);
	{
		glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
		glVertex2f(x, y);
		glVertex2f(x, y + h);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
	}
	glEnd();

	x += ui.lineheight;
	y += ui.lineheight + ui.baseline;

	glColor4f(0, 0, 0, 1);
	if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_TITLE, buf, sizeof buf) > 0)
		y = do_info_line(x, y, "Title", buf);
	if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_AUTHOR, buf, sizeof buf) > 0)
		y = do_info_line(x, y, "Author", buf);
	if (fz_lookup_metadata(ctx, doc, FZ_META_FORMAT, buf, sizeof buf) > 0)
		y = do_info_line(x, y, "Format", buf);
	if (fz_lookup_metadata(ctx, doc, FZ_META_ENCRYPTION, buf, sizeof buf) > 0)
		y = do_info_line(x, y, "Encryption", buf);
	if (pdf_specifics(ctx, doc))
	{
		if (fz_lookup_metadata(ctx, doc, "info:Creator", buf, sizeof buf) > 0)
			y = do_info_line(x, y, "PDF Creator", buf);
		if (fz_lookup_metadata(ctx, doc, "info:Producer", buf, sizeof buf) > 0)
			y = do_info_line(x, y, "PDF Producer", buf);
		buf[0] = 0;
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_PRINT))
			fz_strlcat(buf, "print, ", sizeof buf);
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_COPY))
			fz_strlcat(buf, "copy, ", sizeof buf);
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_EDIT))
			fz_strlcat(buf, "edit, ", sizeof buf);
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_ANNOTATE))
			fz_strlcat(buf, "annotate, ", sizeof buf);
		if (strlen(buf) > 2)
			buf[strlen(buf)-2] = 0;
		else
			fz_strlcat(buf, "none", sizeof buf);
		y = do_info_line(x, y, "Permissions", buf);
	}
}

static int do_help_line(int x, int y, char *label, char *text)
{
	ui_draw_string(ctx, x, y, label);
	ui_draw_string(ctx, x+100, y, text);
	return y + ui.lineheight;
}

static void do_help(void)
{
	float x = canvas_x + 4 * ui.lineheight;
	float y = canvas_y + 4 * ui.lineheight;
	float w = canvas_w - 8 * ui.lineheight;
	float h = 38 * ui.lineheight;

	glBegin(GL_TRIANGLE_STRIP);
	{
		glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
		glVertex2f(x, y);
		glVertex2f(x, y + h);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
	}
	glEnd();


	x += ui.lineheight;
	y += ui.lineheight + ui.baseline;

	glColor4f(0, 0, 0, 1);
	y = do_help_line(x, y, "FlexBV-MuPDF", FZ_VERSION);

	y += ui.lineheight;
	y = do_help_line(x, y, "F1", "show this message");
	y = do_help_line(x, y, "i", "show document information");
	y = do_help_line(x, y, "o", "show/hide outline");
	y = do_help_line(x, y, "L", "show/hide links");
	y = do_help_line(x, y, "r", "reload file");
	y = do_help_line(x, y, "q", "quit");
		y += ui.lineheight;
		y = do_help_line(x, y, "I", "toggle inverted color mode");
		y = do_help_line(x, y, "f", "fullscreen window");
		y = do_help_line(x, y, "W", "shrink wrap window");
		y = do_help_line(x, y, "w or h", "fit to width or height");
		y = do_help_line(x, y, "z", "fit to window");
		y = do_help_line(x, y, "N z", "set zoom to N");
	//	y = do_help_line(x, y, "+ or -", "zoom in or out");
	//	y = do_help_line(x, y, "[ or ]", "rotate left or right");
	//	y = do_help_line(x, y, "arrow keys", "pan in small increments");
	y += ui.lineheight;
	//	y = do_help_line(x, y, "b", "smart move backward");
	//	y = do_help_line(x, y, "Space", "smart move forward");
	y = do_help_line(x, y, ", or PgUp", "go backward");
	y = do_help_line(x, y, ". or PgDn", "go forward");
	y = do_help_line(x, y, "<", "go backward 10 pages");
	y = do_help_line(x, y, ">", "go forward 10 pages");
	y = do_help_line(x, y, "N g", "go to page N");
	y = do_help_line(x, y, "G", "go to last page");
	y += ui.lineheight;
	y = do_help_line(x, y, "t", "go backward in history");
	y = do_help_line(x, y, "T", "go forward in history");
	y = do_help_line(x, y, "N m", "save location in bookmark N");
	y = do_help_line(x, y, "N t", "go to bookmark N");
	y += ui.lineheight;
	y = do_help_line(x, y, "/ or ?", "search for text");
	y = do_help_line(x, y, "n or N", "repeat search");
}

static void do_canvas(void)
{
	//	static int saved_scroll_x = 0;
	//	static int saved_scroll_y = 0;
	//	static int saved_ui_x = 0;
	//	static int saved_ui_y = 0;

	float x, y;

	if (oldpage != currentpage || oldzoom != currentzoom || oldrotate != currentrotate || oldinvert != currentinvert)
	{
		render_page();
		update_title();
		oldpage = currentpage;
		oldzoom = currentzoom;
		oldrotate = currentrotate;
		oldinvert = currentinvert;
	}

	/*
		if (ui.x >= canvas_x && ui.x < canvas_x + canvas_w && ui.y >= canvas_y && ui.y < canvas_y + canvas_h)
		{
		ui.hot = doc;
		if (!ui.active && ui.middle)
		{
		ui.active = doc;
		saved_scroll_x = scroll_x;
		saved_scroll_y = scroll_y;
		saved_ui_x = ui.x;
		saved_ui_y = ui.y;
		}
		}

		if (ui.hot == doc)
		{
		scroll_x -= ui.scroll_x * ui.lineheight * 3;
		scroll_y -= ui.scroll_y * ui.lineheight * 3;
		}

		if (ui.active == doc)
		{
		scroll_x = saved_scroll_x + saved_ui_x - ui.x;
		scroll_y = saved_scroll_y + saved_ui_y - ui.y;
		}
		*/

	x = canvas_x -scroll_x;
	y = canvas_y -scroll_y;

	/*
		if (page_tex.w <= canvas_w)
		{
		scroll_x = 0;
		x = canvas_x + (canvas_w - page_tex.w) / 2;
		}
		else
		{
	//		scroll_x = fz_clamp(scroll_x, 0, page_tex.w - canvas_w);
	x = canvas_x - scroll_x;
	}

	if (page_tex.h <= canvas_h)
	{
	scroll_y = 0;
	y = canvas_y + (canvas_h - page_tex.h) / 2;
	}
	else
	{
	//		scroll_y = fz_clamp(scroll_y, 0, page_tex.h - canvas_h);
	y = canvas_y - scroll_y;
	}
	*/

	ui_draw_image(&page_tex, x - page_tex.x, y - page_tex.y);

	//FIXME do_forms(x, y);

	if (!search_active)
	{
		do_links(links, x, y);
		do_page_selection(x, y, x+page_tex.w, y+page_tex.h);
		if (search_hit_page == currentpage && search_hit_count > 0)
			do_search_hits(x, y);
	}
}

int ddi_get(char *buf, size_t size) {
	int result = 0;

	if (!ddiprefix) return 0;

	if (DDI_pickup( &ddi, buf, size ) == 0) {
		char *p;

		if (debug) fprintf(stderr,"%s:%d: Received '%s'\r\n", FL, buf);
		p = buf;
		//		while (p && *p && (*p != '\n')) { p++; } *p = '\0';
		if (debug) fprintf(stderr,"%s:%d: After filtering '%s'\r\n", FL, buf);
		result = 1;
	}  // if file opened

	return result;
}

static void run_main_loop(void)
{

	//	SDL_GL_MakeCurrent(sdlWindow, glcontext);
	//	SDL_RenderClear(sdlWindow);

	/*
		glViewport(0, 0, window_w, window_h);
	//glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClearColor(0.9f, 0.7f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_w, window_h, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	ui_begin();

	if (search_active) {
		//		int start_time = glutGet(GLUT_ELAPSED_TIME);

		if (ui.key == KEY_ESCAPE)
			search_active = 0;

		/* ignore events during search */
		ui.key = ui.mod = ui.plain = 0;
		ui.down = ui.middle = ui.right = 0;

		while (1) {
			//glutGet(GLUT_ELAPSED_TIME) < start_time + 200)
			search_hit_count = fz_search_page_number(ctx, doc, search_page, search_needle,
					search_hit_bbox, nelem(search_hit_bbox));
			if (debug) fprintf(stderr,"%s:%d: Main loop search - %d hits on '%s' at page %d\r\n", FL, search_hit_count, search_needle, search_page);
			if (search_hit_count)
			{
				fz_point p;
				fz_rect bb;
				search_active = 0;
				needle_has_hits = 1;
				search_hit_page = search_page;

				p.x = (canvas_w/2) *72 / (currentzoom );
				p.y = (canvas_h/2) *72 / (currentzoom );
				bb = search_hit_bbox[0];
				jump_to_page_xy(search_hit_page, bb.x0 -p.x, bb.y0 -p.y );

//				jump_to_page(search_hit_page);
				break;
			}
			else
			{
				if (debug) fprintf(stderr,"%s:%d: No hits on page %d, trying next...\r\n", FL, search_page);
				search_page += search_dir;
				if (search_page < 0 || search_page == fz_count_pages(ctx, doc))
				{
					if (debug) fprintf(stderr,"%s:%d: end of the road for '%s'\r\n", FL, search_needle);
					if (needle_has_hits) {
						search_page = 0;
					} else {
						snprintf(last_search_string,sizeof(last_search_string),"%s", search_needle);
						search_not_found = 1;
						update_title();
						search_active = 0;
					}
					break;
				}
			}
		} // while loop

		/* keep searching later */
		if (search_active) {
			//			glutPostRedisplay();
		}
	} // if search-active

	//do_app();

	/*
		if (doquit) { //		glutDestroyWindow(window); #ifdef __APPLE__ exit(1); #endif return; }
		*/

	canvas_w = window_w - canvas_x;
	canvas_h = window_h - canvas_y;

	do_canvas();

	if (showinfo) do_info();
	else if (showhelp) do_help();
	if (showoutline) do_outline(outline, canvas_x);

	if (showsearch) {
		int state = ui_input(canvas_x, 0, canvas_x + canvas_w, ui.lineheight+4, &search_input);
		if (state == -1)
		{
			ui.focus = NULL;
			showsearch = 0;
		}
		else if (state == 1)
		{
			ui.focus = NULL;
			showsearch = 0;
			search_page = -1;
			if ((search_needle)&&(search_needle != prior_search))
			{
				//				fz_free(ctx, search_needle);
				//???FIXME				search_needle = NULL;
			}
			if (search_input.end > search_input.text)
			{
				//				search_needle = fz_strdup(ctx, search_input.text);
				snprintf(prior_search,sizeof(prior_search),"%s", search_input.text);
				search_needle = prior_search;
				needle_has_hits = 0;
				search_active = 1;
				search_page = currentpage;
				if (debug) fprintf(stderr,"%s:%d: Loading prior search / needle with '%s', currentpage = %d\n", FL, search_needle, currentpage);
			}
			//glutPostRedisplay();
		}
	}

	if (search_active)
	{
		char buf[256];
		int x = canvas_x; // + 1 * ui.lineheight;
		int y = canvas_y; // + 1 * ui.lineheight;
		int w = canvas_w - 8 * ui.lineheight;
		int h = 1.25 * ui.lineheight;

		glBegin(GL_TRIANGLE_STRIP);
		{
			glColor4f(0.9f, 0.9f, 0.1f, 1.0f);
			glVertex2f(x, y);
			glVertex2f(x, y + h);
			glVertex2f(x + w, y);
			glVertex2f(x + w, y + h);
		}
		glEnd();

		sprintf(buf, "%d of %d.", search_page + 1, fz_count_pages(ctx, doc));
		glColor4f(0, 0, 0, 1);
		do_info_line(x, y +(1.1 * ui.lineheight), "Searching: ", buf);
	}

	ui_end();

	//	glutSwapBuffers();

	//	glFlush(); 
	//	glFinish();search_active)
	//	SDL_RenderCopy(sdlWindow);
	//	SDL_RenderPresent(sdlWindow);
	//	SDL_GL_SwapWindow(sdlWindow);
	ogl_assert(ctx, "swap buffers");
}

#if defined(FREEGLUT) && (GLUT_API_VERSION >= 6)
static void on_keyboard(int key, int x, int y)
#else
static void on_keyboard(unsigned char key, int x, int y)
#endif
{
#ifdef __APPLE__
	/* Apple's GLUT has swapped DELETE and BACKSPACE */
	if (key == 8)
		key = 127;
	else if (key == 127)
		key = 8;
#endif
	ui.key = key;
	//FIXME	ui.mod = glutGetModifiers();
	ui.plain = !(ui.mod & ~GLUT_ACTIVE_SHIFT);
	//	run_main_loop();
	ui.key = ui.mod = ui.plain = 0;
}

/*
	static void on_special(int key, int x, int y)
	{
	ui.key = 0;

	switch (key)
	{
	case GLUT_KEY_INSERT: ui.key = KEY_INSERT; break;
#ifdef GLUT_KEY_DELETE
case GLUT_KEY_DELETE: ui.key = KEY_DELETE; break;
#endif
case GLUT_KEY_RIGHT: ui.key = KEY_RIGHT; break;
case GLUT_KEY_LEFT: ui.key = KEY_LEFT; break;
case GLUT_KEY_DOWN: ui.key = KEY_DOWN; break;
case GLUT_KEY_UP: ui.key = KEY_UP; break;
case GLUT_KEY_PAGE_UP: ui.key = KEY_PAGE_UP; break;
case GLUT_KEY_PAGE_DOWN: ui.key = KEY_PAGE_DOWN; break;
case GLUT_KEY_HOME: ui.key = KEY_HOME; break;
case GLUT_KEY_END: ui.key = KEY_END; break;
case GLUT_KEY_F1: ui.key = KEY_F1; break;
case GLUT_KEY_F2: ui.key = KEY_F2; break;
case GLUT_KEY_F3: ui.key = KEY_F3; break;
case GLUT_KEY_F4: ui.key = KEY_F4; break;
case GLUT_KEY_F5: ui.key = KEY_F5; break;
case GLUT_KEY_F6: ui.key = KEY_F6; break;
case GLUT_KEY_F7: ui.key = KEY_F7; break;
case GLUT_KEY_F8: ui.key = KEY_F8; break;
case GLUT_KEY_F9: ui.key = KEY_F9; break;
case GLUT_KEY_F10: ui.key = KEY_F10; break;
case GLUT_KEY_F11: ui.key = KEY_F11; break;
case GLUT_KEY_F12: ui.key = KEY_F12; break;
}

if (ui.key)
{
//FIXME		ui.mod = glutGetModifiers();
ui.plain = !(ui.mod & ~GLUT_ACTIVE_SHIFT);
//run_main_loop();
ui.key = ui.mod = ui.plain = 0;
}
}
*/

static void on_wheel(int direction, int x, int y)
{

	/*
	 * NOTE: We don't set the glutOnMouse() callback to be this
	 * function because it's not dependable in X.  Rather instead
	 * we use on_mouse() to determine all the button states and
	 * call on_wheel() in a more predictable manner.
	 */

	if (!(SDL_GetModState() & KMOD_CTRL ) != (!scroll_wheel_swap)) {
		double oz;
		double tx, ty, desx, desy;
		double pct;
		double tsx, tsy;

		oz = currentzoom;

		tsx = scroll_x;
		tsy = scroll_y;

		pct = 1.2;

		tx = (tsx + x);
		ty = (tsy + y);

		if (direction > 0) {
			currentzoom *= pct;
			if (currentzoom > MAXRES) { currentzoom = oz;  return; }
			desx = tx *pct;
			desy = ty *pct;
			tsx += desx -tx;
			tsy += desy -ty;

		} else  {
			currentzoom /= pct;
			if (currentzoom < MINRES) { currentzoom = oz; return; }
			desx = tx /pct;
			desy = ty /pct;
			tsx += desx -tx;
			tsy += desy -ty;
		}

		scroll_x = floor(tsx);
		scroll_y = floor(tsy);

	} else {
		int jump = 1;
		if (direction < 0) currentpage += jump;
		else currentpage -= jump;
		if (currentpage < 0) currentpage = 0;
		if (currentpage >= fz_count_pages(ctx,doc)) {  currentpage = fz_count_pages(ctx,doc) -1; }
	}
}

static void on_mouse(int button, int action, int x, int y)
{
	ui.x = x;
	ui.y = y;

	if (debug) fprintf(stderr,"%s:%d: button: %d %d\r\n", FL, button, action);
	switch (button)
	{
		case SDL_BUTTON_LEFT: ui.down = (action == SDL_MOUSEBUTTONDOWN); break;
		case SDL_BUTTON_MIDDLE: ui.middle = (action == SDL_MOUSEBUTTONDOWN); break;
		case SDL_BUTTON_RIGHT: ui.right = (action == SDL_MOUSEBUTTONDOWN); break;
	}
}

static void on_motion(int x, int y)
{
	ui.x = x;
	ui.y = y;

	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		if (!am_dragging) {
			am_dragging = 1;
			dragging_start.x = x;
			dragging_start.y = y;
		} else {
			scroll_x += dragging_start.x -x;
			scroll_y += dragging_start.y -y;
			dragging_start.x = x;
			dragging_start.y = y;
		}
	} else {
		am_dragging = 0;
	}
}

static void on_error(const char *fmt, va_list ap)
{
#ifdef _WIN32
	char buf[1000];
	fz_vsnprintf(buf, sizeof buf, fmt, ap);
	MessageBoxA(NULL, buf, "PDF Error", MB_ICONERROR);
#else
	fprintf(stderr, "GLUT error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
#endif
}

static void on_warning(const char *fmt, va_list ap)
{
	fprintf(stderr, "GLUT warning: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
}

static void ddi_check( void ) {
	char sn_a[10240];
	char comp_a[128], comp_b[128];
	char *cmd;

	if (ddi_simulate_option == DDI_SIMULATE_OPTION_SEARCH_NEXT) {
		if (search_needle) snprintf(sn_a,sizeof(sn_a),"%s", search_needle);
		else if (strlen(ddi.last_pickup)) snprintf(sn_a, sizeof(sn_a), "%s", ddi.last_pickup);
		else return;
	}

	/*
	 * 
	 * NOTE: We're only expecting single-line DDI requests here.
	 *
	 * The only compound DDI request we get with muPDF is right at 
	 * the start when we load up.
	 *
	 */

	search_compound = 0;

	if ((ddi_simulate_option)||(ddi_get( sn_a, sizeof(sn_a)))) {

		if (ddi_simulate_option == DDI_SIMULATE_OPTION_NONE) {
			search_type = SEARCH_TYPE_DDI_SEQUENCE;
		} 

		ddi_simulate_option = DDI_SIMULATE_OPTION_NONE;

		if (strlen(sn_a) < 2) return;

		if (debug) fprintf(stderr,"%s:%d: Searching: '%s'\r\n", FL, sn_a);
		if ((cmd = strstr(sn_a, "!strictmatch:"))) {
			char tmp[1024];
			snprintf(tmp,sizeof(tmp),"%s",sn_a);
			snprintf(sn_a,sizeof(sn_a), "%s", cmd +strlen("!strictmatch:"));
			if (debug) fprintf(stderr,"%s:%d: Strict match", FL);
			ctx->flags |= FZ_CTX_FLAGS_STRICT_MATCH;
		}

		if ((cmd = strstr(sn_a, "!stdmatch:"))) {
			char tmp[1024];
			snprintf(tmp,sizeof(tmp),"%s",sn_a);
			snprintf(sn_a,sizeof(sn_a), "%s", cmd +strlen("!stdmatch:"));
			if (debug) fprintf(stderr,"%s:%d: Standard match (default)", FL);
			ctx->flags &= ~FZ_CTX_FLAGS_STRICT_MATCH;
		}


		if ((cmd = strstr(sn_a, "!compsearch:"))) {
			/*
			 * compound search requested.  First we find the page with the
			 * first part, then we find the second part.
			 *
			 */
			char *p;


			snprintf(comp_a, sizeof(comp_a), "%s", cmd +strlen("!compsearch:"));
			fprintf(stderr,"%s:%d: Comp search main:'%s' comp_a:'%s'\r\n", FL, sn_a, comp_a);
			p = strrchr(comp_a, ':');
			if (p) {
				fprintf(stderr,"%s:%d: Split found\r\n", FL);
				snprintf(comp_b, sizeof(comp_b), "%s", p+1);
				*p = '\0';
				snprintf(sn_a,sizeof(sn_a),"%s", comp_a);
				fprintf(stderr,"%s:%d: main = '%s', secondary = '%s'\r\n", FL, sn_a, comp_b);
				search_compound = 1;
			} else {
				fprintf(stderr,"%s:%d: No split in '%s'", FL, comp_a);
				search_compound = 0;
			}
		}

		if (strncmp(sn_a, "!pagesearch:", strlen("!pagesearch:"))==0) {
			char tmp[128];
			search_in_page_only = 1;
			snprintf(tmp, sizeof(tmp), "%s", sn_a+strlen("!pagesearch:"));
			snprintf(sn_a, sizeof(sn_a), "%s", tmp);
		}

		if (strstr(sn_a, "!quit:")) {
			if (time(NULL) -process_start_time > 2) quit();

		} else if (strstr(sn_a, "!debug:")) {
			fprintf(stderr,"%s:%d: DEBUG mode ACTIVE\r\n", FL);
			debug = 1;

		} else if (strstr(sn_a, "!cinvert:")) {
			currentinvert = !currentinvert;

		} else if (strstr(sn_a, "!ss:")) {
			scroll_wheel_swap = 1;

		} else if (strstr(sn_a, "!raise:")) {
			raise_on_search = 1;

		} else if (strstr(sn_a, "!noraise:")) {
			raise_on_search = 0;

		} else if (strstr(sn_a, "!load:")) {

			char *fnp;

			/*
			 * load a file, not searching.
			 */

			fnp = strstr(sn_a, "!load:");
			if (debug) fprintf(stderr,"%s:%d: fnp = '%s'\r\n", FL, fnp );
			snprintf(filename,sizeof(filename),"%s", fnp +strlen("!load:"));
			fnp = strpbrk(filename,"\n\r");
			if (fnp) *fnp = '\0';
			if (debug) fprintf(stderr,"%s:%d: filename = '%s'\r\n", FL, filename );

			title = strrchr(filename, '/');
			if (!title)
				title = strrchr(filename, '\\');
			if (title)
				++title;
			else
				title = filename;

			ctx = fz_new_context(NULL, NULL, 0);
			fz_register_document_handlers(ctx);

			if (layout_css)
			{
				fz_buffer *buf = fz_read_file(ctx, layout_css);
				fz_set_user_css(ctx, fz_string_from_buffer(ctx, buf));
				fz_drop_buffer(ctx, buf);
			}

			fz_set_use_document_css(ctx, layout_use_doc_css);

			reload();

		} else {
			char sn_b[1024];

			/*
			 * searching
			 *
			 * Sending the same search string as before
			 * will result in us moving to the next index.
			 *
			 * sn contains the search string.
			 *
			 */

			sn_b[0] = '\0';
			if (search_heuristics == 1) {
				if (strchr(sn_a,'_')) {
					int i;
					snprintf(sn_b,sizeof(sn_b),"%s", sn_a);
					for (i = 0; i < (strlen(sn_b) -1); i++) {
						if (sn_b[i] == '_') sn_b[i] = ' ';
					}
					if(debug)fprintf(stderr,"%s:%d: Alternative search: '%s'\r\n", FL, sn_b);
				}
			} else {
				if (debug) fprintf(stderr,"%s:%d: Search heuristics disabled\r\n", FL);
			}

			if (search_page == -1) search_page = 0;

			/*
			 * Step 1: check our input
			 *
			 */
			if (search_in_page_only) {
				search_inpage_index = 0;

			} else if (strcmp(sn_a, prior_search)==0) {
				/*
				 * If we're resuming an existing search
				 */
				//				search_inpage_index++; 

			} else {
				/*
				 * If we're starting a new search 
				 */
				search_current_page = -1;
				search_inpage_index = -1;
				search_page = 0;
				snprintf(prior_search, sizeof(prior_search), "%s", sn_a);
			}


			glViewport(0, 0, window_w, window_h);
			glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, window_w, window_h, 0, -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			ui_begin();

			{
				//search_page = search_current_page;
				search_active = 1;
				search_not_found = 0;

				if (raise_on_search == 1) {
					SDL_RaiseWindow(sdlWindow);
				}

				while (search_active) {
					if (search_page > fz_count_pages(ctx, doc) -1) {
						search_current_page = -1;
						search_inpage_index = -1;
						search_page = 0;
						prior_search[0] = 0;
						sn_a[0] = 0;
						search_active = 0;
						break;
					}

					/*
					 * Because of the prevelance of space vs underscore strings in PDF schematics
					 * we try search for both variants
					 *
					 */
					search_hit_count = fz_search_page_number(ctx, doc, search_page, sn_a, search_hit_bbox, nelem(search_hit_bbox));
					if (debug) fprintf(stderr,"%s:%d:Searching for '%s', %d hits on page %d\n", FL, sn_a, search_hit_count, search_page +1);
					if ((search_hit_count == 0)&&(strlen(sn_b))&&(strchr(sn_b,' '))) {
						search_hit_count = fz_search_page_number(ctx, doc, search_page, sn_b, search_hit_bbox, nelem(search_hit_bbox));
						if (debug) fprintf(stderr,"%s:%d:Searching for '%s', %d hits on page %d\n", FL, sn_b, search_hit_count, search_page +1);
					} 

					/*
					 * With compound searching, we're using using the initial part just to locate our page
					 *
					 */
					if ((search_compound == 1)&&(search_hit_count > 0)) {

						if (debug) fprintf(stderr,"%s:%d: page:%d, compound searching, now check for '%s'\r\n", FL, search_page+1, comp_b);

						search_hit_count = fz_search_page_number(ctx, doc, search_page, comp_b, search_hit_bbox, nelem(search_hit_bbox));
						if (debug) fprintf(stderr,"%s:%d: '%s' matched %d time(s)\r\n", FL, comp_b, search_hit_count);
						//						if (local_hits > 0) {
						//							snprintf(sn_a, sizeof(sn_a), "%s", comp_b);
						//							search_hit_count = fz_search_page_number(ctx, doc, search_page, sn_a, search_hit_bbox, nelem(search_hit_bbox));
						//						} else search_hit_count = 0;
					}

					/*
					 * If we've used up all our hits in this page
					 *
					 * OR if there were no hits on this page
					 *
					 */
					if (!search_in_page_only) {
						if (debug) fprintf(stderr,"%s:%d: Normal page search: %d hits, inpage_index=%d, page=%d\r\n", FL, search_hit_count, search_inpage_index, currentpage);
						if ((search_hit_count == 0)||(search_inpage_index > search_hit_count -2)) {
							search_inpage_index = -1;
							search_current_page = -1;
							search_page++;

							/*
							 * If we've used up all the pages in the document
							 *
							 */
							if (search_page >= fz_count_pages(ctx, doc) -1) {

								/*
								 * If the document does have hits
								 *
								 */
								if (document_has_hits) {
									if (debug) fprintf(stderr,"%s:%d: End of document reached, but resetting back to start\r\n", FL);
									search_page = 0;
									document_has_hits = 0;
									search_active = 1;
									//									continue;
								} else {
									char b[1024];
									if (debug) fprintf(stderr,"%s:%d: End of document reached, no hits found at all\r\n", FL);
									snprintf(last_search_string, sizeof(last_search_string),"%s", sn_a);
									snprintf(b,sizeof(b),"'%s' not found", sn_a);

									search_not_found = 1;
									update_title();
									search_active = 0;
									break; // no hits in document
								}
							}
							continue;
						}
					} else {
						search_inpage_index = -1;
					}


					/*
					 * If we have search hits...
					 *
					 */ 
					if ((search_hit_count)&&(!search_in_page_only)) {
						fz_point p;
						fz_rect bb;
						document_has_hits = 1;

						/*
						 * If the search_current_page hasn't yet been initialised
						 * then set it to this page that we've got hits on.
						 *
						 */
						search_current_page = search_page;
						search_inpage_index++;

						search_active = 0;
						search_hit_page = search_page;

						p.x = (canvas_w/2) *72 / (currentzoom );
						p.y = (canvas_h/2) *72 / (currentzoom );
						bb = search_hit_bbox[search_inpage_index];
						jump_to_page_xy(search_hit_page, bb.x0 -p.x, bb.y0 -p.y );
					} // if search hit count > 0

					if (search_in_page_only) {
						search_active = 0;
						search_in_page_only = 0;
					}

				} // while search is active

			} // block braces only

			// FIXME glutPostRedisplay();


			//			glViewport(0, 0, window_w, window_h);
			//do_app();

			if (doquit)
			{
				//FIXME				glutDestroyWindow(window);
#ifdef __APPLE__
				exit(1); /* GLUT on MacOS keeps running even with no windows */
#endif
				return;
			}
		}

	}
}

/*
 * FIXME - put this in the SDL event loop instead
 static void on_timer( int value ) {
 ddi_check();
 glutTimerFunc( GLUT_TIMER_DURATION, on_timer, 1 );
 }
 */

void ui_set_clipboard(const char *buf)
{
	SDL_SetClipboardText(buf);
}

const char *ui_get_clipboard(void)
{
	return SDL_GetClipboardText();
}

static void usage(const char *argv0)
{
	fprintf(stderr, "mupdf-gl version %s\n", FZ_VERSION);
	fprintf(stderr, "usage: %s [options] document [page]\n", argv0);
	fprintf(stderr, "\t-p -\tpassword\n");
	fprintf(stderr, "\t-r -\tresolution\n");
	fprintf(stderr, "\t-I\tinvert colors\n");
	fprintf(stderr, "\t-D\t<ddi prefix>\n");
	fprintf(stderr, "\t-W -\tpage width for EPUB layout\n");
	fprintf(stderr, "\t-H -\tpage height for EPUB layout\n");
	fprintf(stderr, "\t-S -\tfont size for EPUB layout\n");
	fprintf(stderr, "\t-U -\tuser style sheet for EPUB layout\n");
	fprintf(stderr, "\t-X\tdisable document styles for EPUB layout\n");
	exit(1);
}


/*
	void GLAPIENTRY
MessageCallback( GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam )
{
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
			type, severity, message );
}
*/


//do other stuff.
#ifdef _MSC_VER
int main_utf8(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	int c;
	int check_again = 0;

	if (debug) fprintf(stderr,"start.\r\n");

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 3;
	}

	//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	sdlWindow = SDL_CreateWindow("FlexBV PDF", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	SDL_GLContext glcontext = SDL_GL_CreateContext(sdlWindow);
//	SDL_GL_SetSwapInterval(1);
	SDL_EnableScreenSaver();


	window_w = 1280;
	window_h = 720;
	windowx = 1280;
	windowy = 720;


	process_start_time = time(NULL); // used to discriminate if we're picking up old !quit: calls.

	if (debug) fprintf(stderr,"Parsing parameters\r\n");
	while ((c = fz_getopt(argc, argv, "p:r:IW:H:S:U:X:D:")) != -1)
	{
		switch (c)
		{
			default: usage(argv[0]); break;
			case 'p': password = fz_optarg; break;
			case 'r': currentzoom = fz_atof(fz_optarg); break;
			case 'I': currentinvert = !currentinvert; break;
			case 'W': layout_w = fz_atof(fz_optarg); break;
			case 'H': layout_h = fz_atof(fz_optarg); break;
			case 'S': layout_em = fz_atof(fz_optarg); break;
			case 'U': layout_css = fz_optarg; break;
			case 'X': layout_use_doc_css = 0; break;
			case 'D': ddiprefix = fz_optarg; break;
		}
	}

	if (fz_optind < argc)
		anchor = argv[fz_optind++];

	/* ddi setup */
	if (debug) fprintf(stderr,"DDI setup '%s'\r\n", ddiprefix);
	DDI_init(&ddi);
	DDI_set_prefix(&ddi, ddiprefix);
	DDI_set_mode(&ddi, DDI_MODE_SLAVE);

	{
		/*
		 * DDI setup package, is the first one we receive
		 * and may contain multiple commands for us to process.
		 *
		 */
		char s[10240];
		int x = 10;
		if (debug) fprintf(stderr,"%s:%d: DDI PICKUP\r\n",FL);
		while ((DDI_pickup(&ddi, s, sizeof(s))==0)&&(x--)) {
			char *p, *q;
			usleep(10000); // 0.1 sec
		
			if ((p = strstr(s, "!debug:"))) {
				debug = 1;
				if (debug) fprintf(stderr,"%s:%d:DDI Data---------\r\n%s\r\n-------------\r\n",FL,s);
			}

			if ((p = strstr(s,"!load:"))!=NULL) {
				q = strchr(p,'\n');
				if (q) *q = '\0';
				snprintf(filename, sizeof(filename), "%s", p+6 );
				if (debug) fprintf(stderr,"%s:%d: Filename set to '%s'\r\n",FL, filename);
				if (q) *q = '\n';
			}

			if ((p = strstr(s, "!setwindowsize:"))) {
				q = strchr(p,'\n');
				if (q) *q = '\0';
				sscanf(p +strlen("!setwindowsize:"),"%d %d", &windowx, &windowy );
				if (q) *q = '\n';
			}

			if ((p = strstr(s, "!cinvert:"))) {
				if (debug) fprintf(stderr,"%s:%d: Colour invert: ENABLED\r\n", FL);
				currentinvert = !currentinvert;
			}

			if ((p = strstr(s, "!ss:"))) {
				if (debug) fprintf(stderr,"%s:%d: Zoom on scroll: ENABLED\r\n", FL);
				scroll_wheel_swap = 1;
			}

			if ((p = strstr(s, "!raise:"))) {
				if (debug) fprintf(stderr,"%s:%d: Raise on search: ENABLED\r\n", FL);
				raise_on_search = 1;
			}


			if ((p = strstr(s, "!strictmatch:"))) {
				if (debug) fprintf(stderr,"%s:%d: Strict match", FL);
				ctx->flags |= FZ_CTX_FLAGS_STRICT_MATCH;
			}

			if ((p = strstr(s, "!stdmatch:"))) {
				if (debug) fprintf(stderr,"%s:%d: Standard match (default)", FL);
				ctx->flags &= ~FZ_CTX_FLAGS_STRICT_MATCH;
			}

			if ((p = strstr(s, "!noheuristics:"))) {
				if (debug) fprintf(stderr,"%s:%d: No heuristics", FL);
				search_heuristics = 0;
			}

			if ((p = strstr(s, "!heuristics:"))) {
				search_heuristics = 1;
				if (debug) fprintf(stderr,"%s:%d: Heuristics", FL);
			}

			if ((p = strstr(s, "!noraise:"))) {
				raise_on_search = 0;
			}

		} // while we're trying to read the DDI packet

		if (x == 0) {

			if (fz_optind < argc)
			{
				fz_strlcpy(filename, argv[fz_optind++], sizeof filename);
			}
			else
			{
#ifdef _WIN32
				win_install();
				if (!win_open_file(filename, sizeof filename))
					exit(0);
#else
				usage(argv[0]);
#endif
			}


		}
	} // DDI read block



	title = strrchr(filename, '/');
	if (!title)
		title = strrchr(filename, '\\');
	if (title)
		++title;
	else
		title = filename;

	if (debug) fprintf(stderr,"Initialising FlexBV-PDF\r\n");
	/* Init MuPDF */

	ctx = fz_new_context(NULL, NULL, 0);
	if (search_heuristics) ctx->flags |= FZ_CTX_FLAGS_SPACE_HEURISTIC;

	fz_register_document_handlers(ctx);

	if (layout_css)
	{
		fz_buffer *buf = fz_read_file(ctx, layout_css);
		fz_set_user_css(ctx, fz_string_from_buffer(ctx, buf));
		fz_drop_buffer(ctx, buf);
	}

	fz_set_use_document_css(ctx, layout_use_doc_css);

	if (debug) fprintf(stderr,"%s:%d: Loading document\r\n", FL);
	load_document();
	if (debug) fprintf(stderr,"%s:%d: Loading page\r\n", FL);
	load_page();
	if (debug) fprintf(stderr,"%s:%d: Setting memory and search\r\n", FL);

	/* Init IMGUI */

	memset(&ui, 0, sizeof ui);

	search_input.p = search_input.text;
	search_input.q = search_input.p;
	search_input.end = search_input.p;

	if (debug) fprintf(stderr,"%s:%d: ARB non-power-of-two test\r\n", FL);
	has_ARB_texture_non_power_of_two = 0;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	ui.fontsize = DEFAULT_UI_FONTSIZE;
	ui.baseline = DEFAULT_UI_BASELINE;
	ui.lineheight = DEFAULT_UI_LINEHEIGHT;

	if (debug) fprintf(stderr,"%s:%d: ui init fonts\r\n", FL);
	ui_init_fonts(ctx, ui.fontsize);

	if (debug) fprintf(stderr,"%s:%d: render page\r\n", FL);
	render_page();

//	shrinkwrap();


	if (debug) fprintf(stderr,"%s:%d: update title\r\n", FL);
	update_title();

	if (debug) fprintf(stderr,"%s:%d: SDL loop starting\r\n\r\n", FL);
	{
		while (!doquit) {


			glViewport(0,0,window_w, window_h);
			glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, window_w, window_h, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			//if (SDL_PollEvent(&sdlEvent) ) {
			while (SDL_PollEvent(&sdlEvent) ) {
			//if (SDL_WaitEvent(&sdlEvent) ) {
				switch (sdlEvent.type) {
					case SDL_WINDOWEVENT:
						switch (sdlEvent.window.event) {
							case SDL_WINDOWEVENT_RESIZED:
							case SDL_WINDOWEVENT_SIZE_CHANGED:
//							case SDL_WINDOWEVENT_MAXIMIZED:
//							case SDL_WINDOWEVENT_RESTORED:
								window_w = sdlEvent.window.data1;
								window_h = sdlEvent.window.data2;
								break;
						}
						break;

					case SDL_QUIT:
						quit();
						break;

					case SDL_KEYDOWN:
						{
							ui.key = sdlEvent.key.keysym.sym;
							if (SDL_GetModState() & KMOD_SHIFT) ui.key = toupper(ui.key);
							do_keypress();
//							if (sdlEvent.key.keysym.sym == SDLK_q) {
//								quit = 1;
//							}
						}
						break;

					case SDL_MOUSEMOTION:
						{
							int x, y;
							SDL_GetMouseState( &x, &y );
							on_motion( x, y );
						}
						break;

					case SDL_MOUSEWHEEL:
						{
							int x, y;
							SDL_GetMouseState( &x, &y );
							on_wheel(  sdlEvent.wheel.y, x, y );
						}
						break;

					case SDL_MOUSEBUTTONDOWN:
						{
							int x, y;
							SDL_GetMouseState( &x, &y );
							on_mouse( sdlEvent.button.button, SDL_MOUSEBUTTONDOWN, x, y);
						}
						break;


					case SDL_MOUSEBUTTONUP:
						{
							int x, y;
							SDL_GetMouseState( &x, &y );
							on_mouse( sdlEvent.button.button, SDL_MOUSEBUTTONUP, x, y);
						}
						break;
				}

			} // while SDL event
			
				if (check_again) {
					check_again--;
				} else {
					ddi_check();
					check_again = 10;
				}

			run_main_loop();
			ui.key = ui.mod = ui.plain = 0;

		//		do_canvas();
				SDL_GL_SwapWindow(sdlWindow);

		} // while
	}

	if (debug) fprintf(stderr,"%s:%d: SDL loop ended\r\n", FL);

	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(sdlWindow);


	ui_finish_fonts(ctx);

	SDL_DestroyWindow(sdlWindow);

	SDL_Quit();

#ifndef NDEBUG
	if (fz_atoi(getenv("FZ_DEBUG_STORE")))
		fz_debug_store(ctx);
#endif

	fz_drop_stext_page(ctx, text);
	fz_drop_link(ctx, links);
	fz_drop_page(ctx, page);
	fz_drop_outline(ctx, outline);
	fz_drop_document(ctx, doc);
	fz_drop_context(ctx);

	return 0;
}

#ifdef _MSC_VER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc;
	LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	char **argv = fz_argv_from_wargv(argc, wargv);
	int ret = main_utf8(argc, argv);
	fz_free_argv(argc, argv);
	return ret;
}
#endif
