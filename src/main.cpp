/// \file main.cpp
//

#include "application.h"
#include "utils/image_loader.h"
#include <SDL3/SDL_main.h>
#include <chrono>
#include <memory>
#include <string_view>
#include <utility>

using namespace Multor;

BaseMesh *TestMesh, *TestMesh2;

int main(int argc, char* args[])
{
    //getchar();
    try
        {
            Application app;

            auto spawnDebugCubes = [&app]() -> std::pair<std::shared_ptr<Vulkan::Mesh>,
                                                         std::shared_ptr<Vulkan::Mesh>>
            {
                const float cubePos[72] = {
                    // front
                    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                    -0.5f, 0.5f,  0.5f,
                    // back
                    0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
                    0.5f,  0.5f,  -0.5f,
                    // left
                    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                    -0.5f, 0.5f,  -0.5f,
                    // right
                    0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
                    0.5f,  0.5f,  0.5f,
                    // top
                    -0.5f, 0.5f,  0.5f,  0.5f,  0.5f, 0.5f,  0.5f, 0.5f, -0.5f,
                    -0.5f, 0.5f,  -0.5f,
                    // bottom
                    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,
                    -0.5f, -0.5f, 0.5f};

                const float cubeTexCoords[48] = {
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

                std::vector<std::uint32_t> indices = {
                    0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11,
                    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22,
                    20, 22, 23};

                std::shared_ptr<Multor::BaseTexture> tex =
                    std::make_shared<BaseTexture>(
                        std::string("Diff"), std::string("core"),
                        Texture_Types::Diffuse,
                        std::vector<std::shared_ptr<Image> >(
                            {ImageLoader::LoadTexture("./Res/matrix.jpg")}));

                auto* testMesh = new BaseMesh(
                    std::make_unique<Vertexes>(24, &cubePos[0],
                                               std::vector<std::uint32_t>(indices),
                                               nullptr, &cubeTexCoords[0]),
                    nullptr, std::vector<std::shared_ptr<BaseTexture> >({tex}));
                auto* testMesh2 = new BaseMesh(
                    std::make_unique<Vertexes>(24, &cubePos[0], std::move(indices),
                                               nullptr, &cubeTexCoords[0]),
                    nullptr, std::vector<std::shared_ptr<BaseTexture> >({tex}));

                return {app.GetRenderer()->AddMesh(testMesh),
                        app.GetRenderer()->AddMesh(testMesh2)};
            };

            auto spawnDebugGroundPlane =
                [&app]() -> std::shared_ptr<Vulkan::Mesh>
            {
                const float planeSize = 20.0f;
                const float y         = -1.0f;
                const float planePos[12] = {
                    -planeSize, y, -planeSize, planeSize, y, -planeSize,
                    planeSize,  y, planeSize,  -planeSize, y, planeSize};
                const float planeNormals[12] = {
                    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
                const float planeTexCoords[8] = {
                    0.0f, 0.0f, 8.0f, 0.0f, 8.0f, 8.0f, 0.0f, 8.0f};

                std::vector<std::uint32_t> planeIndices = {0, 1, 2, 0, 2, 3};
                std::shared_ptr<Multor::BaseTexture> tex =
                    std::make_shared<BaseTexture>(
                        std::string("Ground"), std::string("core"),
                        Texture_Types::Diffuse,
                        std::vector<std::shared_ptr<Image> >(
                            {ImageLoader::LoadTexture("./Res/wall.jpg")}));

                auto* ground = new BaseMesh(
                    std::make_unique<Vertexes>(4, &planePos[0],
                                               std::move(planeIndices),
                                               &planeNormals[0],
                                               &planeTexCoords[0]),
                    nullptr, std::vector<std::shared_ptr<BaseTexture> >({tex}));
                return app.GetRenderer()->AddMesh(ground);
            };

            bool loadedScene = false;
            if (argc > 1 && args && args[1] != nullptr)
                loadedScene = app.LoadSceneFromFile(std::string_view(args[1]));

            // Force a stable debug camera for scene-import testing.
            if (auto scene = app.GetScene())
                {
                    if (auto controller = scene->GetController())
                        {
                            controller->cam_ = std::make_shared<Camera>(
                                glm::vec3(6.0f, 4.0f, 10.0f));
                            controller->UpdateViewMatrix();
                        }
                }

            std::shared_ptr<Vulkan::Mesh> m1;
            std::shared_ptr<Vulkan::Mesh> m2;
            std::shared_ptr<Vulkan::Mesh> ground;
            if (!loadedScene)
                {
                    app.AddLight(std::make_shared<DirectionalLight>(
                        glm::vec3(0.15f, 0.15f, 0.15f),
                        glm::vec3(0.9f, 0.9f, 0.9f),
                        glm::vec3(0.2f, 0.2f, 0.2f),
                        glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::normalize(glm::vec3(-0.6f, -1.0f, -0.4f))));
                    std::tie(m1, m2) = spawnDebugCubes();
                    ground = spawnDebugGroundPlane();
                    if (m2 && m2->tr_)
                        m2->tr_->updateModel(app.GetRenderer()->GetCurFrame(),
                                             glm::translate(glm::mat4(1.0f),
                                                            glm::vec3(1.2f, 0.0f, 0.0f)));
                }
            else
                {
                    // Add visible debug helpers for imported scenes to simplify testing.
                    app.AddLight(std::make_shared<DirectionalLight>(
                        glm::vec3(0.2f, 0.2f, 0.2f),
                        glm::vec3(1.0f, 1.0f, 1.0f),
                        glm::vec3(0.5f, 0.5f, 0.5f),
                        glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::normalize(glm::vec3(-0.4f, -1.0f, -0.2f))));
                    std::tie(m1, m2) = spawnDebugCubes();
                    ground = spawnDebugGroundPlane();
                    if (m2 && m2->tr_)
                        m2->tr_->updateModel(app.GetRenderer()->GetCurFrame(),
                                             glm::translate(glm::mat4(1.0f),
                                                            glm::vec3(1.2f, 0.0f, 0.0f)));
                }

            static auto startTime = std::chrono::high_resolution_clock::now();
            while (app.MainLoop())
                {
                    if (!m1 || !m2)
                        continue;

                    auto currentTime =
                        std::chrono::high_resolution_clock::now();
                    float time =
                        std::chrono::duration<float,
                                              std::chrono::seconds::period>(
                            currentTime - startTime)
                            .count();

                    m1->tr_->updateModel(
                        app.GetRenderer()->GetCurFrame(),
                        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f)));
                    m2->tr_->updateModel(
                        app.GetRenderer()->GetCurFrame(),
                        glm::translate(glm::mat4(1.0f),
                                       glm::vec3(1.2f, 0.0f, 0.0f)) *
                            glm::rotate(glm::mat4(1.0f),
                                        time * glm::radians(90.0f),
                                        glm::vec3(1.0f, 0.0f, 0.0f)));
                }
        }
    catch (std::exception e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

    return EXIT_SUCCESS;
}
