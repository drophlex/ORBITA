#include "Engine.h"
#include <cassert>
int main() {
    for(int k=1;k<=16;++k) {
        int count=0;
        for(int i=0;i<16;++i) count+=OrbitaEngine::hit(i,k);
        assert(count==k);
    }
    for(double sr : {44100.,48000.,96000.}) {
        OrbitaEngine e; e.prepare(sr);
        assert(e.render(2,1,.2,.7)==0);
        e.trigger(60,.85);
        double energy=0;
        for(int i=0;i<48000;i++) {
            float y=e.render(2.37,2.5,.25,.75);
            assert(std::isfinite(y) && std::abs(y)<=.75);
            energy+=y*y;
        }
        assert(energy>1);
        e.lift(60);
        for(int i=0;i<sr*2;i++) e.render(2,2,.25,.75);
        assert(e.render(2,2,.25,.75)==0);
    }
}
