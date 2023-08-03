#include "Matrix4.hpp"

Matrix4 &Matrix4::invert() {
  double t[4][4] = {};
  for (int i = 0; i < 4; i++)
    t[i][i] = 1;
  for (int i = 0; i < 4; i++) {
    // find a pivot
    int pivot = i;
    while (pivot < 4 && m[pivot][i] < EPS)
      pivot++;
    if (pivot == 4)
      throw "Matrix4::invert(): matrix is singular";
    // swap rows
    double p = m[pivot][i];
    for (int j = 0; j < 4; j++) {
      std::swap(m[i][j], m[pivot][j]);
      m[i][j] /= p;
      std::swap(t[i][j], t[pivot][j]);
    }
    // subtract rows
    for (int j = i + 1; j < 4; j++) {
      if (i == j)
        continue;
      double p = m[j][i];
      for (int k = 0; k < 4; k++) {
        m[j][k] -= p * m[i][k];
        t[j][k] -= p * t[i][k];
      }
    }
  }
  std::swap(m, t);
  return *this;
}
