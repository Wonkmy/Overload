/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <OvEditor/Core/ProjectHub.h>

#include <OvTools/Utils/PathParser.h>
#include <OvTools/Utils/SystemCalls.h>

#include <OvUI/Widgets/Buttons/Button.h>
#include <OvUI/Widgets/Layout/Columns.h>
#include <OvUI/Widgets/Layout/Group.h>
#include <OvUI/Widgets/Layout/Spacing.h>
#include <OvUI/Widgets/InputFields/InputText.h>
#include <OvUI/Widgets/Selection/ComboBox.h>
#include <OvUI/Widgets/Texts/Text.h>
#include <OvUI/Widgets/Visual/Separator.h>

#include <OvWindowing/Dialogs/MessageBox.h>
#include <OvWindowing/Dialogs/OpenFileDialog.h>
#include <OvWindowing/Dialogs/SaveFileDialog.h>

#define PROJECTS_FILE std::string(OvTools::Utils::SystemCalls::GetPathToAppdata() + "\\OverloadTech\\OvEditor\\projects.ini")

struct TemplateProject
{
	const std::string_view name;
	const std::string_view url;
};

constexpr auto kTemplateProjects = std::to_array<TemplateProject>({
	{"Showroom", "https://github.com/Overload-Technologies/Overload-Showroom"},
	{"Cargo", "https://github.com/Overload-Technologies/Cargo-Demo"}
});

class ProjectHubPanel : public OvUI::Panels::PanelWindow
{
public:
	ProjectHubPanel(bool& p_readyToGo, std::string& p_path, std::string& p_projectName) :
		PanelWindow("Overload - Project Hub", true),
		m_readyToGo(p_readyToGo),
		m_path(p_path),
		m_projectName(p_projectName)
	{
		resizable = false;
		movable = false;
		titleBar = false;

		std::filesystem::create_directories(OvTools::Utils::SystemCalls::GetPathToAppdata() + "\\OverloadTech\\OvEditor\\");

		SetSize({ 1300, 580 });
		SetPosition({ 0.f, 0.f });

		auto& templateOptions = CreateWidget<OvUI::Widgets::Selection::ComboBox>(-2);
		templateOptions.choices = {
			{-3, "Create Empty Project"},
			{-2, "Load From Disk"},
			{-1, "Create Project From GitHub URL"}
		};
		for (uint8_t i = 0; i < kTemplateProjects.size(); ++i)
		{
			templateOptions.choices.emplace(i, std::format("Create From Template: {}", kTemplateProjects[i].name));
		}
		
		templateOptions.lineBreak = false;

		m_goButton = &CreateWidget<OvUI::Widgets::Buttons::Button>("GO");
		m_goButton->idleBackgroundColor = OvUI::Types::Color{ 0.f, 0.5f, 0.0f };
		auto& templateField = CreateWidget<OvUI::Widgets::InputFields::InputText>("", "Template URL");
		templateField.enabled = false;

		// UpdateGoButton("");

		templateOptions.ValueChangedEvent += [this, &templateField](int p_index)
		{
			templateField.content = "";
			templateField.enabled = p_index == -1;

			if (p_index >= 0 && p_index < kTemplateProjects.size())
			{
				templateField.content = kTemplateProjects[p_index].url;
			}
		};

		auto selectNewProjectLocation = []
		{
			OvWindowing::Dialogs::SaveFileDialog dialog("New project location");
			dialog.DefineExtension("Overload Project", "..");
			dialog.Show();
			if (dialog.HasSucceeded())
			{
				std::string result = dialog.GetSelectedFilePath();
				std::string path{ result.data(), result.data() + result.size() - std::string("..").size() }; // remove auto extension
				path += "\\";
				return path;
			}
			return std::string();
		};

		auto selectOpenProjectLocation = []
		{
			OvWindowing::Dialogs::OpenFileDialog dialog("Open project");
			dialog.AddFileType("Overload Project", "*.ovproject");
			dialog.Show();
			if (dialog.HasSucceeded())
			{
				std::string ovProjectPath = dialog.GetSelectedFilePath();
				std::string rootFolderPath = OvTools::Utils::PathParser::GetContainingFolder(ovProjectPath);
				return rootFolderPath;
			}
			return std::string();
		};

		m_goButton->ClickedEvent += [&]
		{
			if (templateOptions.currentChoice == -3)
			{
				const auto path = selectNewProjectLocation();
				if (!path.empty())
				{
					CreateProject(path);
					RegisterProject(path);
					OpenProject(path);
				}
			}
			else if (templateOptions.currentChoice == -2)
			{
				const auto path = selectOpenProjectLocation();
				if (!path.empty())
				{
					RegisterProject(path);
					OpenProject(path);
				}
			}
			else if (templateOptions.currentChoice == -1)
			{
				const auto path = selectNewProjectLocation();
				if (!path.empty())
				{
					CreateProjectFromTemplate(path, templateField.content);
					RegisterProject(path);
					OpenProject(path);
				}
			}
			else
			{
				const auto path = selectNewProjectLocation();
				if (!path.empty())
				{
					CreateProjectFromTemplate(path, std::string{ kTemplateProjects[templateOptions.currentChoice].url });
					RegisterProject(path);
					OpenProject(path);
				}
			}
			
		};

		for (uint8_t i = 0; i < 2; ++i)
			CreateWidget<OvUI::Widgets::Layout::Spacing>();

		CreateWidget<OvUI::Widgets::Visual::Separator>();

		for (uint8_t i = 0; i < 2; ++i)
			CreateWidget<OvUI::Widgets::Layout::Spacing>();

		auto& columns = CreateWidget<OvUI::Widgets::Layout::Columns<2>>();

		columns.widths = { 750, 500 };

		std::string line;
		std::ifstream myfile(PROJECTS_FILE);
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				if (std::filesystem::exists(line)) // TODO: Delete line from the file
				{
					auto& text = columns.CreateWidget<OvUI::Widgets::Texts::Text>(line);
					auto& actions = columns.CreateWidget<OvUI::Widgets::Layout::Group>();
					auto& openButton = actions.CreateWidget<OvUI::Widgets::Buttons::Button>("Open");
					auto& deleteButton = actions.CreateWidget<OvUI::Widgets::Buttons::Button>("Delete");

					openButton.idleBackgroundColor = { 0.7f, 0.5f, 0.f };
					deleteButton.idleBackgroundColor = { 0.5f, 0.f, 0.f };

					openButton.ClickedEvent += [this, line]
					{
						OpenProject(line);
					};

					std::string toErase = line;
					deleteButton.ClickedEvent += [this, &text, &actions, toErase]
					{
						text.Destroy();
						actions.Destroy();

						std::string line;
						std::ifstream fin(PROJECTS_FILE);
						std::ofstream temp("temp");

						while (getline(fin, line))
							if (line != toErase)
								temp << line << std::endl;

						temp.close();
						fin.close();

						std::filesystem::remove(PROJECTS_FILE);
						std::filesystem::rename("temp", PROJECTS_FILE);
					};

					openButton.lineBreak = false;
					deleteButton.lineBreak;
				}
			}
			myfile.close();
		}
	}

	void UpdateGoButton(const std::string& p_path)
	{
		bool validPath = p_path != "";
		m_goButton->idleBackgroundColor = validPath ? OvUI::Types::Color{ 0.f, 0.5f, 0.0f } : OvUI::Types::Color{ 0.1f, 0.1f, 0.1f };
		m_goButton->disabled = !validPath;
	}

	void CreateProject(const std::string& p_path)
	{
		if (!std::filesystem::exists(p_path))
		{
			std::filesystem::create_directory(p_path);
			std::filesystem::create_directory(p_path + "Assets\\");
			std::filesystem::create_directory(p_path + "Scripts\\");
			std::ofstream projectFile(p_path + '\\' + OvTools::Utils::PathParser::GetElementName(std::string(p_path.data(), p_path.data() + p_path.size() - 1)) + ".ovproject");
		}
	}

	void CreateProjectFromTemplate(const std::string& p_path, const std::string& p_templateUrl)
	{
		if (!std::filesystem::exists(p_path))
		{
			std::system(std::format("git clone {} {}", p_templateUrl, p_path).c_str());
		}
	}

	void RegisterProject(const std::string& p_path)
	{
		bool pathAlreadyRegistered = false;

		{
			std::string line;
			std::ifstream myfile(PROJECTS_FILE);
			if (myfile.is_open())
			{
				while (getline(myfile, line))
				{
					if (line == p_path)
					{
						pathAlreadyRegistered = true;
						break;
					}
				}
				myfile.close();
			}
		}

		if (!pathAlreadyRegistered)
		{
			std::ofstream projectsFile(PROJECTS_FILE, std::ios::app);
			projectsFile << p_path << std::endl;
		}
	}
	
	void OpenProject(const std::string& p_path)
	{
		m_readyToGo = std::filesystem::exists(p_path);
		if (!m_readyToGo)
		{
			using namespace OvWindowing::Dialogs;
			MessageBox errorMessage("Project not found", "The selected project does not exists", MessageBox::EMessageType::ERROR, MessageBox::EButtonLayout::OK);
		}
		else
		{
			m_path = p_path;
			m_projectName = OvTools::Utils::PathParser::GetElementName(m_path);
			Close();
		}
	}

	void Draw() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 50.f, 50.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);

		OvUI::Panels::PanelWindow::Draw();

		ImGui::PopStyleVar(2);
	}

private:
	bool& m_readyToGo;
	std::string& m_path;
	std::string& m_projectName;
	OvUI::Widgets::Buttons::Button* m_goButton = nullptr;
};

OvEditor::Core::ProjectHub::ProjectHub()
{
	SetupContext();
	m_mainPanel = std::make_unique<ProjectHubPanel>(m_readyToGo, m_projectPath, m_projectName);

	m_uiManager->SetCanvas(m_canvas);
	m_canvas.AddPanel(*m_mainPanel);
}

std::tuple<bool, std::string, std::string> OvEditor::Core::ProjectHub::Run()
{
	while (!m_window->ShouldClose())
	{
		m_device->PollEvents();
		m_uiManager->Render();
		m_window->SwapBuffers();

		if (!m_mainPanel->IsOpened())
			m_window->SetShouldClose(true);
	}

	return { m_readyToGo, m_projectPath, m_projectName };
}

void OvEditor::Core::ProjectHub::SetupContext()
{
	/* Settings */
	OvWindowing::Settings::DeviceSettings deviceSettings;
	OvWindowing::Settings::WindowSettings windowSettings;
	windowSettings.title = "Overload - Project Hub";
	windowSettings.width = 1000;
	windowSettings.height = 580;
	windowSettings.maximized = false;
	windowSettings.resizable = false;
	windowSettings.decorated = true;

	/* Window creation */
	m_device = std::make_unique<OvWindowing::Context::Device>(deviceSettings);
	m_window = std::make_unique<OvWindowing::Window>(*m_device, windowSettings);
	m_window->MakeCurrentContext();

	auto[monWidth, monHeight] = m_device->GetMonitorSize();
	auto[winWidth, winHeight] = m_window->GetSize();
	m_window->SetPosition(monWidth / 2 - winWidth / 2, monHeight / 2 - winHeight / 2);

	/* Graphics context creation */
	m_driver = std::make_unique<OvRendering::Context::Driver>(OvRendering::Settings::DriverSettings{ false });

	m_uiManager = std::make_unique<OvUI::Core::UIManager>(m_window->GetGlfwWindow(), OvUI::Styling::EStyle::ALTERNATIVE_DARK);
	m_uiManager->LoadFont("Ruda_Big", "Data\\Editor\\Fonts\\Ruda-Bold.ttf", 18);
	m_uiManager->UseFont("Ruda_Big");
	m_uiManager->EnableEditorLayoutSave(false);
	m_uiManager->EnableDocking(false);
}

void OvEditor::Core::ProjectHub::RegisterProject(const std::string& p_path)
{
	static_cast<ProjectHubPanel*>(m_mainPanel.get())->RegisterProject(p_path);
}
