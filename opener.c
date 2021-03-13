//
// opener.c
// Open URLS in C
//
// MIT licensed.
// Copyright (c) Abraham Hernandez <abraham@abranhe.com>
//

#include <cstring>
#include <cstdlib>


static const char * operating_system()
{
    #ifdef _WIN32
    return "win32";
    #elif _WIN64
    return "win64";
    #elif __unix || __unix__
    return "unix";
    #elif __APPLE__ || __MACH__
    return "macOS";
    #elif __linux__
    return "linux";
    #elif __FreeBSD__
    return "freeBSD";
    #else
    return "other";
    #endif
}

char *
create_cmd(const char * cmd, const char * link) {
    char * url = (char *)malloc(strlen(cmd) + strlen(link) + 1);
    strcpy(url, cmd);
    strcat(url, " ");
    strcat(url, link);
    return url;
}

int
opener(const char *url)
{
    const char *platform = operating_system();
    const char *cmd = NULL;

    // Hanlde macOS
    if (!strcmp(platform, "macOS")) {
      cmd = "open";

    // Handle Windows
    } else if (!strcmp(platform, "win32") || !strcmp(platform, "win64")) {
      cmd = "start";

    // Handle Linux, Unix, etc
    } else if (!strcmp(platform, "unix")
      || !strcmp(platform, "linux") 
      || !strcmp(platform, "freeBSD") 
      || !strcmp(platform, "other")) {
      cmd = "xdg-open";
    }

    char *script = create_cmd(cmd, url);

    system(script);
    free(script);
    return 0;
}
