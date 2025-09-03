#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "config.h"

void ConfigRead(Config *conf, char *path) 
{
	FILE *pF = fopen(path, "r");
	if(!pF) 
	{
		printf("ERROR: Could not open configuration file at: %s\n", path);	
		ConfigSetDefault(conf);
		return;
	}

	puts("Reading configuration file...");
	
	char line[64];
	while(fgets(line, sizeof(line), pF)) 
	{
		ConfigParseLine(conf, line);		
		//printf("%s", line);
	}
	
	fclose(pF);
	ConfigPrintValues(conf);
}

void ConfigParseLine(Config *conf, char *line) 
{
	// Ignore commented lines
	if(line[0] == '#') return;
	
	// Split value and option key
	char *eq = strchr(line, '='); 
	if(!eq) return;	

	*eq = '\0';
	char *key = line;
	char *val = eq + 1;

	if(streq(key, "window_width")) 
	{
		if(streq(val, AUTO)) 
			conf->windowWidth = GetMonitorWidth(0);
		else 
			sscanf(val, "%d", &conf->windowWidth);
	} 
	else if(streq(key, "window_height")) 
	{
		if(streq(val, AUTO)) 
			conf->windowHeight = GetMonitorHeight(0);
		else
			sscanf(val, "%d", &conf->windowHeight);
	} 
	else if(streq(key, "refresh_rate")) 
	{
		if(streq(val, AUTO)) 
			conf->refreshRate = GetMonitorRefreshRate(0);
		else 
			sscanf(val, "%f", &conf->refreshRate);
	}
}

void ConfigSetDefault(Config *conf) 
{
	*conf = (Config) {
		.windowWidth  = CONFIG_DEFAULT_WW,
		.windowHeight = CONFIG_DEFAULT_WH,
		.refreshRate  = CONFIG_DEFAULT_RR
	};

	ConfigPrintValues(conf);
}

void ConfigPrintValues(Config *conf) 
{
	printf("resolution: %dx%d\n", conf->windowWidth, conf->windowHeight);
	printf("refresh rate: %f\n", conf->refreshRate);
}

