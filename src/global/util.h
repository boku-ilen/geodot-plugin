#pragma once

struct ExtentData {
    ExtentData(double left, double right, double top, double down)
        : left(left), right(right), top(top), down(down) {}

    ExtentData() = default;

    double left;
    double right;
    double top;
    double down;
};