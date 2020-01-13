/*
	SCE sign driver
	Charles MacDonald
	Based on clock.cc
*/

#include <stdint.h>
#include <limits.h>
#include "led-matrix.h"
#include "graphics.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <list>

using namespace rgb_matrix;
using namespace std;


volatile bool interrupt_received = false;

static void InterruptHandler(int signo) 
{
	interrupt_received = true;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

typedef struct
{
	string font_filename;
	uint32_t font_color;
	uint32_t border_color;
	uint32_t background_color;
    uint32_t shadow_color;
	uint32_t speed;
	uint32_t brightness;
	string message;
} sign_cfg_t;

sign_cfg_t sign_cfg;
	
void opt_set_font_filename(string parameter)
{
	sign_cfg.font_filename = parameter;
}

void opt_set_text(string parameter)
{
	sign_cfg.message = parameter;
}

void opt_set_brightness(string parameter)
{
	sign_cfg.brightness = strtoul(parameter.c_str(), NULL, 10);
}

void opt_set_speed(string parameter)
{
	sign_cfg.speed = strtoul(parameter.c_str(), NULL, 10);
}

void opt_set_background_color(string parameter)
{
	sign_cfg.background_color = strtoul(parameter.c_str(), NULL, 16);
}

void opt_set_font_color(string parameter)
{
	sign_cfg.font_color = strtoul(parameter.c_str(), NULL, 16);
}

void opt_set_border_color(string parameter)
{
	sign_cfg.border_color = strtoul(parameter.c_str(), NULL, 16);
}

void opt_set_shadow_color(string parameter)
{
	sign_cfg.shadow_color = strtoul(parameter.c_str(), NULL, 16);
}

typedef struct
{
	const char *key;
	void (*callback)(string parameter);
} opt_entry;

const opt_entry opt_list[] =
{
	{"--set-font-filename"      , opt_set_font_filename},
	{"--set-text"				, opt_set_text},
	{"--set-brightness"			, opt_set_brightness},
	{"--set-speed"				, opt_set_speed},
	{"--set-background-color"	, opt_set_background_color},
	{"--set-font-color"			, opt_set_font_color},
	{"--set-border-color"		, opt_set_border_color},
	{"--set-shadow-color"		, opt_set_shadow_color},
	{NULL						, NULL},
};

int process_args(int argc, char *argv[])
{
	list<string> args;
	for(int i = 1; i < argc; i++)
	{
		args.push_back(string(argv[i]));
	}

	for(auto it = args.begin(); it != args.end();)
	{
		bool matched = false;
		bool consumed = false;
		string arg = *it;
		string parameter;

		for(int i = 0; opt_list[i].key != NULL && !matched; i++)
		{
			if(strcmp(opt_list[i].key, arg.c_str()) == 0)
			{
				matched = true;
				it = args.erase(it);
				if(it == args.end())
				{
					printf("Missing parameter to option `%s'.\n", arg.c_str());
					return 0;
				}
				string param = *it;
				if(opt_list[i].callback != NULL)
				{
					opt_list[i].callback(param);
				}
				it = args.erase(it);
				consumed = true;
			}
		}

		if(!consumed)
			it++;

		if(!matched)
		{
			printf("Error, unknown option `%s'.\n", arg.c_str());
			return 0;
		}

	}
	return 1;
} /* process_args */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[]) 
{
	RGBMatrix::Options matrix_options;
  	rgb_matrix::RuntimeOptions runtime_opt;

	const char *time_format = "%H:%M";
	Color color(255, 255, 0);
	Color bg_color(0, 0, 0);

	const char *bdf_font_file = NULL;
	int x_orig = 0;
	int y_orig = 0;
	int brightness = 100;
	int letter_spacing = 0;

    printf("SCE Sign Control Utility\n");
    printf("Project contributors:\n");
    printf("* Will Zegers       : Wireless interface.\n");
    printf("* Wilson Luc        : Mechanical design and construction.\n");
    printf("* Charles MacDonald : Software and circuit design.\n");
    printf("* Khalil Estell     : Python server software and Web interface.\n");
    printf("Available parameters:\n");
    for(int i = 0; opt_list[i].key != NULL; i++)
    {
        printf("%s\n", opt_list[i].key);
    }
    printf("\n");
    
	/* Set defaults */
	sign_cfg.font_filename = "fonts/9x18B.bdf";
	sign_cfg.font_color = 0x00FF00;
	sign_cfg.border_color = 0xFF0000;
	sign_cfg.background_color = 0x0000FF;
    sign_cfg.shadow_color = 0x000000;
	sign_cfg.speed = 8;
	sign_cfg.brightness = 50;
	sign_cfg.message = "Welcome to San Jose State University";

	if(!process_args(argc, argv))
	{
		; /* Invalid args */
		exit(1);
	}

	if(sign_cfg.font_filename == "")
	{
		printf("No font specified.\n");
		return 0;
	}
	else
	{
		bdf_font_file = strdup(sign_cfg.font_filename.c_str());
	}

	/* Configure RGB matrix chain */
	matrix_options.chain_length = 4;
	matrix_options.rows = 16;
	matrix_options.cols = 32;
	matrix_options.pwm_bits = 10;
	matrix_options.hardware_mapping = "adafruit-hat-pwm";
	//matrix_options.brightness = 100;

    RGBMatrix *matrix;
	rgb_matrix::Font font;
    FrameCanvas *offscreen;
    FrameCanvas *rendered_message;

	const Color text_color(
		(sign_cfg.font_color >> 16) & 0xFF,
		(sign_cfg.font_color >>  8) & 0xFF,
		(sign_cfg.font_color >>  0) & 0xFF
		);

    const Color background_color(
        (sign_cfg.background_color >> 16) & 0xFF,
        (sign_cfg.background_color >>  8) & 0xFF,
        (sign_cfg.background_color >>  0) & 0xFF
        );

    const Color border_color(
        (sign_cfg.border_color >> 16) & 0xFF,
        (sign_cfg.border_color >>  8) & 0xFF,
        (sign_cfg.border_color >>  0) & 0xFF
        );

    const Color shadow_color(
        (sign_cfg.shadow_color >> 16) & 0xFF,
        (sign_cfg.shadow_color >>  8) & 0xFF,
        (sign_cfg.shadow_color >>  0) & 0xFF
        );
	
    /* Load BDF font file */
  	if (!font.LoadFont(bdf_font_file)) 
	{
    	fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    	return 1;  	
	}

	/* Make RGB matrix */
	matrix = rgb_matrix::CreateMatrixFromOptions(matrix_options, runtime_opt);
	if (matrix == NULL)
	{
	    return 1;
	}

	/* Set brightness */
	brightness = sign_cfg.brightness;

    /* Validate brightness */
  	if (brightness < 1 || brightness > 100) 
	{
    	fprintf(stderr, "Brightness is outside usable range.\n");
    	return 1;
  	}

    /* Set brightness */
	matrix->SetBrightness(brightness);

  	offscreen = matrix->CreateFrameCanvas();
	char text_buffer[256];

  	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

  	int frame_count = 0;
	int font_width = 9; // extract from font filename
	const int panel_width = 128;
	const int panel_height = 16;

	sprintf(text_buffer, sign_cfg.message.c_str());

    int codepoint_width_accum = 0;
    int codepoint_width_min = INT_MAX;
    int codepoint_width_max = INT_MIN;
    for(int i = 0; i < sign_cfg.message.size(); i++)
    {
        char ch = sign_cfg.message[i];
        int width = font.CharacterWidth(ch);
        codepoint_width_accum += width;
        if(width > codepoint_width_max)
            codepoint_width_max = width;
        if(width < codepoint_width_min)
            codepoint_width_min = width;
    }

    printf("Message information:\n");
    printf("Text:  [%s]\n", sign_cfg.message.c_str());
    printf("Size:  %d characters\n", sign_cfg.message.size());
    printf("Width: %d pixels wide (avg. %d px/character, %d px min., %d px max.)\n", 
        codepoint_width_accum, 
        codepoint_width_accum / sign_cfg.message.size(),
        codepoint_width_min,
        codepoint_width_max
        );

	int message_length = strlen(text_buffer);
	int message_width = message_length * font_width;
	int scroll_limit = panel_width + message_width;

	int scroll_position = 0;
	int scroll_update_count = 0;
	int scroll_update_interval = sign_cfg.speed;

    int border_top_y = 0;
    int border_bottom_y = panel_height - 1;

	while (!interrupt_received) 
	{
		/* Clear buffer with background color */
		offscreen->Fill(background_color.r, background_color.g, background_color.b);
   
		/* Draw top and bottom border */
        DrawLine(offscreen, 0, border_top_y, panel_width - 1, border_top_y, border_color);
        DrawLine(offscreen, 0, border_bottom_y, panel_width - 1, border_bottom_y, border_color);      

		/* Calculate text X position */
		int fb_xpos = panel_width - scroll_position;

		/* Calculate text Y position */	
		int fb_ypos = (panel_height - font.height()) / 2;

		/* Draw text (drop shadow) */
		rgb_matrix::DrawText(
			offscreen, 
			font, 
			fb_xpos + 1, 
			fb_ypos + font.baseline() + 1,
			shadow_color, 
			NULL, 
			text_buffer,
			letter_spacing
		);

		/* Draw text */
		rgb_matrix::DrawText(
			offscreen, 
			font, 
			fb_xpos, 
			fb_ypos + font.baseline(),
			text_color,
			NULL, 
			text_buffer,
			letter_spacing
		);

      	// Atomic swap with double buffer
		offscreen = matrix->SwapOnVSync(offscreen);

		/* Update frame count */
		++frame_count;

		/* Update scroll counter */
		if(++scroll_update_count >= scroll_update_interval)
		{
			scroll_update_count = 0;
			scroll_position++;
		}

		/* Update scroll position */
		if(scroll_position >= scroll_limit)
		{
			scroll_position = 0;
		}
	}

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
  return 0;
}




