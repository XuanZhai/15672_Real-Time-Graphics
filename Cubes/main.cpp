#include <iostream>
#include "Cube.h"
#include "Lambertian.h"

int main(int argc, char** argv) {

    Cube* obj = new Lambertian();
    obj->ReadFile(argv[1]);
    obj->Processing(1000,64,64);

    delete obj;

    return 0;
}
