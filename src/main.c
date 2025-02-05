#include "application.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main(void) {

    t_Application app;

    app_init(&app);

    app_run(&app);

    app_end(&app);

    return 0;
}
