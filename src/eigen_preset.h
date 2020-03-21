// eigen_preset.h
#pragma once
#include <Eigen/Dense>

#define DefMatrix(num, postfix) using Matrix##num##postfix = Eigen::Matrix##num##postfix
#define DefVector(num, postfix) using Vector##num##postfix = Eigen::Vector##num##postfix

//using Vector3f = Eigen::Vector3f; ... 
DefVector(2, f);
DefVector(3, f);
DefVector(4, f);
DefVector(2, d);
DefVector(3, d);
DefVector(4, d);

//using Matrix3f = Eigen::Vector3f; ... 
DefMatrix(2, f);
DefMatrix(3, f);
DefMatrix(4, f);
DefMatrix(2, d);
DefMatrix(3, d);
DefMatrix(4, d);

using VectorRGB = Eigen::Matrix<unsigned char, 3, 1>;
using Vector3u = VectorRGB;
using VectorRGBA = Eigen::Matrix<unsigned char, 4, 1>;
using Vector4u = VectorRGBA;

