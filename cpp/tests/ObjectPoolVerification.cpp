#include "EchoheartsObjectPool.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using Echohearts::Core::ObjectPool;

namespace {

void Require(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct PooledEffectState {
    int intensity{0};
    bool active{false};
};

} // namespace

int main()
{
    try {
        ObjectPool<PooledEffectState> pool(
            2,
            [](PooledEffectState& state) {
                state.intensity = 0;
                state.active = false;
            });

        Require(pool.Capacity() == 2, "initial capacity mismatch");
        Require(pool.Available() == 2, "initial available count mismatch");
        Require(pool.InUse() == 0, "initial in-use count mismatch");

        PooledEffectState* a = pool.Acquire();
        PooledEffectState* b = pool.Acquire();
        Require(a != nullptr && b != nullptr && a != b, "pool returned invalid or duplicate objects");
        Require(pool.InUse() == 2 && pool.Available() == 0, "acquire accounting mismatch");

        a->intensity = 99;
        a->active = true;

        PooledEffectState* c = pool.Acquire();
        Require(c != nullptr, "pool failed to grow");
        Require(pool.Capacity() == 3, "pool growth capacity mismatch");

        Require(pool.Release(a), "valid release failed");
        Require(!pool.Release(a), "double release was incorrectly accepted");

        PooledEffectState foreign;
        Require(!pool.Release(&foreign), "foreign object was incorrectly accepted");

        PooledEffectState* reused = pool.Acquire();
        Require(reused == a, "released object was not reused as expected");
        Require(reused->intensity == 0 && !reused->active, "reset callback did not restore object state");

        Require(pool.Owns(a) && pool.Owns(b) && pool.Owns(c), "ownership tracking failed");
        Require(!pool.Owns(&foreign), "foreign ownership check failed");

        pool.ReleaseAll();
        Require(pool.InUse() == 0, "ReleaseAll left objects marked in use");
        Require(pool.Available() == pool.Capacity(), "ReleaseAll did not restore all objects to free list");

        std::cout << "Echohearts ObjectPool verification: passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Echohearts ObjectPool verification FAILED: " << ex.what() << '\n';
        return 1;
    }
}
