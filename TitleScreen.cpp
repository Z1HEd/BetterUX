#include <4dm.h>
#include <execution>
#include "Config.h"
#include <chrono>

using namespace fdm;

static std::mutex chunkQueueMutex;

$hook(bool, WorldTitleScreen, loadCube, const glm::vec4& pos, uint8_t _distance) {
    // distance is unused in the original code, it is also probably undefined
    if (!self->chunksReady) return false;
    self->chunksReady = false;

    int cx = int(std::floor(pos.x * 0.125f));
    int cw = int(std::floor(pos.w * 0.125f));
    int cz = int(std::floor(pos.z * 0.125f));
    int R = titleScreenWorldRenderDistance;

    if (R == 0) {
        self->chunksReady = true;
        return true;
    }

    std::vector<glm::ivec3> coords;
    coords.reserve((2 * R + 1) * (2 * R + 1) * (2 * R + 1)); // overalocate a bit

    for (int x = cx - R; x <= cx + R; ++x)
        for (int z = cz - R; z <= cz + R; ++z)
            for (int w = cw - R; w <= cw + R; ++w) {
                // Spherical area 
                if (glm::dot(glm::vec3{ cx,cz,cw } - glm::vec3{ x,z,w },
                    glm::vec3{ cx,cz,cw } - glm::vec3{ x, z, w }) >
                    titleScreenWorldRenderDistance * titleScreenWorldRenderDistance)
                    continue;
                coords.emplace_back(x, z, w);
            }

    if (coords.empty()) {
        self->chunksReady = true;
        return true;
    }

    unsigned nThreads = std::max(1u, std::thread::hardware_concurrency());
    size_t total = coords.size();
    size_t perThread = (total + nThreads - 1) / nThreads;

    std::vector<std::vector<std::unique_ptr<Chunk>>> localChunks(nThreads);
    std::vector<std::vector<Chunk*>>                 localPtrs(nThreads);
    std::vector<std::thread>                         allocThreads;
    allocThreads.reserve(nThreads);

    for (unsigned t = 0; t < nThreads; ++t) {
        size_t start = t * perThread;
        size_t end = std::min(start + perThread, total);
        if (end <= start)continue;
        allocThreads.emplace_back([&, t, start, end]() {
            auto& lc = localChunks[t];
            auto& lp = localPtrs[t];
            lc.reserve(end - start);
            lp.reserve(end - start);
            for (size_t i = start; i < end; ++i) {
                const auto& c = coords[i];
                auto up = std::make_unique<Chunk>(c);
                lp.push_back(up.get());
                lc.push_back(std::move(up));
            }
            });
    }

    for (auto& th : allocThreads) th.join();

    {
        std::lock_guard<std::mutex> lk(chunkQueueMutex);
        for (unsigned t = 0; t < nThreads; ++t) {
            auto& lc = localChunks[t];
            auto& lp = localPtrs[t];
            for (size_t i = 0; i < lc.size(); ++i) {
                const glm::ivec3& coord = lc[i]->pos;
                self->chunks.emplace(coord, std::move(lc[i]));
                self->chunksToLoad.push_back(lp[i]);
            }
        }
    }

    std::thread([self]() { self->loadChunks(); }).detach();
    return true;
}

$hook(void, WorldTitleScreen, loadChunks) {
    std::vector<Chunk*> work;
    {
        std::lock_guard<std::mutex> lk(chunkQueueMutex);
        work.swap(self->chunksToLoad);
    }
    size_t total = work.size();
    if (total == 0) {
        self->chunksReady = true;
        return;
    }

    unsigned nThreads = std::max(1u, std::thread::hardware_concurrency());

    // Load chunks
    {
        std::atomic<size_t> nextIdx{ 0 };
        std::vector<std::thread> loaders;
        loaders.reserve(nThreads);

        for (unsigned t = 0; t < nThreads; ++t) {
            loaders.emplace_back([&]() {
                while (true) {
                    size_t i = nextIdx.fetch_add(1, std::memory_order_relaxed);
                    if (i >= total) break;
                    self->loadChunk(work[i], self->shadows);
                }
                });
        }
        for (auto& th : loaders) th.join();
    }

    // Generate meshes
    {
        std::atomic<size_t> nextIdx{ 0 };
        std::vector<std::thread> meshers;
        meshers.reserve(nThreads);

        for (unsigned t = 0; t < nThreads; ++t) {
            meshers.emplace_back([&]() {
                while (true) {
                    size_t i = nextIdx.fetch_add(1, std::memory_order_relaxed);
                    if (i >= total) break;
                    Chunk* chunk = work[i];
                    if (chunk->loaded._Storage._Value) {
                        self->generateMesh(chunk, self->smoothLighting, self->shadows, self->lights);
                    }
                }
                });
        }
        for (auto& th : meshers) th.join();
    }

    // Update cache
    for (Chunk* chunk : work) {
        self->updateChunkCache(chunk, self->smoothLighting, self->shadows);
    }

    self->chunksReady = true;
}
