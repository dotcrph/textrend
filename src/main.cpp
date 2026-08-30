#include "main.hpp"

#include <chrono>
#include <fstream>
#include <iostream>

#include "args.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "screen.hpp"
#include "terminal.hpp"
#include "transform.hpp"
#include "utils.hpp"
#include "obj_parser.hpp"
#include "rasterizer.hpp"

Mesh *meshWS        = nullptr;
Mesh *meshDisplayed = nullptr;

int main(int argc, char *argv[])
{
    bool good;

    // Read args
    std::string objPath;
    good = args::read(argc, argv, objPath);

    if (!good) {
        quit();
        return 1;
    }

    // Open the OBJ file and parse the contents

    // Opening in binary mode because I am handling 
    // line endings manually inside obj::parse()
    std::ifstream objFile(objPath, std::ios::binary);

    if (!objFile.is_open()) {
        logger::error("Could not open file '%s' for reading", objPath.c_str());

        quit();
        return 1;
    }

    meshWS = new Mesh();

    good = obj::parse(objFile, meshWS);
    objFile.close();

    if (!good) {
        quit();
        return 1;
    }

    // Set up terminal environment
    good = terminal::initialize();

    if (!good) {
        quit();
        return 1;
    }

    // Initialize buffers
    good = screen::initialize();

    if (!good) {
        quit();
        return 1;
    }

    // Main loop
    Camera camera;
    camera.center(meshWS, screen::getWidthDivHeight());

    meshDisplayed = new Mesh();

    // NOTE: Multiplying the original size to account for new vertices 
    // generated after clipping (using the worst case scenario for each)
    meshDisplayed->verts.reserve(meshWS->verts.size() * 2);
    meshDisplayed->faces.reserve(meshWS->faces.size() * 4);

    auto lastFrameTime = std::chrono::steady_clock::now();

    bool running = true;
    while (running) {
        // Init
        double deltaTime = updateDeltaTime(lastFrameTime);

        meshDisplayed->verts.clear();
        meshDisplayed->faces.clear();

        // Update
        if (terminal::shouldResizeWindow()) {
            screen::cleanup();
            screen::initialize();
        }

        // NOTE: terminal::update() resets the window 
        // resizing flag, so I'm calling it after
        terminal::update();

        if (terminal::getKeyHeld('\x1b'))
            running = false;

        camera.update(deltaTime);
        rasterizer::update();

        // Draw
        transform::worldToClip(
            meshWS, 
            meshDisplayed, 
            camera, 
            screen::getWidthDivHeight()
        );

        transform::clipToNDC(
            meshWS, 
            meshDisplayed, 
            camera
        );

        screen::clear();
        rasterizer::rasterize(meshDisplayed, camera);

        const char *debugString = str::quickFormat(
            "FPS: %f\nVerts: %zu\nFaces: %zu", 
            1 / deltaTime,
            meshDisplayed->verts.size(),
            meshDisplayed->faces.size()
        );

        screen::drawText(5, 2, debugString);
        screen::print();
    }

    quit();
    return 0;
}

double updateDeltaTime(
    std::chrono::time_point<std::chrono::steady_clock> &lastFrameTime)
{
    auto currentFrameTime = std::chrono::steady_clock::now();

    std::chrono::duration<double> frameTimeDuration
        = currentFrameTime - lastFrameTime;

    lastFrameTime = currentFrameTime;

    return frameTimeDuration.count();
}

void quit()
{
    terminal::cleanup();
    screen::cleanup();

    delete meshWS;
    delete meshDisplayed;
}
