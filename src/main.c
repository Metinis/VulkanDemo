#include "application.h"

int main(void) {

    t_Application app;

    app_init(&app);

    app_run(&app);

    app_end(&app);

    return 0;
}
