#pragma once
#include "Individual.h"
#include <iostream>


class Draw{
public:
    Draw(int width, int height, Individual individual){
        std::cout << "Drawing individual with image size " << width << "x" << height << std::endl;
    }
    ~Draw() = default;
};