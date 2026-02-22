/// \file main.cpp
//

#include "application.h"
#include "utils/image_loader.h"
#include <SDL3/SDL_main.h>
#include <memory>

using namespace Multor;

BaseMesh *TestMesh, *TestMesh2;

int main(int argc, char* args[])
{
    //getchar();
    try
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
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // front
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // back
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // left
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // right
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // top
                0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f   // bottom
            };

            std::vector<std::uint32_t> indices = {0,  1,  2,  0,  2,  3,  4,  5,  6,
                                                  4,  6,  7,  8,  9,  10, 8,  10, 11,
                                                  12, 13, 14, 12, 14, 15, 16, 17, 18,
                                                  16, 18, 19, 20, 21, 22, 20, 22, 23};
            std::shared_ptr<Multor::BaseTexture> tex =
                std::make_shared<BaseTexture>(
                    std::string("Diff"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture(
                            "./Res/matrix.jpg")}));
            TestMesh = new BaseMesh(
                std::make_unique<Vertexes>(24, &cubePos[0],
                                           std::vector<std::uint32_t>(indices),
                                           nullptr, &cubeTexCoords[0]),
                nullptr,
                std::vector<std::shared_ptr<BaseTexture> >(
                    {tex})); //new BaseMesh(std::make_unique<Vertexes>(3, ptr, std::move(TrinIndices)), nullptr, std::vector<std::shared_ptr<BaseTexture>>({ tex }));//
            TestMesh2 = new BaseMesh(
                std::make_unique<Vertexes>(24, &cubePos[0], std::move(indices),
                                           nullptr, &cubeTexCoords[0]),
                nullptr, std::vector<std::shared_ptr<BaseTexture> >({tex}));
            Application             app;
            app.AddLight(std::make_shared<DirectionalLight>(
                glm::vec3(0.15f, 0.15f, 0.15f), glm::vec3(0.9f, 0.9f, 0.9f),
                glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 0.0f, 0.0f),
                glm::normalize(glm::vec3(-0.6f, -1.0f, -0.4f))));
            std::shared_ptr<Vulkan::Mesh> m1 = app.GetRenderer()->AddMesh(TestMesh);
            std::shared_ptr<Vulkan::Mesh> m2 = app.GetRenderer()->AddMesh(TestMesh2);
            static auto startTime = std::chrono::high_resolution_clock::now();
            while (app.MainLoop())
                {
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
