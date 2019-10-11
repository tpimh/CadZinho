#include "gui.h"

/* ***********************************************************
the fowling functions are for rendering text in gui, by rastering each
glyphs in a monochrome image in memory. */

struct gui_font * gui_new_font (struct nk_user_font *base_font){
	/* create a new font structute */
	/* memory allocation */
	struct gui_font *font = calloc(1, sizeof(struct gui_font));
	if (font){
		/* initial parameters */
		font->font = base_font;
		font->glyphs = NULL;
		font->next = NULL;
	}
	return font;
}

struct gui_font * gui_font_add (struct gui_font *list, struct nk_user_font *base_font){
	/* create a new font structute and append in list*/
	if (!list) return NULL;
	/* create font */
	struct gui_font *font = gui_new_font (base_font);
	if (font){
		struct gui_font *curr = list;
		/* find the list tail */
		while (curr->next){
			curr = curr->next;
		}
		/*atach the new font at list tail */
		curr->next = font;
	}
	return font;
}

struct gui_font * gui_get_font (struct gui_font *list, struct nk_user_font *base_font){
	/* return a font structure in list, by its pointer */
	if (!list) return NULL;
	
	/* try to find the font previously stored */
	struct gui_font *curr = list->next;
	while(curr){ /*sweep list */
		if (curr->font == base_font) return curr; /* success */
		curr = curr->next;
	}
	
	/* if not found, add the font in list*/
	return gui_font_add (list, base_font);
}

struct gui_glyph * gui_new_glyph (struct gui_font *gfont, int code_p){
	/* create a new glyph structute */
	if (!gfont) return NULL;
	
	/* memory allocation */
	struct gui_glyph *glyph = calloc(1, sizeof(struct gui_glyph));
	if (glyph){
		/* initial parameters */
		glyph->code_p = code_p;
		glyph->w = 0;
		glyph->h = 0;
		glyph->adv = 0.0;
		memset(glyph->rast, 0, 20*25);
		glyph->next = NULL;
	}
	/* -------- raster the glyph and get its size parameters ----------*/
	struct tfont *font = (struct tfont *)gfont->font->userdata.ptr;
	if (font->type != FONT_TT){ /* shape font*/
		
		/* TODO*/
		
	}
	else { /* True Type font*/
		struct tt_font *tt_fnt = font->data;
		float scale = tt_fnt->scale * gfont->font->height;
		int x0, y0, x1, y1, w, h;
		
		/* try to find tt glyph */
		struct tt_glyph *curr_glyph =  tt_find_cp (tt_fnt, code_p);
		if (!curr_glyph){/* if not found, add the glyph in font for future use*/
			curr_glyph = tt_get_glyph (tt_fnt, code_p);
			if(curr_glyph){
				if (tt_fnt->end) tt_fnt->end->next = curr_glyph;
				tt_fnt->end = curr_glyph;
			}
		}
		if (curr_glyph){ /* glyph found */
			/* find size parameters */
			stbtt_GetGlyphBitmapBox((stbtt_fontinfo *) tt_fnt->info, curr_glyph->g_idx, scale, scale, &x0, &y0, &x1, &y1);
			w = x1 - x0; h = y1 - y0;
			/* size limits */
			w = (w < 25)? w : 25;
			h = (h < 20)? h : 20;
			/* rasterize glyph*/
			stbtt_MakeGlyphBitmap((stbtt_fontinfo *) tt_fnt->info, glyph->rast, w, h, 25, scale, scale, curr_glyph->g_idx);
			
			/* update glyph parameters */
			glyph->x = x0;
			glyph->y = y0;
			glyph->w = w;
			glyph->h = h;
			glyph->adv = curr_glyph->adv * gfont->font->height;
			
		}
	}
	/* ------------------------------------------------------------ */
	return glyph;
}

struct gui_glyph * gui_glyph_add (struct gui_font *font, int code_p){
	/* create a new glyph structute and append in font*/
	if (!font) return NULL;
	/* create glyph */
	struct gui_glyph *glyph = gui_new_glyph (font, code_p);
	if (glyph){
		struct gui_glyph *curr = font->glyphs;
		/* find the font's tail */
		if (curr){
			while (curr->next){
				curr = curr->next;
			}
			/*atach the new glyph at font's tail */
			curr->next = glyph;
		}
		else{ /* if is the first glyph in font */ 
			font->glyphs = glyph;
		}
	}
	
	return glyph;
}

struct gui_glyph * gui_get_glyph (struct gui_font *font, int code_p){
	/* return a glyph structure in font, by its code point (unicode) */
	if (!font) return NULL;
	
	/* try to find the glyph previously stored */
	struct gui_glyph *curr = font->glyphs;
	while(curr){ /* sweep font */
		if (curr->code_p == code_p) return curr; /* success */
		curr = curr->next;
	}
	/* if not found, add the glyph in font*/
	return gui_glyph_add (font, code_p);
}

int gui_list_font_free (struct gui_font *list){
	/* free list of fonts and all its childs structures */
	if (!list) return 0;
	struct gui_font *curr_font = list->next;
	struct gui_font *next_font;
	struct gui_glyph *curr_glyph, *next_glyph;
	
	while(curr_font){ /* sweep font list */
		next_font = curr_font->next;
		curr_glyph = curr_font->glyphs;
	
		while(curr_glyph){ /* sweep current font */
			next_glyph = curr_glyph->next;
			free(curr_glyph); /* free each glyph */
			curr_glyph = next_glyph;
		}
		free (curr_font); /* free each font */
		curr_font = next_font;
	}
	
	free (list); /* free main list structure */
}
/* ************************************************************* */

int gui_tab (gui_obj *gui, const char *title, int active){
	const struct nk_user_font *f = gui->ctx->style.font;
	struct nk_style_button *sel_type;
	
	/* calcule button size */ 
	float text_width = f->width(f->userdata, f->height, title, nk_strlen(title));
	float widget_width = text_width + 6 * gui->ctx->style.button.padding.x;
	/* verify if active or not */
	sel_type = &gui->b_icon_unsel;
	if (active) sel_type = &gui->b_icon_sel;
	/* do the button */
	nk_layout_row_push(gui->ctx, widget_width);
	int r = nk_button_label_styled(gui->ctx, sel_type, title);
	
	return r;
}

/* ************************************************** */

int gui_default_modal(gui_obj *gui){
	gui->modal = SELECT;
	
	return gui_first_step(gui);
}

int gui_first_step(gui_obj *gui){
	gui->en_distance = 0;
	gui->draw_tmp = 0;
	gui->element = NULL;
	gui->draw = 1;
	gui->step = 0;
	gui->draw_phanton = 0;
	return gui_next_step(gui);
}
int gui_next_step(gui_obj *gui){
	
	gui->lock_ax_x = 0;
	gui->lock_ax_y = 0;
	gui->user_flag_x = 0;
	gui->user_flag_y = 0;

	gui->draw = 1;
	return 1;
}

void set_style(struct nk_context *ctx, enum theme theme){
    struct nk_color table[NK_COLOR_COUNT];
    if (theme == THEME_WHITE) {
        table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_RED) {
        table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
        table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
        table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_BLUE) {
        table[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
        table[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
        table[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_DARK) {
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
        table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_ZE) {
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 71, 58, 215);
        table[NK_COLOR_HEADER] = nk_rgba(52, 57, 52, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(71, 161, 80, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(89, 201, 100, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(46, 57, 46, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_SELECT] = nk_rgba(58, 67, 57, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 112, 54, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(59, 115, 53, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(71, 161, 80, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_EDIT] = nk_rgba(50, 61, 50, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_COMBO] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_CHART] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(59, 115, 53, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(71, 161, 80, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 112, 54, 255);
        nk_style_from_table(ctx, table);
    } else {
        nk_style_default(ctx);
    }
}

float nk_user_font_get_text_width2(nk_handle handle, float height, const char *text, int len){
	struct tfont *font = (struct tfont *)handle.ptr;
	if ((text!= NULL) && (font!=NULL)) {
		
	
		/* We must copy into a new buffer with exact length null-terminated
		as nuklear uses variable size buffers and shx_fonts routines doesn't
		accept a length, it infers length from null-termination */
		char txt_cpy[len+2];
		strncpy((char*)&txt_cpy, text, len);
		//txt_cpy[len - 1] = ' ';
		txt_cpy[len] = '\0';
		
		double txt_w;
		if (font_str_w(font, txt_cpy, &txt_w, 0)){
			return (float) height * txt_w;
		}
	}
	return 0.0;
}

bmp_color nk_to_bmp_color(struct nk_color color){
	bmp_color ret_color = {.r = color.r, .g = color.g, .b = color.b, .a = color.a };
	return ret_color;
}

int gui_check_draw(gui_obj *gui){
	int draw = 0;
	
	draw = 0;
	if (gui){
		//gui->draw = 0;
		if (gui->ctx){
			void *cmds = nk_buffer_memory(&(gui->ctx->memory));
			if (cmds){
				if (memcmp(cmds, gui->last, gui->ctx->memory.allocated)) {
					memcpy(gui->last, cmds, gui->ctx->memory.allocated);
					//gui->draw = 1;
					draw = 1;
				}
			}
		}
	}
	return draw;
}

NK_API void nk_sdl_render(gui_obj *gui, bmp_img *img){
	const struct nk_command *cmd = NULL;
	bmp_color color = {.r = 255, .g = 255, .b =255, .a = 255};
	int iter = 0;
	
	static int one_time = 0;
	if (!one_time){
		//one_time =1;
	}
	
	if ((img != NULL) && (gui != NULL)){
		
		/* initialize the image with a solid line pattern */
		img->patt_i = 0;
		img->pix_count = 0;
		img->patt_size = 1;
		img->pattern[0] = 1;
		img->zero_tl =1;

		nk_foreach(cmd, gui->ctx){
			
			/* break the loop, if more then 10000 iterations */
			iter += 1;
			if (iter > 10000){
				printf("error render\n");
				break;
			}
			
			switch (cmd->type) {
				case NK_COMMAND_NOP: break;
				
				case NK_COMMAND_SCISSOR: {
					const struct nk_command_scissor *s =(const struct nk_command_scissor*)cmd;
					img->clip_x = (unsigned int)s->x;
					img->clip_y = (unsigned int)s->y;
					img->clip_w = (unsigned int)s->w;
					img->clip_h = (unsigned int)s->h;
				} break;
				
				case NK_COMMAND_LINE: {
					const struct nk_command_line *l = (const struct nk_command_line *)cmd;
					color = nk_to_bmp_color(l->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = (unsigned int) l->line_thickness;
					
					bmp_line(img, l->begin.x, l->begin.y, l->end.x,l->end.y);
				} break;
				
				case NK_COMMAND_RECT: {
					const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
					color = nk_to_bmp_color(r->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = (unsigned int) r->line_thickness;
					
					int x0, y0, x1, y1, i, cx, cy;
					
					bmp_line(img, r->x + r->rounding, r->y, r->x + r->w -r->rounding, r->y);
					x0 =  r->x + r->w - r->rounding;
					cx = x0;
					y0 = r->y;
					cy = r->y + r->rounding;
					for (i=13; i <= 16; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x + r->w, r->y + r->rounding, r->x + r->w, r->y + r->h - r->rounding);
					cx = r->x + r->w - r->rounding;
					x0 = r->x + r->w;
					y0 = r->y + r->h - r->rounding;
					cy = y0;
					for (i=1; i <= 4; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x + r->w - r->rounding, r->y + r->h, r->x + r->rounding, r->y + r->h);
					x0 = r->x + r->rounding;
					cx = x0;
					y0 = r->y + r->h;
					cy = r->y + r->h - r->rounding;
					for (i=5; i <= 8; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x, r->y + r->h - r->rounding, r->x, r->y + r->rounding);
					cx = r->x + r->rounding;
					x0 = r->x;
					y0 = r->y + r->rounding;
					cy = y0;
					for (i=9; i <= 12; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
				} break;
				
				case NK_COMMAND_RECT_FILLED: {
					const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
					
					int vert_x[20], vert_y[20];
					int i, cx, cy;
					
					vert_x[0] = r->x + r->rounding;
					vert_x[1] = r->x + r->w - r->rounding;
					
					cx =  r->x + r->w - r->rounding;
					cy = r->y + r->rounding;
					for (i=13; i < 16; i++){
						vert_x[i-11] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i-11] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[5] = r->x + r->w;
					vert_x[6] = r->x + r->w;
					
					cx = r->x + r->w - r->rounding;
					cy = r->y + r->h - r->rounding;
					for (i=1; i < 4; i++){
						vert_x[i+6] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+6] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[10] = r->x + r->w - r->rounding;
					vert_x[11] = r->x + r->rounding;
					
					cx = r->x + r->rounding;
					cy = r->y + r->h - r->rounding;
					for (i=5; i < 8; i++){
						vert_x[i+7] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+7] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[15] = r->x;
					vert_x[16] = r->x;
					
					cx = r->x + r->rounding;
					cy = r->y + r->rounding;
					for (i=9; i < 12; i++){
						vert_x[i+8] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+8] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_y[0] = r->y;
					vert_y[1] = r->y;
					
					vert_y[5] = r->y + r->rounding;
					vert_y[6] = r->y + r->h - r->rounding;
					
					vert_y[10] = r->y + r->h;
					vert_y[11] = r->y + r->h;
					
					vert_y[15] = r->y + r->h - r->rounding;
					vert_y[16] = r->y + r->rounding;
					
					color = nk_to_bmp_color(r->color);
					/*change the color */
					img->frg = color;
					
					bmp_poly_fill(img, 20, vert_x, vert_y, NULL);
				} break;
				
				case NK_COMMAND_CIRCLE: {
					const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
					color = nk_to_bmp_color(c->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = c->line_thickness;
					int xr = c->w/2;
					
					bmp_circle(img, c->x + xr, c->y + xr, xr);
				} break;
				
				case NK_COMMAND_CIRCLE_FILLED: {
					const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
					color = nk_to_bmp_color(c->color);
					/*change the color */
					img->frg = color;
					int xr = c->w/2;
					
					bmp_circle_fill(img, c->x + xr, c->y + xr, xr);
				} break;
				
				case NK_COMMAND_TRIANGLE: {
					const struct nk_command_triangle*t = (const struct nk_command_triangle*)cmd;
					color = nk_to_bmp_color(t->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = t->line_thickness;
					bmp_line(img, t->a.x, t->a.y, t->b.x, t->b.y);
					bmp_line(img, t->b.x, t->b.y, t->c.x, t->c.y);
					bmp_line(img, t->c.x, t->c.y, t->a.x, t->a.y);
				} break;
				
				case NK_COMMAND_TRIANGLE_FILLED: {
					const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
					int vert_x[3] = {t->a.x, t->b.x, t->c.x};
					int vert_y[3] = {t->a.y, t->b.y, t->c.y};
					
					color = nk_to_bmp_color(t->color);
					/*change the color */
					img->frg = color;
					
					bmp_poly_fill(img, 3, vert_x, vert_y, NULL);
				} break;
				
				case NK_COMMAND_POLYGON: {
					const struct nk_command_polygon *p = (const struct nk_command_polygon*)cmd;
					color = nk_to_bmp_color(p->color);
					//int i;
					//float vertices[p->point_count * 2];
					//for (i = 0; i < p->point_count; i++) {
					// vertices[i*2] = p->points[i].x;
					// vertices[(i*2) + 1] = p->points[i].y;
					//}
					//al_draw_polyline((const float*)&vertices, (2 * sizeof(float)),
					//    (int)p->point_count, ALLEGRO_LINE_JOIN_ROUND, ALLEGRO_LINE_CAP_CLOSED,
					//  color, (float)p->line_thickness, 0.0);
					//printf("polygon ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_POLYGON_FILLED: {
					const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
					color = nk_to_bmp_color(p->color);
					//int i;
					//float vertices[p->point_count * 2];
					// for (i = 0; i < p->point_count; i++) {
					//    vertices[i*2] = p->points[i].x;
					//     vertices[(i*2) + 1] = p->points[i].y;
					// }
					//  al_draw_filled_polygon((const float*)&vertices, (int)p->point_count, color);
					//printf("fill_polygon ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_POLYLINE: {
					const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
					color = nk_to_bmp_color(p->color);
					//int i;
					//float vertices[p->point_count * 2];
					//  for (i = 0; i < p->point_count; i++) {
					//      vertices[i*2] = p->points[i].x;
					//      vertices[(i*2) + 1] = p->points[i].y;
					//  }
					//  al_draw_polyline((const float*)&vertices, (2 * sizeof(float)),
					//      (int)p->point_count, ALLEGRO_LINE_JOIN_ROUND, ALLEGRO_LINE_CAP_ROUND,
					//      color, (float)p->line_thickness, 0.0);
					//printf("polyline ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_TEXT: {
					const struct nk_command_text *t = (const struct nk_command_text*)cmd;
					color = nk_to_bmp_color(t->foreground);
					img->frg = color;
					
					/* get font descriptor */
					struct tfont *font = (struct tfont *)t->font->userdata.ptr;
					
					if (font->type != FONT_TT){ /*shape font */
						/* rendering text by default general drawing engine */
						list_node * graph = list_new(NULL, FRAME_LIFE);
						if (graph) {
							if (font_parse_str(font, graph, FRAME_LIFE, (char *)t->string, NULL, 0)){
								graph_list_color(graph, color);
								graph_list_modify(graph, t->x, t->y + t->font->height, t->font->height, -t->font->height, 0.0);
								//graph_list_draw(graph, img, 0.0, 0.0, 1.0);
								graph_list_draw_aa(graph, img, 0.0, 0.0, 1.0);
							}
						}
					}
					else { /* True Type font */
						/* rendering text by glyph raster images - used only in GUI */
						/* get gui font structure */
						struct gui_font * gfont = gui_get_font (gui->ui_font_list, (struct nk_user_font *) t->font);
						struct gui_glyph *curr_glyph = NULL;
						int ofs = 0, str_start = 0, code_p;
						double ofs_x = 0.0;
						
						/* sweep the text string, decoding UTF8 in code points*/
						while (ofs = utf8_to_codepoint((char *)t->string + str_start, &code_p)){
							str_start += ofs;
							curr_glyph = gui_get_glyph (gfont, code_p);
							
							/* calcule current glyph position in main image */
							int x = t->x + curr_glyph->x + (int) ofs_x;
							int y = t->y + curr_glyph->y + (int) t->font->height;
							
							int w = curr_glyph->w;
							int h = curr_glyph->h;
							
							/* verify if glyph will be showed in image's clip area */
							rect_pos pos_p0 = rect_find_pos(x, y, img->clip_x, img->clip_y, img->clip_x + img->clip_w, img->clip_y + img->clip_h);
							rect_pos pos_p1 = rect_find_pos(x+w, y+h, img->clip_x, img->clip_y, img->clip_x + img->clip_w, img->clip_y + img->clip_h);
							if (!(pos_p0 & pos_p1)){
								/* copy rasterized glyph in main image */
								int i = 0, j = 0;
								for (j = 0; j < h; j++){
									for (i = 0; i < w; i++){ /* pixel by pixel */
										img->frg.a = (curr_glyph->rast[j * 25 + i] * color.a) / 255;
										bmp_point_raw (img, x + i, y + j);
									}
								}
							}
							ofs_x += curr_glyph->adv; /*update position for next glyph */
						}
					}
				} break;
				
				case NK_COMMAND_CURVE: {
					const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
					color = nk_to_bmp_color(q->color);
					// float points[8];
					// points[0] = (float)q->begin.x;
					// points[1] = (float)q->begin.y;
					// points[2] = (float)q->ctrl[0].x;
					// points[3] = (float)q->ctrl[0].y;
					//points[4] = (float)q->ctrl[1].x;
					// points[5] = (float)q->ctrl[1].y;
					// points[6] = (float)q->end.x;
					// points[7] = (float)q->end.y;
					// al_draw_spline(points, color, (float)q->line_thickness);
					//printf("curve ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_ARC: {
					const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
					color = nk_to_bmp_color(a->color);
					//    al_draw_arc((float)a->cx, (float)a->cy, (float)a->r, a->a[0],
					//       a->a[1], color, (float)a->line_thickness);
					//printf("arc ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_RECT_MULTI_COLOR: {
					if (!one_time){
						one_time =1;
						//printf("multi_c ");//------------------------------------teste
					}
				} break;
				
				case NK_COMMAND_IMAGE: {
					const struct nk_command_image *i = (struct nk_command_image *)cmd;
					if (i->h > 0 && i->w > 0){
						bmp_img *w_img = (bmp_img *)i->img.handle.ptr;
						if (w_img){
							w_img->zero_tl = 1;
							bmp_copy(w_img, img, i->x, i->y);
							w_img->zero_tl = 0;
						}
					}
				} break;
				
				case NK_COMMAND_ARC_FILLED: {
					
				} break;
				
				default: break;
			}
		}
		/* reset image parameters */
		img->zero_tl = 0;
		img->clip_x = 0;
		img->clip_y = 0;
		img->clip_w = img->width;
		img->clip_h = img->height;
	}
}

static void nk_sdl_clipbard_paste(nk_handle usr, struct nk_text_edit *edit){
	const char *text = SDL_GetClipboardText();
	if (text) nk_textedit_paste(edit, text, nk_strlen(text));
	(void)usr;
}

static void nk_sdl_clipbard_copy(nk_handle usr, const char *text, int len){
	char *str = 0;
	(void)usr;
	if (!len) return;
	str = (char*)malloc((size_t)len+1);
	if (!str) return;
	memcpy(str, text, (size_t)len);
	str[len] = '\0';
	SDL_SetClipboardText(str);
	free(str);
}

NK_API int nk_sdl_init(gui_obj* gui){
	
	//gui_obj *gui = malloc(sizeof(gui_obj));
	
	if (gui){
		gui->ctx = malloc(sizeof(struct nk_context));
		gui->buf = calloc(1,FIXED_MEM);
		gui->last = calloc(1,FIXED_MEM);
		if((gui->ctx == NULL) || (gui->buf == NULL) || (gui->last == NULL)){
			return 0;
		}
	}
	else return 0;
	
	
	
	//nk_init_default(gui->ctx, font);
	//nk_init_fixed(gui->ctx, gui->buf, FIXED_MEM, font);
	nk_init_fixed(gui->ctx, gui->buf, FIXED_MEM, NULL);
	gui->ctx->clip.copy = nk_sdl_clipbard_copy;
	gui->ctx->clip.paste = nk_sdl_clipbard_paste;
	gui->ctx->clip.userdata = nk_handle_ptr(0);
	
	
	
	//struct tfont *ui_font = get_font_list(gui->font_list, "OpenSans-Regular.ttf");
	
	//struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
	
	//gui->ui_font.userdata = nk_handle_ptr(ui_font);
	//gui->ui_font.height = 11.0;
	
	gui->ui_font.width = nk_user_font_get_text_width2;
	
	nk_style_set_font(gui->ctx, &(gui->ui_font));
	
	nk_textedit_init_default(&(gui->text_edit));
	nk_textedit_init_default(&(gui->debug_edit));
	
	return 1;
}

NK_API int
nk_sdl_handle_event(gui_obj *gui, SDL_Window *win, SDL_Event *evt)
{
	struct nk_context *ctx = gui->ctx;

	/* optional grabbing behavior 
	if (ctx->input.mouse.grab) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		ctx->input.mouse.grab = 0;
	}
	else if (ctx->input.mouse.ungrab) {
		int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(win, x, y);
		ctx->input.mouse.ungrab = 0;
	}*/
	if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
	/* key events */
		int down = evt->type == SDL_KEYDOWN;
		const Uint8* state = SDL_GetKeyboardState(0);
		SDL_Keycode sym = evt->key.keysym.sym;
		
		if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
			nk_input_key(ctx, NK_KEY_SHIFT, down);
		else if (sym == SDLK_DELETE)
			nk_input_key(ctx, NK_KEY_DEL, down);
		else if (sym == SDLK_RETURN)
			nk_input_key(ctx, NK_KEY_ENTER, down);
		else if (sym == SDLK_TAB)
			nk_input_key(ctx, NK_KEY_TAB, down);
		else if (sym == SDLK_BACKSPACE)
			nk_input_key(ctx, NK_KEY_BACKSPACE, down);
		else if (sym == SDLK_HOME) {
			nk_input_key(ctx, NK_KEY_TEXT_START, down);
			nk_input_key(ctx, NK_KEY_SCROLL_START, down);
		}
		else if (sym == SDLK_END) {
			nk_input_key(ctx, NK_KEY_TEXT_END, down);
			nk_input_key(ctx, NK_KEY_SCROLL_END, down);
		}
		else if (sym == SDLK_PAGEDOWN) {
			nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
		}
		else if (sym == SDLK_PAGEUP) {
			nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
		}
		else if (sym == SDLK_z)
			nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_r)
			nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_c)
			nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
		else if ((sym == SDLK_v) && (state[SDL_SCANCODE_LCTRL])){
			nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
			/*
			const char *text = SDL_GetClipboardText();
			if (text){
				nk_rune str_uni;
				int glyph_size;
				char *curr = (char *)text;
				
				while ((curr - text) < strlen(text)) {
				
					glyph_size = nk_utf_decode(curr, &str_uni, 2);
					if (glyph_size){
						nk_input_unicode(ctx, str_uni);
						curr += glyph_size;
					}
					else {
						break;
					}
				}
				SDL_free(text);
				SDL_Delay(100);
			}*/
		}
		else if (sym == SDLK_x)
			nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_b)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_e)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_UP)
			nk_input_key(ctx, NK_KEY_UP, down);
		else if (sym == SDLK_DOWN)
			nk_input_key(ctx, NK_KEY_DOWN, down);
		else if (sym == SDLK_LEFT) {
			if (state[SDL_SCANCODE_LCTRL])
				nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
			else nk_input_key(ctx, NK_KEY_LEFT, down);
		} 
		else if (sym == SDLK_RIGHT) {
			if (state[SDL_SCANCODE_LCTRL])
				nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
			else nk_input_key(ctx, NK_KEY_RIGHT, down);
		} else return 0;
		return 1;
	} 
	else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
		/* mouse button */
		int down = evt->type == SDL_MOUSEBUTTONDOWN;
		const int x = evt->button.x, y = evt->button.y;
		
		if (evt->button.button == SDL_BUTTON_LEFT) {
			if (evt->button.clicks > 1)
				nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
			nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
		}
		else if (evt->button.button == SDL_BUTTON_MIDDLE)
			nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
		else if (evt->button.button == SDL_BUTTON_RIGHT)
			nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
		else return 0;
		return 1;
	}
	else if (evt->type == SDL_MOUSEMOTION) {
		/* mouse motion */
		if (ctx->input.mouse.grabbed) {
			int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
			nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
		}
		else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
		return 1;
	}
	else if (evt->type == SDL_TEXTINPUT) {
		/* text input */
		nk_glyph glyph;
		memcpy(glyph, evt->text.text, NK_UTF_SIZE);
		nk_input_glyph(ctx, glyph);
		return 1;
	}
	else if (evt->type == SDL_MOUSEWHEEL) {
		/* mouse wheel */
		if (nk_window_is_any_hovered(ctx)){
			nk_input_scroll(ctx,nk_vec2((float)evt->wheel.x,(float)evt->wheel.y));
			return 1;
		}
		else return 0;
	}
	else if (evt->type == SDL_DROPFILE) {      // In case if dropped file
		char *dropped_filedir = evt->drop.file;
		
		SDL_SetClipboardText(dropped_filedir);
		
		SDL_free(dropped_filedir);    // Free dropped_filedir memory
		
		nk_input_key(ctx, NK_KEY_PASTE, 1);
		SDL_Delay(50);
		nk_input_key(ctx, NK_KEY_PASTE, 0);
		
		return 1;
	}
	return 0;
}

NK_API
void nk_sdl_shutdown(gui_obj *gui)
{
	if(gui){
		if (gui->lua_script.L != NULL)
			lua_close(gui->lua_script.L);
		
		nk_textedit_free(&(gui->text_edit));
		nk_textedit_free(&(gui->debug_edit));
		
		gui_list_font_free (gui->ui_font_list);
		
		//nk_free(gui->ctx);
		free(gui->ctx);
		gui->ctx = NULL;
		//free(gui->font);
		gui->font = NULL;
		
		free(gui->buf);
		gui->buf = NULL;
		free(gui->last);
		gui->last = NULL;
		free(gui);
	}
		
    //memset(&sdl, 0, sizeof(sdl));
}

int gui_start(gui_obj *gui){
	if(!gui) return 0;
	
	gui->ui_font_list = gui_new_font (NULL);
	
	int i = 0, j = 0;
	bmp_color gray = {.r = 100, .g = 100, .b = 100, .a = 255};
	
	gui->ctx = NULL;
	gui->font = NULL;
	gui->buf = NULL;
	gui->last = NULL;
	
	gui->drawing = NULL;
	gui->element = NULL;
	gui->near_el = NULL;
	
	gui->main_w = 2048;
	gui->main_h = 2048;
	
	gui->win_w = 1200;
	gui->win_h = 710;
	
	gui->next_win_x = 0;
	gui->next_win_y = 0;
	gui->next_win_w = 0;
	gui->next_win_h = 0;
	
	gui->mouse_x = 0;
	gui->mouse_y = 0;
	
	gui->zoom = 20.0;
	gui->ofs_x = -11.0;
	gui->ofs_y = -71.0;
	gui->prev_zoom = 20.0;
	
	gui->user_x = 0.0;
	gui->user_y = 0.0;
	for (i = 0; i <10; i++){
		gui->step_x[i] =0.0;
		gui->step_y[i] =0.0;
	}
	gui->near_x = 0.0;
	gui->near_y = 0.0;
	gui->bulge = 0.0;
	gui->scale = 1.0;
	gui->angle = 0.0;
	gui->txt_h = 1.0;
	gui->rect_w = 0.0;
	
	gui->color_idx = 256;
	gui->lw_idx = 0;
	gui->t_al_v = 0;
	gui->t_al_h = 0;
	gui->layer_idx = 0;
	gui->ltypes_idx = 0;
	gui->t_sty_idx = 0;
	
	gui->step = 0;
	gui->user_flag_x = 0;
	gui->user_flag_y = 0;
	gui->lock_ax_x = 0;
	gui->lock_ax_y = 0;
	gui->user_number = 0;
	gui->keyEnter = 0;
	gui->draw = 0;
	gui->draw_tmp = 0;
	gui->draw_phanton = 0;
	gui->near_attr = 0;
	gui->text2tag = 0;
	gui->en_distance = 0;
	gui->entry_relative = 1;
	
	gui->action = NONE;
	gui->modal = SELECT;
	gui->prev_modal = SELECT;
	gui->ev = EV_NONE;
	gui->curr_attr_t = ATRC_END|ATRC_MID|ATRC_QUAD;
	
	gui->background = gray;
	
	gui->svg_curves = NULL;
	gui->svg_bmp = NULL;
	gui->preview_img = NULL;
	
	//gui->b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	//gui->b_icon_sel, gui->b_icon_unsel;
	
	gui->log_msg[0] = 0;
	gui->txt[0] = 0;
	gui->long_txt[0] = 0;
	gui->blk_name[0] = 0;
	gui->tag_mark[0] = 0;
	
	gui->sel_list = NULL;
	gui->phanton = NULL;
	//gui->list_do;
	gui->font_list = NULL;
	
	
	/* initialize the hatch pattern list */
	gui->hatch_fam_idx = 0;
	gui->hatch_idx = 0;
	gui->hatch_assoc = 0;
	gui->h_type = HATCH_USER;
	
	gui->user_patt.ang = 45.0;
	gui->user_patt.ox = 0.0; gui->user_patt.oy = 0.0;
	gui->user_patt.dx = 0.0; gui->user_patt.dy = 1;
	gui->user_patt.num_dash = 0;
	gui->user_patt.next = NULL;
	
	strncpy(gui->list_pattern.name, "USER_DEF", DXF_MAX_CHARS);
	strncpy(gui->list_pattern.descr, "User definied simple pattern", DXF_MAX_CHARS);
	gui->list_pattern.num_lines = 1;
	gui->list_pattern.lines = &(gui->user_patt);
	gui->list_pattern.next = NULL;
	
	strncpy(gui->hatch_fam.name, "USER_DEF", DXF_MAX_CHARS);
	strncpy(gui->hatch_fam.descr, "User definied simple pattern", DXF_MAX_CHARS);
	gui->hatch_fam.list = &(gui->list_pattern);
	gui->hatch_fam.next = NULL;
	gui->end_fam = &(gui->hatch_fam);
	
	gui->hatch_fam.next = dxf_hatch_family("Standard", "Internal standard pattern library", (char*)std_h_pat);
	if(gui->hatch_fam.next) gui->end_fam = gui->hatch_fam.next;
	//dxf_parse_patt((char*)acadiso_pat, &(gui->list_pattern));
	
	gui->patt_scale = 1.0;
	gui->patt_ang = 0.0;
	gui->patt_name[0] = 0;
	gui->patt_descr[0] = 0;
	gui->h_fam_name[0] = 0;
	gui->h_fam_descr[0] = 0;
	
	gui->dflt_fonts_path[0] = 0;
	
	gui->curr_path[0] = 0;
	gui->base_dir[0] = 0;
	gui->dwg_file[0] = 0;
	gui->dwg_dir[0] = 0;
	
	gui->paper_fam = 0;
	gui->sel_paper = 6;
	
	gui->keep_orig = 0;
	
	gui->lua_script.L = NULL;
	gui->lua_script.T = NULL;
	gui->lua_script.active = 0;
	gui->lua_script.dynamic = 0;
	gui->lua_script.status = LUA_OK;
	gui->lua_script.path[0] = 0;
	strncpy(gui->curr_script, "D:\\documentos\\cadzinho\\lua\\test.lua", MAX_PATH_LEN - 1);
	gui->script_timeout = 10.0;
	
	gui->script_win[0] = 0;
	gui->script_win_title[0] = 0;
	
	gui->script_dynamic[0] = 0;
	
	#if(0)
	#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
	strncpy(gui->dflt_fonts_path, "C:\\Windows\\Fonts\\", 5 * DXF_MAX_CHARS);
	#else
	strncpy(gui->dflt_fonts_path, "/usr/share/fonts/", 5 * DXF_MAX_CHARS);
	#endif
	#endif
	
	gui->font_list = list_new(NULL, PRG_LIFE);
	//if(add_font_list(gui->font_list, "romans.shx", gui->dflt_fonts_path)){
	//struct tfont * add_shp_font_list(list_node *list, char *name, char *buf);
	
	if(add_shp_font_list(gui->font_list, "romans.shx", (char *)shp_font_romans)){
		struct tfont *font = get_font_list(gui->font_list, "romans.shx");
		if (font){
			gui->dflt_font = font;
			//printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			//shp_font_print(font->data);
		}
	}
	if(add_shp_font_list(gui->font_list, "txt.shx", (char *)shp_font_txt)){
		/*
		struct tfont *font = get_font_list(gui->font_list, "txt.shx");
		if (font){
			//gui->dflt_font = font;
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			//shp_font_print(font->data);
		}
		*/
	}
	/*if(add_font_list(gui->font_list, "romans.shx", gui->dflt_fonts_path)){
		struct tfont *font = get_font_list(gui->font_list, "romans.shx");
		if (font){
			gui->dflt_font = font;
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			shp_font_print(font->data);
		}
	}*/
	//if(add_font_list(gui->font_list, "OpenSans-Regular.ttf", gui->dflt_fonts_path)){
		/*
		struct tfont *font = get_font_list(gui->font_list, "OpenSans-Regular.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			//gui->dflt_font = font;
		}
		*/
	//}
	
	
	//if(add_font_list(gui->font_list, "NotoSans-Regular.ttf", gui->dflt_fonts_path)){
		/*struct tfont *font = get_font_list(gui->font_list, "Lato-Hairline.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			//gui->ui_font = font;
		}*/
	//}
	/*
	if(add_font_list(gui->font_list, "arialbd.ttf", gui->dflt_fonts_path)){
		struct tfont *font = get_font_list(gui->font_list, "arialbd.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
		}
	}
	if(add_font_list(gui->font_list, "Lato-Light.ttf", gui->dflt_fonts_path)){
		struct tfont *font = get_font_list(gui->font_list, "Lato-Light.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
		}
	}
	if(add_font_list(gui->font_list, "ProggyClean.ttf", gui->dflt_fonts_path)){
		struct tfont *font = get_font_list(gui->font_list, "ProggyClean.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
		}
	}
	if(add_font_list(gui->font_list, "Roboto-LightItalic.ttf", "C:\\Windows\\Fonts\\;E:\\documentos\\cadzinho\\res\\fonts\\tt\\Roboto")){
		struct tfont *font = get_font_list(gui->font_list, "Roboto-LightItalic.ttf");
		if (font){
			printf("\n------------------------------------------\n         FONT INIT OK - type = %d\n---------------------------------\n", font->type);
			
		}
	}
	*/
	
	for (i = 0; i < DRWG_HIST_MAX; i++)
		gui->drwg_hist[i][0] = 0;
	gui->drwg_hist_size = 0;
	gui->drwg_hist_pos = 0;
	gui->drwg_hist_wr = 0;
	gui->drwg_hist_head = 0;
	
	gui->show_edit_text = 0;
	
	gui->num_brk_pts = 0;
	
	return 1;
}

int gui_tstyle(gui_obj *gui){
	if(!gui) return 0;
	
	int i = 0;
	
	int num_tstyles = gui->drawing->num_tstyles;
	dxf_tstyle *t_sty = gui->drawing->text_styles;
	struct tfont * font;
		
	for (i = 0; i < num_tstyles; i++){
		if(font = add_font_list(gui->font_list, t_sty[i].file, gui->dflt_fonts_path)){
			t_sty[i].font = font;
			//if (font->type == FONT_SHP) shp_font_print((shp_typ *) font->data);
		}
		else{
			t_sty[i].font = gui->dflt_font;
			strncpy(t_sty[i].subst_file, gui->dflt_font->name, DXF_MAX_CHARS);
		}
		
	}
	
	
	
	return 1;
}

int gui_tstyle2(gui_obj *gui, dxf_drawing *drawing){
	if(!gui) return 0;
	
	int i = 0;
	
	int num_tstyles = drawing->num_tstyles;
	dxf_tstyle *t_sty = drawing->text_styles;
	struct tfont * font;
		
	for (i = 0; i < num_tstyles; i++){
		if(font = add_font_list(gui->font_list, t_sty[i].file, gui->dflt_fonts_path)){
			t_sty[i].font = font;
			//if (font->type == FONT_SHP) shp_font_print((shp_typ *) font->data);
		}
		else{
			t_sty[i].font = gui->dflt_font;
			strncpy(t_sty[i].subst_file, gui->dflt_font->name, DXF_MAX_CHARS);
		}
		
	}
	
	
	
	return 1;
}

//#endif
