#include "Application.h"

int main() {
    Application app("test app", 1024, 768);
    if (!app.initApp())
        return -1;
    return app.loop();
}
