/// \file main.cpp
//

#include "RenderEngine.h"
#include "utils/image_loader.h"
#include <SDL3/SDL_main.h>
#include <memory>

using namespace Multor;

BaseMesh *TestMesh, *TestMesh2;
/*
class Application 
{
public:

	Application() 
	{
		wnd = std::make_shared<Window>();
		eng = std::make_shared<VulkanRenderer>(wnd);
		eng->AddMesh(TestMesh);
	}

	void exec() 
	{
		mainLoop();
		//cleanup();
	}
	~Application() 
	{

	}
private:
	std::shared_ptr<Window> wnd;
	std::shared_ptr<VulkanRenderer> eng;

	void mainLoop() 
	{
		while (wnd->Run())
		{
			if (wnd->resized)
			{
				eng->Update();
				wnd->resized = false;
			}
			eng->Draw();
		}
	}

	void cleanup() {};
};*/

int main(int argc, char* args[])
{
    //getchar();
    try
        {
            const float cubePos[24] = {
                -0.5f, 0.25f, 0.25f,  0.0f,   0.25f, 0.25f, 0.0f,   -0.25f,
                0.25f, -0.5f, -0.25f, 0.25f,  -0.5f, 0.25f, 0.75f,  0.0f,
                0.25f, 0.75f, 0.0f,   -0.25f, 0.75f, -0.5f, -0.25f, 0.75f};

            const float cubePos2[24] = {
                -0.0f,  0.75f, -0.25f, 0.5f,   0.75f, -0.25f, 0.5f,  0.25f,
                -0.25f, -0.0f, 0.25f,  -0.25f, -0.0f, 0.75f,  0.25f, 0.5f,
                0.75f,  0.25f, 0.5f,   0.25f,  0.25f, -0.0f,  0.25f, 0.25f};

            std::vector<std::uint32_t> indices = std::vector<std::uint32_t>(
                {0, 1, 2, 0, 2, 3, 2, 1, 5, 2, 5, 6, 3, 2, 6, 3, 6, 7,
                 0, 3, 7, 0, 7, 4, 1, 0, 4, 1, 4, 5, 6, 5, 4, 6, 4, 7});
            const float                Triangle[9] = {0.0, -0.5, 0.0, 0.5, 0.5,
                                                      0.0, -0.5, 0.5, 0.0};
            std::vector<std::uint32_t> TrinIndices =
                std::vector<std::uint32_t>({1, 2, 0});
            std::shared_ptr<Multor::BaseTexture> tex =
                std::make_shared<BaseTexture>(
                    std::string("Diff"), std::string("core"),
                    Texture_Types::Diffuse,
                    std::vector<std::shared_ptr<Image> >(
                        {ImageLoader::LoadTexture(
                            "A:/VulkanEngine/build2/matrix.jpg")}));
            TestMesh = new BaseMesh(
                std::make_unique<Vertexes>(8, &cubePos[0],
                                           std::vector<std::uint32_t>(indices)),
                nullptr,
                std::vector<std::shared_ptr<BaseTexture> >(
                    {tex})); //new BaseMesh(std::make_unique<Vertexes>(3, ptr, std::move(TrinIndices)), nullptr, std::vector<std::shared_ptr<BaseTexture>>({ tex }));//
            TestMesh2 = new BaseMesh(
                std::make_unique<Vertexes>(8, &cubePos2[0], std::move(indices)),
                nullptr, std::vector<std::shared_ptr<BaseTexture> >({tex}));
            Application             app;
            std::shared_ptr<VkMesh> m1 = app.GetRenderer()->AddMesh(TestMesh);
            std::shared_ptr<VkMesh> m2 = app.GetRenderer()->AddMesh(TestMesh2);
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
                        app.GetRenderer()->getCurFrame(),
                        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f)));
                    m2->tr_->updateModel(
                        app.GetRenderer()->getCurFrame(),
                        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
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
