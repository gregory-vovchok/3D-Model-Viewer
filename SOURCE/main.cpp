
#include <Window/Window.h>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>     
#include <Shader/Shader.h>
#include <Texture/Texture2d.h>
#include <memory>
#include <Common/Common.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Camera/Camera.h>
#include <Model/Model.h>
#include <Lamp/Lamp.h>
#include <Skybox/Skybox.h>
#include <ModelView/ModelView.h>

#define AMOUNT_OF_SHADERS 4
#define AMOUNT_OF_POINT_LIGHTS 2


class Cat : public Model, public ModelView
{ };


class FBXViewerApplication : public Window  
{
	public: class DirectionalLight
	{
		public: glm::vec3 color;   
		public: glm::vec3 ambient;   
		public: glm::vec3 diffuse;   
		public: glm::vec3 specular;   
		public: glm::vec3 direction;    
	};


	private: std::shared_ptr<Shader> catShader;       

	private: glm::mat4 projectionView;     
	private: std::shared_ptr<Camera> camera; 

	private: std::shared_ptr<Skybox> skybox; 

	private: std::shared_ptr<Shader> shaders[AMOUNT_OF_SHADERS];  
	private: std::shared_ptr<Cat> cat;     
	private: std::shared_ptr<Lamp> lamp[AMOUNT_OF_POINT_LIGHTS];    



	public: virtual bool Init(const Window::Define& _define) override
	{
		if (Window::Init(_define))
		{
			LampInit();
			CameraInit();
			CatInit();
			return true;
		}
		return false;
	}

	public: virtual void Draw() override
	{
		CalculateCameraSpace();

		ClearScreen();  

		DrawScene();     
	}


	public: virtual void Update(float _delta) override
	{
		for (int i = 0; i < AMOUNT_OF_POINT_LIGHTS; i++)
		{
			lamp[i]->Update(_delta);
		}
	}


	protected: virtual void KeyboardEventHandler(int _key, int _action, int _mode) override
	{
		if (_key == GLFW_KEY_ESCAPE && _action == GLFW_PRESS)   
		{
			glfwSetWindowShouldClose(GetHandler(), GL_TRUE);     
		}
		else if (_key == GLFW_KEY_1 && _action == GLFW_PRESS)
		{
			catShader = shaders[0];
		}
		else if (_key == GLFW_KEY_2 && _action == GLFW_PRESS)
		{
			catShader = shaders[1];
		}
		else if (_key == GLFW_KEY_3 && _action == GLFW_PRESS)
		{
			catShader = shaders[2];
		}
		else if (_key == GLFW_KEY_4 && _action == GLFW_PRESS)
		{
			catShader = shaders[3];
		}
		else if (_key == GLFW_KEY_Q && _action == GLFW_PRESS)
		{
			cat->SetTextureFilteringMode(Texture2d::FilteringMode::POINT_FILTERING);
		}
		else if (_key == GLFW_KEY_W && _action == GLFW_PRESS)
		{
			cat->SetTextureFilteringMode(Texture2d::FilteringMode::BILINEAR_FILTERING);
		}
		else if (_key == GLFW_KEY_E && _action == GLFW_PRESS)
		{
			cat->SetTextureFilteringMode(Texture2d::FilteringMode::TRILINEAR_FILTERING);
		}
		else if (_key == GLFW_KEY_R && _action == GLFW_PRESS)
		{
			cat->SetTextureFilteringMode(Texture2d::FilteringMode::ANISOTROPY_FILTERING);
		}
		else if (_key == GLFW_KEY_A && _action == GLFW_PRESS)
		{
			SetCullFaceMode(Window::CULL_BACK_FACES);
		}
		else if (_key == GLFW_KEY_S && _action == GLFW_PRESS)
		{
			SetCullFaceMode(Window::CULL_DISABLE);
		}
		else if (_key == GLFW_KEY_Z && _action == GLFW_PRESS)
		{
			SetMultisamplingATCEnable(false);
		}
		else if (_key == GLFW_KEY_X && _action == GLFW_PRESS)
		{
			SetMultisamplingATCEnable(true);
		}

		Window::KeyboardEventHandler(_key, _action, _mode);
	}

	public: virtual void MouseMoveEventHandler(glm::vec2 _position) override
	{
		float xPosOffset = _position.x - GetLastCursorPosition().x;
		float yPosOffset = GetLastCursorPosition().y - _position.y;        

		camera->MouseMoveEventHandler(xPosOffset, yPosOffset);

		Window::MouseMoveEventHandler(_position);
	}


	public: virtual void MouseWheelEventHandler(int _offset) override
	{
		camera->Zoom(_offset);
	}


	protected: virtual void Destroy() override
	{
		Window::Destroy();
	}


	private: void CalculateCameraSpace()
	{
		projectionView = camera->GetProjectionMatrix() * camera->GetViewMatrix();
	}


	private: void DrawScene()
	{
		DrawCat();

		DrawLamp();

	}

	private: void LampInit()
	{
		for (int i = 0; i < AMOUNT_OF_POINT_LIGHTS; i++)
		{
			lamp[i] = std::make_shared<Lamp>();

			lamp[i]->state.color = glm::vec3(1.0f, 1.0f, 1.0f);   
			lamp[i]->state.position = glm::vec3(1.5f, 3.0f - i * 1.2f, 1.5f);   

			lamp[i]->state.ambient = glm::vec3(0.0f, 0.0f, 0.0f);   
			lamp[i]->state.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);   
			lamp[i]->state.specular = glm::vec3(1.0f, 1.0f, 1.0f);   

			lamp[i]->state.radius = 7.3f;       
			lamp[i]->state.t = i * (lamp[i]->state.radius / AMOUNT_OF_POINT_LIGHTS);   
			lamp[i]->state.moveSpeed = 1.0f;    

			lamp[i]->state.constant = 1.0f;   
			lamp[i]->state.linear = 0.0014f;   
			lamp[i]->state.quadratic = 0.000007f;   
		}
	}


	private: void DrawLamp()
	{
		for (int i = 0; i < AMOUNT_OF_POINT_LIGHTS; i++)
		{
			lamp[i]->Draw(projectionView);
		}
	}


	private: void CatShaderInit()
	{
		shaders[0] = std::make_shared<Shader>();

		if (shaders[0])    
		{
			if (!shaders[0]->CreateShaderProgramFromFile("shaders/blinn-phong_light4.vs", "shaders/blinn-phong_light4.fs"))
			{
				throw std::runtime_error("Failed to create shader program");
			}
		}

		for (int i = 0; i < AMOUNT_OF_SHADERS; i++)
		{
			if (shaders[i])   
			{
				shaders[i]->Bind();     
				shaders[i]->SetValue("model", cat->GetTransformation());

				for (int j = 0; j < AMOUNT_OF_POINT_LIGHTS; j++)
				{
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].position", lamp[j]->state.position);
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].color", lamp[j]->state.color);

					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].ambient", lamp[j]->state.ambient);
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].diffuse", lamp[j]->state.diffuse);
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].specular", lamp[j]->state.specular);

					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].constant", lamp[j]->state.constant);
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].linear", lamp[j]->state.linear);
					shaders[i]->SetValue("pointLights[" + std::to_string(j) + "].quadratic", lamp[j]->state.quadratic);
				}

				shaders[i]->SetValue("material.shininess", 32.0f);
			}
		}

		catShader = shaders[0];     
	}

	private: void CatInit()
	{
		cat = std::make_shared<Cat>();

		cat->SetTransformation(glm::vec3(0.2f), glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f)), glm::vec3(0.0f));

		CatShaderInit();

		cat->Create("cat/cat.fbx");
		cat->AddTextureMap("cat/body_norm.png", Texture2d::Role::NORMAL, 0);
		cat->AddTextureMap("cat/fur_norm.png", Texture2d::Role::NORMAL, 1);

		cat->AddTextureMap("cat/body_spec.png", Texture2d::Role::SPECULAR, 0);
		cat->AddTextureMap("cat/fur_diff_spec.png", Texture2d::Role::SPECULAR, 1);
		cat->SetTextureFilteringMode(Texture2d::FilteringMode::ANISOTROPY_FILTERING);
	}

	private: void SkyboxInit()
	{
		skybox = std::make_shared<Skybox>();
		skybox->SetScale(glm::vec3(40));

		std::vector<std::string> order
		{
			"skyboxes/sunset/right.png",
			"skyboxes/sunset/left.png",
			"skyboxes/sunset/up.png",
			"skyboxes/sunset/down.png",
			"skyboxes/sunset/back.png",
			"skyboxes/sunset/front.png",
		};

		skybox->Create(order);
	}

	private: void CameraInit()
	{
		camera = std::unique_ptr<Camera>(new Camera(GetClientAreaSize().x / GetClientAreaSize().y));
		camera->SetRotationMode(Camera::RotationMode::OBJECT_VIEWER);

		camera->SetZoomEnable(true);
		camera->SetZoomFar(18.0f);
		camera->SetZoomNear(8.0f);
		camera->SetZoomSpeed(0.8f);

		LockCursor();        
	}


	private: void DrawSkybox()
	{
		if (skybox)
		{
			glm::mat4 skyboxModelView = camera->GetProjectionMatrix() * glm::mat4(glm::mat3(camera->GetViewMatrix()));             

			skybox->Draw(skyboxModelView);
		}
	}

	private: void DrawCat()
	{
		if (catShader)   
		{
			catShader->Bind();    

			catShader->SetValue("projectionView", projectionView);
			catShader->SetValue("viewPos", camera->GetPosition());
			catShader->SetValue("normalMatrix", cat->GetNormalMatrix());

			for (int i = 0; i < AMOUNT_OF_POINT_LIGHTS; i++)
			{
				catShader->SetValue("pointLights[" + std::to_string(i) + "].position", lamp[i]->state.position);
			}

			if (cat) { cat->Draw(catShader); }    
		}
	}
};


int main()
{
	FBXViewerApplication app;

	Window::Define define;
	define.title = "FBX Viewer";
	define.clientAreaSize = glm::vec2(1920, 1080);
	define.multisamplingMode = Window::MultisamplingMode::MULTI_SAMPLING_8X;
	define.multisamplingATCEnable = true;
	define.cullFaceMode = Window::CullFaceMode::CULL_DISABLE;


	if (app.Init(define))
	{
		app.Loop();  
	}

	return EXIT_SUCCESS;
}