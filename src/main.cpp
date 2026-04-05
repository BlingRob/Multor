/// \file main.cpp
//

#include "application.h"
#include "utils/image_loader.h"
#include <SDL3/SDL_main.h>
#include <chrono>
#include <cmath>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

using namespace Multor;

BaseMesh *TestMesh, *TestMesh2;

int main(int argc, char* args[])
{
    //getchar();
    try
        {
            Application app;
            std::shared_ptr<PointLight> demoPointLight;
            std::shared_ptr<Node> demoPointLightMarker;

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

            auto buildTexturedCubeMesh =
                [](const std::shared_ptr<Multor::BaseTexture>& tex) -> std::shared_ptr<BaseMesh>
            {
                const float cubePos[72] = {
                    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                    -0.5f, 0.5f,  0.5f,
                    0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
                    0.5f,  0.5f,  -0.5f,
                    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                    -0.5f, 0.5f,  -0.5f,
                    0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
                    0.5f,  0.5f,  0.5f,
                    -0.5f, 0.5f,  0.5f,  0.5f,  0.5f, 0.5f,  0.5f, 0.5f, -0.5f,
                    -0.5f, 0.5f,  -0.5f,
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

                return std::make_shared<BaseMesh>(
                    std::make_unique<Vertexes>(24, &cubePos[0], std::move(indices),
                                               nullptr, &cubeTexCoords[0]),
                    nullptr,
                    std::vector<std::shared_ptr<BaseTexture> >({tex}));
            };

            auto buildGroundMesh =
                [](const std::shared_ptr<Multor::BaseTexture>& tex) -> std::shared_ptr<BaseMesh>
            {
                const float planeSize = 20.0f;
                const float y = -1.0f;
                const float planePos[12] = {
                    -planeSize, y, -planeSize, planeSize, y, -planeSize,
                    planeSize, y, planeSize, -planeSize, y, planeSize};
                const float planeNormals[12] = {
                    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
                const float planeTexCoords[8] = {
                    0.0f, 0.0f, 8.0f, 0.0f, 8.0f, 8.0f, 0.0f, 8.0f};
                const float planeTangents[12] = {
                    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
                const float planeBitangents[12] = {
                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
                std::vector<std::uint32_t> planeIndices = {0, 1, 2, 0, 2, 3};

                return std::make_shared<BaseMesh>(
                    std::make_unique<Vertexes>(4, &planePos[0], std::move(planeIndices),
                                               &planeNormals[0], &planeTexCoords[0],
                                               &planeTangents[0], &planeBitangents[0]),
                    nullptr,
                    std::vector<std::shared_ptr<BaseTexture> >({tex}));
            };

            auto buildSolidColorTexture =
                [](const std::string& name, const glm::u8vec4& color,
                   Texture_Types type = Texture_Types::Diffuse) -> std::shared_ptr<BaseTexture>
            {
                auto* pixel = new unsigned char[4] {
                    color.r, color.g, color.b, color.a};
                auto image = std::make_shared<Image>(1, 1, 4, pixel);
                return std::make_shared<BaseTexture>(
                    name, std::string("generated://") + name, type,
                    std::vector<std::shared_ptr<Image> >({image}));
            };

            auto buildSphereMesh =
                [](const std::shared_ptr<Multor::BaseTexture>& tex,
                   float radius = 1.0f,
                   std::uint32_t stacks = 48,
                   std::uint32_t slices = 64) -> std::shared_ptr<BaseMesh>
            {
                std::vector<float> positions;
                std::vector<float> normals;
                std::vector<float> texCoords;
                std::vector<float> tangents;
                std::vector<float> bitangents;
                std::vector<std::uint32_t> indices;

                positions.reserve((stacks + 1) * (slices + 1) * 3);
                normals.reserve((stacks + 1) * (slices + 1) * 3);
                texCoords.reserve((stacks + 1) * (slices + 1) * 2);
                tangents.reserve((stacks + 1) * (slices + 1) * 3);
                bitangents.reserve((stacks + 1) * (slices + 1) * 3);

                constexpr float pi = 3.14159265358979323846f;
                for (std::uint32_t stack = 0; stack <= stacks; ++stack)
                    {
                        const float v = static_cast<float>(stack) / static_cast<float>(stacks);
                        const float phi = v * pi;
                        const float y = std::cos(phi);
                        const float ringRadius = std::sin(phi);

                        for (std::uint32_t slice = 0; slice <= slices; ++slice)
                            {
                                const float u = static_cast<float>(slice) / static_cast<float>(slices);
                                const float theta = u * 2.0f * pi;
                                const float x = ringRadius * std::cos(theta);
                                const float z = ringRadius * std::sin(theta);

                                positions.push_back(x * radius);
                                positions.push_back(y * radius);
                                positions.push_back(z * radius);

                                normals.push_back(x);
                                normals.push_back(y);
                                normals.push_back(z);

                                glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
                                if (glm::length(tangent) < 1e-4f)
                                    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                                tangent = glm::normalize(tangent);

                                glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
                                glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
                                if (glm::length(bitangent) < 1e-4f)
                                    bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

                                tangents.push_back(tangent.x);
                                tangents.push_back(tangent.y);
                                tangents.push_back(tangent.z);

                                bitangents.push_back(bitangent.x);
                                bitangents.push_back(bitangent.y);
                                bitangents.push_back(bitangent.z);

                                texCoords.push_back(u);
                                texCoords.push_back(1.0f - v);
                            }
                    }

                for (std::uint32_t stack = 0; stack < stacks; ++stack)
                    {
                        for (std::uint32_t slice = 0; slice < slices; ++slice)
                            {
                                const std::uint32_t first = stack * (slices + 1) + slice;
                                const std::uint32_t second = first + slices + 1;

                                indices.push_back(first);
                                indices.push_back(second);
                                indices.push_back(first + 1);

                                indices.push_back(second);
                                indices.push_back(second + 1);
                                indices.push_back(first + 1);
                            }
                    }

                return std::make_shared<BaseMesh>(
                    std::make_unique<Vertexes>(positions.size() / 3,
                                               positions.data(),
                                               std::move(indices),
                                               normals.data(),
                                               texCoords.data(),
                                               tangents.data(),
                                               bitangents.data()),
                    nullptr,
                    std::vector<std::shared_ptr<BaseTexture> >({tex}));
            };

            auto buildSamplePhysicsScene =
                [&app, &buildTexturedCubeMesh, &buildGroundMesh]() -> std::shared_ptr<Scene>
            {
                auto baseScene = app.GetScene();
                auto scene = std::make_shared<Scene>(
                    baseScene ? baseScene->GetController() : nullptr);

                auto cubeTex = std::make_shared<BaseTexture>(
                    std::string("Diff"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/matrix.jpg")}));
                auto groundTex = std::make_shared<BaseTexture>(
                    std::string("Ground"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall.jpg")}));

                auto groundNode = std::make_shared<Node>();
                groundNode->SetName("physics_ground");
                groundNode->addMesh(buildGroundMesh(groundTex));
                scene->AddNode(groundNode);
                scene->AddRigidBody(
                    groundNode,
                    RigidBodyDesc {.type_ = RigidBodyType::Static},
                    ColliderDesc {.shape_ = ColliderShape::Box,
                                  .halfExtents_ = glm::vec3(20.0f, 1.0f, 20.0f)});

                auto cubeA = std::make_shared<Node>();
                cubeA->SetName("physics_cube_a");
                cubeA->addMesh(buildTexturedCubeMesh(cubeTex));
                cubeA->SetLocalTransform(
                    glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f)));
                scene->AddNode(cubeA);
                scene->AddRigidBody(
                    cubeA,
                    RigidBodyDesc {.type_ = RigidBodyType::Dynamic,
                                   .mass_ = 1.0f,
                                   .linearDamping_ = 0.05f},
                    ColliderDesc {.shape_ = ColliderShape::Box,
                                  .halfExtents_ = glm::vec3(0.5f)});

                auto cubeB = std::make_shared<Node>();
                cubeB->SetName("physics_cube_b");
                cubeB->addMesh(buildTexturedCubeMesh(cubeTex));
                cubeB->SetLocalTransform(
                    glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 6.0f, 0.0f)));
                scene->AddNode(cubeB);
                scene->AddRigidBody(
                    cubeB,
                    RigidBodyDesc {.type_ = RigidBodyType::Dynamic,
                                   .mass_ = 1.0f,
                                   .linearDamping_ = 0.03f},
                    ColliderDesc {.shape_ = ColliderShape::Box,
                                  .halfExtents_ = glm::vec3(0.5f)});

                scene->AddLight(std::make_shared<DirectionalLight>(
                    glm::vec3(0.18f, 0.18f, 0.18f),
                    glm::vec3(0.95f, 0.95f, 0.95f),
                    glm::vec3(0.35f, 0.35f, 0.35f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::normalize(glm::vec3(-0.6f, -1.0f, -0.4f))));
                scene->SetBackGround(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));
                scene->SetPhysicsEnabled(true);
                return scene;
            };

            auto buildPointShadowDemoScene =
                [&app, &buildGroundMesh, &buildSphereMesh, &buildSolidColorTexture,
                 &demoPointLight, &demoPointLightMarker]() -> std::shared_ptr<Scene>
            {
                auto baseScene = app.GetScene();
                auto scene = std::make_shared<Scene>(
                    baseScene ? baseScene->GetController() : nullptr);

                auto groundAlbedoTex = std::make_shared<BaseTexture>(
                    std::string("GroundAlbedo"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall/albedo.png")}));
                auto groundNormalTex = std::make_shared<BaseTexture>(
                    std::string("GroundNormal"), std::string("core"),
                    Texture_Types::Normal,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall/normal-ogl.png")}));
                auto groundMetallicTex = std::make_shared<BaseTexture>(
                    std::string("GroundMetallic"), std::string("core"),
                    Texture_Types::Metallic,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall/metallic.png")}));
                auto groundRoughnessTex = std::make_shared<BaseTexture>(
                    std::string("GroundRoughness"), std::string("core"),
                    Texture_Types::Roughness,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall/roughness.png")}));
                auto groundAoTex = std::make_shared<BaseTexture>(
                    std::string("GroundAO"), std::string("core"),
                    Texture_Types::Ambient_occlusion,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/wall/ao.png")}));

                auto sphereAlbedoTex = std::make_shared<BaseTexture>(
                    std::string("SphereBaseColor"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/ball/albedo.png")}));
                auto sphereNormalTex = std::make_shared<BaseTexture>(
                    std::string("SphereNormal"), std::string("core"),
                    Texture_Types::Normal,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/ball/normal-ogl.png")}));
                auto sphereMetallicTex = std::make_shared<BaseTexture>(
                    std::string("SphereMetallic"), std::string("core"),
                    Texture_Types::Metallic,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/ball/metallic.png")}));
                auto sphereRoughnessTex = std::make_shared<BaseTexture>(
                    std::string("SphereRoughness"), std::string("core"),
                    Texture_Types::Roughness,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/ball/roughness.png")}));
                auto sphereAoTex = std::make_shared<BaseTexture>(
                    std::string("SphereAO"), std::string("core"),
                    Texture_Types::Ambient_occlusion,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture("./Res/ball/ao.png")}));
                auto lightMarkerTex = buildSolidColorTexture(
                    "PointLightMarker", glm::u8vec4(255, 232, 180, 255));

                auto groundNode = std::make_shared<Node>();
                groundNode->SetName("shadow_demo_ground");
                auto groundMesh = buildGroundMesh(groundAlbedoTex);
                groundMesh->AddTexture(groundNormalTex);
                groundMesh->AddTexture(groundMetallicTex);
                groundMesh->AddTexture(groundRoughnessTex);
                groundMesh->AddTexture(groundAoTex);
                if (auto* mat = groundMesh->GetMaterial())
                    mat->UseMetallicRoughnessPBR(glm::vec4(1.0f), 0.05f, 0.85f);
                groundNode->addMesh(groundMesh);
                scene->AddNode(groundNode);

                auto sphereNode = std::make_shared<Node>();
                sphereNode->SetName("shadow_demo_sphere");
                auto sphereMesh = buildSphereMesh(sphereAlbedoTex, 1.0f, 72, 96);
                sphereMesh->AddTexture(sphereNormalTex);
                sphereMesh->AddTexture(sphereMetallicTex);
                sphereMesh->AddTexture(sphereRoughnessTex);
                sphereMesh->AddTexture(sphereAoTex);
                if (auto* mat = sphereMesh->GetMaterial())
                    {
                        mat->UseMetallicRoughnessPBR(glm::vec4(1.0f), 0.15f, 0.32f);
                        mat->normalScale = 0.25f;
                    }
                sphereNode->addMesh(sphereMesh);
                sphereNode->SetLocalTransform(
                    glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.25f, 0.0f)));
                scene->AddNode(sphereNode);

                auto metalSphereNode = std::make_shared<Node>();
                metalSphereNode->SetName("shadow_demo_metal_sphere");
                auto metalSphereMesh = buildSphereMesh(sphereAlbedoTex, 0.8f, 72, 96);
                metalSphereMesh->AddTexture(sphereNormalTex);
                metalSphereMesh->AddTexture(sphereMetallicTex);
                metalSphereMesh->AddTexture(sphereRoughnessTex);
                metalSphereMesh->AddTexture(sphereAoTex);
                if (auto* mat = metalSphereMesh->GetMaterial())
                    mat->UseMetallicRoughnessPBR(
                        glm::vec4(1.0f, 0.92f, 0.8f, 1.0f), 0.85f, 0.18f);
                if (auto* mat = metalSphereMesh->GetMaterial())
                    mat->normalScale = 0.12f;
                metalSphereNode->addMesh(metalSphereMesh);
                metalSphereNode->SetLocalTransform(
                    glm::translate(glm::mat4(1.0f), glm::vec3(-2.1f, -0.2f, 1.2f)));
                scene->AddNode(metalSphereNode);

                auto pointLight = std::make_shared<PointLight>(
                    glm::vec3(0.06f, 0.06f, 0.06f),
                    glm::vec3(2.4f, 2.2f, 2.0f),
                    glm::vec3(1.2f, 1.2f, 1.2f),
                    glm::vec3(1.0f, 0.09f, 0.032f),
                    glm::vec3(2.0f, 4.0f, 2.0f));
                pointLight->SetName("shadow_demo_point_light");
                scene->AddLight(pointLight);
                demoPointLight = pointLight;

                auto lightMarkerNode = std::make_shared<Node>();
                lightMarkerNode->SetName("shadow_demo_point_light_marker");
                auto lightMarkerMesh = buildSphereMesh(lightMarkerTex, 0.075f, 12, 16);
                if (auto* mat = lightMarkerMesh->GetMaterial())
                    {
                        mat->UseMetallicRoughnessPBR(glm::vec4(1.0f), 0.0f, 0.9f);
                        mat->emissiveFactor = glm::vec3(8.0f, 6.5f, 4.0f);
                    }
                lightMarkerNode->addMesh(lightMarkerMesh);
                lightMarkerNode->SetLocalTransform(
                    glm::translate(glm::mat4(1.0f), pointLight->GetPos()));
                scene->AddNode(lightMarkerNode);
                demoPointLightMarker = lightMarkerNode;

                scene->SetBackGround(glm::vec4(0.04f, 0.05f, 0.07f, 1.0f));
                scene->SetPhysicsEnabled(false);
                return scene;
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
                    app.SetScene(buildPointShadowDemoScene());
                    app.GetRenderer()->LoadEnvironmentTexture("./Res/matrix.jpg");
                    app.GetRenderer()->LoadIrradianceTexture("./Res/matrix.jpg");
                    app.GetRenderer()->LoadPrefilteredEnvironmentTexture("./Res/matrix.jpg");
                    if (auto scene = app.GetScene())
                        {
                            if (auto controller = scene->GetController())
                                {
                                    controller->cam_ = std::make_shared<Camera>(
                                        glm::vec3(4.5f, 3.0f, 6.5f),
                                        glm::vec3(0.0f, 0.0f, -1.0f),
                                        glm::vec3(0.0f, 1.0f, 0.0f),
                                        -125.0f, -18.0f);
                                    controller->UpdateViewMatrix();
                                }
                        }
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
                    if (demoPointLightMarker)
                        {
                            auto scene = app.GetScene();
                            auto activeLight =
                                scene ? std::dynamic_pointer_cast<PointLight>(
                                            scene->GetLight("shadow_demo_point_light"))
                                      : nullptr;
                            if (activeLight)
                                {
                                    demoPointLight = activeLight;
                                    demoPointLightMarker->SetLocalTransform(
                                        glm::translate(glm::mat4(1.0f),
                                                       activeLight->GetPos()));
                                }
                            else
                                {
                                    demoPointLight.reset();
                                    demoPointLightMarker->SetLocalTransform(
                                        glm::translate(glm::mat4(1.0f),
                                                       glm::vec3(0.0f, -1000.0f, 0.0f)));
                                }
                        }

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
