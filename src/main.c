#include "application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

int main(void) {

    t_Application app;

    app_init(&app);

    app_run(&app);

    app_end(&app);

    return 0;
}
