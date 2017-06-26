#ifndef EOLO_TRIGONOMETRY_MATH_H
#define EOLO_TRIGONOMETRY_MATH_H

#define PI 3.14159265358979f

#define radians(x) ((x)*PI/180.0)
#define degrees(x) ((x)*180.0/PI)


double cos(double value);
double sin(double value);

double hypot(double x, double y);
double atan2(double x, double y);

#endif //EOLO_TRIGONOMETRY_MATH_H
