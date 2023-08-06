#include "miao/core/transform.hpp"
#include "miao/core/vec3.hpp"
#include "miao/integrators/integrator.hpp"

#include <iostream>
using namespace std;
using namespace miao;

int main() {
  vec3 joe{1, 2, 3};
  cout << joe.ts() << " frank\n";

  Transformation t =
      Transformation::scale(1, 2, 3) * Transformation::scale(3, 2, 1);
  cout << t(joe).ts() << " frank2!\n";

  Transformation ta = Transformation::rotateX(270);
  vec3 z{69, 420, 69};
  cout << ta(z).ts() << " frank3\n";
}
