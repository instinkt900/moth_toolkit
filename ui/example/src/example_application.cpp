#include "example_application.h"

#include "moth_ui/utils/vector_serialization.h"
#include "moth_ui/layers/layer_stack.h"
#include "moth_ui/context.h"
#include "example_layer.h"

#include <fstream>

char const* const ExampleApplication::IMGUI_FILE = "imgui.ini";
char const* const ExampleApplication::PERSISTENCE_FILE = "example.json";
ExampleApplication* g_App;

ExampleApplication::ExampleApplication()
    : Application("Example") {
    m_imguiSettingsPath = (std::filesystem::current_path() / IMGUI_FILE).string();
    m_persistentFilePath = std::filesystem::current_path() / PERSISTENCE_FILE;
    std::ifstream persistenceFile(m_persistentFilePath.string());
    if (persistenceFile.is_open()) {
        try {
            persistenceFile >> m_persistentState;
        } catch (std::exception&) {
        }

        if (!m_persistentState.is_null()) {
            m_windowPos = m_persistentState.value("window_pos", m_windowPos);
            m_windowWidth = m_persistentState.value("window_width", m_windowWidth);
            m_windowHeight = m_persistentState.value("window_height", m_windowHeight);
            m_windowMaximized = m_persistentState.value("window_maximized", m_windowMaximized);
        }
    }

    g_App = this;
}

ExampleApplication::~ExampleApplication() {
    std::ofstream ofile(m_persistentFilePath.string());
    if (ofile.is_open()) {
        m_persistentState["current_path"] = std::filesystem::current_path().string();
        m_persistentState["window_pos"] = m_windowPos;
        m_persistentState["window_width"] = m_windowWidth;
        m_persistentState["window_height"] = m_windowHeight;
        m_persistentState["window_maximized"] = m_windowMaximized;
        ofile << m_persistentState;
    }

    g_App = nullptr;
}

void ExampleApplication::SetupLayers() {
    if (m_persistentState.contains("current_path")) {
        std::string const currentPath = m_persistentState["current_path"];
        try {
            std::filesystem::current_path(currentPath);
        } catch (std::exception&) {
            // ...
        }
    }

    moth_ui::Context::GetCurrentContext()->GetFontFactory().LoadProject("assets/fonts.json");


    auto exampleLayer = std::make_unique<ExampleLayer>("assets/layouts/title.mothui");

    m_layerStack = std::make_unique<moth_ui::LayerStack>(m_windowWidth, m_windowHeight, m_windowWidth, m_windowHeight);
    m_layerStack->PushLayer(std::move(exampleLayer));
    m_layerStack->SetEventListener(this);
}
