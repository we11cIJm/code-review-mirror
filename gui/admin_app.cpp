#define NOMINMAX

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cmath>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>

#include "imgui_filebrowser-src/imfilebrowser.h"
#include "imguicolortextedit-src/TextEditor.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


void pullAll(std::filesystem::path root, std::vector<std::string> urlList) {

}

void pushAll(std::filesystem::path root, std::vector<std::string> urlList) {

}

void distribute() {

}

std::istream& readOneChar(std::istream& input) {
    char tmpC = 0;
    input.get(tmpC);
    return input;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow* window = glfwCreateWindow(3000, 2000, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float globalScale = 1.1;

    bool cFiles = false;
    bool sFiles = false;
    bool cReviews = false;

    bool openMain = true;
    bool openNoPath = false;
    bool openModeChooser = false;
    bool openRespondent = false;
    bool openPersonChooser = false;
    bool openFileChooser = false;

    bool toScrollEditor = false;
    bool toScrollComments = false;
    float prevScrollEditor = 0;
    float prevScrollComments = 0;
    float scrollEditor = 0;
    float scrollComments = 0;

    std::map<int, std::string> reviewData;
    int numLines = 0;

    bool openChanger = false;
    bool openEditor = false;

    ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_ConfirmOnEnter);
    std::filesystem::path fileName;
    fileDialog.SetTitle("Select path to repo");
    std::filesystem::path workPath = "";
    std::filesystem::path repoName = "";

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    editor.SetLanguageDefinition(lang);
    editor.SetReadOnly(true);
    editor.SetImGuiChildIgnored(true);

    if (std::filesystem::exists("settings.txt")) {
        std::ifstream settings("settings.txt");
        settings >> workPath;
    }
    else {
        std::string err = "sth went wrong";
    }

#ifdef __EMSCRIPTEN__
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int x, y;
        glfwGetWindowSize(window, &x, &y);

        if (openPersonChooser) {
            ImGui::Begin("Reviewer", &openPersonChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            ImGui::Text("Choose reviewer's repo"); ImGui::SameLine();

            if (ImGui::Button("Return")) {
                openPersonChooser = false;
                openModeChooser = true;
            }
            for (const auto& entry : std::filesystem::directory_iterator(workPath)) {
                if (std::filesystem::exists(entry.path() / ".git") && ImGui::Button(entry.path().string().c_str())) {
                    repoName = entry.path();
                    openPersonChooser = false;
                    openFileChooser = true;
                }
            }
            
            ImGui::End();
        }
        if (openFileChooser) {
            ImGui::Begin("PersonChooser", &openFileChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            ImGui::Text("Choose reviewer's file"); ImGui::SameLine();

            if (ImGui::Button("Return")) {
                openFileChooser = false;
                openPersonChooser = true;
            }
            int fileCount = 0;
            for (const auto& entry : std::filesystem::directory_iterator(workPath / repoName / "to_review")) {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    ++fileCount;
                    if (ImGui::Button(entry.path().string().c_str())) {
                        fileName = entry.path().stem();
                        openFileChooser = false;
                        openEditor = true;
                    }
                }
            }
            std::ifstream readFirstFileTo_review((workPath / repoName / "to_review" / fileName).replace_extension(".cpp"));
            std::ifstream readFirstFileReview((workPath / repoName / "review" / fileName).replace_extension(".txt"));
            int num;
            std::string comment;
            while (std::getline(readOneChar(readFirstFileReview >> num), comment).good()) reviewData[num] = comment;
            if (fileCount == 0) numLines = 0; else numLines = std::count(std::istreambuf_iterator<char>(readFirstFileTo_review), std::istreambuf_iterator<char>(), '\n');
            readFirstFileTo_review.seekg(0, std::ios::beg);
            std::string str((std::istreambuf_iterator<char>(readFirstFileTo_review)), std::istreambuf_iterator<char>());
            editor.SetText(str);

            ImGui::End();
        }
        if (openEditor) {
            auto cpos = editor.GetCursorPosition();
            ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(x / 2., y), 0);

            ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            if (ImGui::Button("Return")) {
                openEditor = false;
                openFileChooser = true;
            }
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Quit", "Alt-F4"))
                        break;
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                        editor.Copy();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Select all", nullptr, nullptr))
                        editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Dark palette"))
                        editor.SetPalette(TextEditor::GetDarkPalette());
                    if (ImGui::MenuItem("Light palette"))
                        editor.SetPalette(TextEditor::GetLightPalette());
                    if (ImGui::MenuItem("Retro blue palette"))
                        editor.SetPalette(TextEditor::GetRetroBluePalette());
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            auto mbstr = fileName.c_str();
//            char mbstr[256];
            fileName.replace_extension(".cpp");
//            wcstombs(mbstr, fileName.c_str(), sizeof(mbstr));
            fileName.replace_extension("");
            ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                editor.IsOverwrite() ? "Ovr" : "Ins",
                editor.CanUndo() ? "*" : " ",
                editor.GetLanguageDefinition().mName.c_str(), mbstr);

            ImGui::BeginChild("TextEditor", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoMove);
            if (toScrollEditor) {
                ImGui::SetScrollY(scrollComments * ImGui::GetScrollMaxY());
                toScrollEditor = false;
                prevScrollEditor = scrollEditor;
                scrollEditor = scrollComments;
            }
            else {
                prevScrollEditor = scrollEditor;
                scrollEditor = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
            }
            editor.Render("TextEditor");
            ImGui::EndChild();

            if (io.KeyCtrl) {
                globalScale += io.MouseWheel * 0.1f;
                globalScale = std::max(0.8f, globalScale);
            }

            ImGui::End();


            ImGui::SetNextWindowPos(ImVec2(x / 2., 0));
            ImGui::SetNextWindowSize(ImVec2(x / 2., 0.71 * (globalScale * 37 + 18) / 1920 * x));
            bool oi = true;

            ImGui::Begin("Main", &oi, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            if (ImGui::Button("Change review file")) {
                openEditor = false;
                openFileChooser = true;
            }

            ImGui::End();


            ImGui::SetNextWindowPos(ImVec2(x / 2., 0.71 * (globalScale * 37 + 18) / 1920 * x));
            ImGui::SetNextWindowSize(ImVec2(x / 2., y - 0.71 * (globalScale * 37 + 18) / 1920 * x));

            ImGui::Begin("Comments", &oi, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(0.88 * globalScale * x / 1920);

            ImGui::SetCursorPosY(globalScale * 13 / 1920 * x * numLines);
            ImGui::Dummy(ImVec2(0, globalScale * 22 / 1920 * x));

            if (toScrollComments) {
                ImGui::SetScrollY(scrollEditor * ImGui::GetScrollMaxY());
                toScrollComments = false;
                prevScrollComments = scrollComments;
                scrollComments = scrollEditor;
            }
            else {
                prevScrollComments = scrollComments;
                scrollComments = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
            }

            for (auto el : reviewData) {
                int v = el.first;
                ImGui::SetCursorPos(ImVec2(5, 7 + globalScale * 13 / 1920 * x * (v - 1)));
                char* tmp = new char(128);
                tmp = const_cast<char*>(el.second.c_str());
                ImGui::InputTextWithHint((std::string("##com ") + std::to_string(v)).c_str(), ("comment " + std::to_string(v)).c_str(), tmp, 128, ImGuiInputTextFlags_ReadOnly);
            }
            
            ImGui::End();
        }
        if (openRespondent) {
            ImGui::Begin("Respondent", &openRespondent, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::Text("Stay tuned");
            if (ImGui::Button("Return")) {
                openRespondent = false;
                openModeChooser = true;
            }
            //not that simple
            ImGui::End();
        }
        if (openMain) {
            ImGui::Begin("Main code", &openPersonChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            if ((!cFiles && !sFiles && !cReviews || cReviews || cFiles) && ImGui::Button("Collect files")) {
                cFiles = true;
                cReviews = false;
                pullAll(workPath, std::vector<std::string>());
            }
            if (cFiles && ImGui::Button("Send files")) {
                sFiles = true;
                cFiles = false;
                distribute();
                pushAll(workPath, std::vector<std::string>());
            }
            if ((sFiles || cReviews) && ImGui::Button("Collect reviews")) {
                cReviews = true;
                sFiles = false;
                pullAll(workPath, std::vector<std::string>());
            }
            ImGui::SameLine();
            //show statistics
            if (ImGui::Button("Check reviews")) {
                if (workPath == "") {
                    openNoPath = true;
                }
                else {
                    openModeChooser = true;
                    openMain = false;
                }
            }
            if (openNoPath) {
                ImGui::SetNextWindowPos(ImVec2(0.4 * x, 0.4 * y));
                ImGui::SetNextWindowSize(ImVec2(0.2 * x, 0.2 * y));

                ImGui::Begin("Didn't set path for data folder", &openNoPath, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                ImGui::SetWindowFontScale(globalScale * x / 1920);

                if (ImGui::Button("Close")) {
                    openNoPath = false;
                }
                ImGui::End();
            }
            if (ImGui::Button("Set path to data folder")) {
                fileDialog.Open();
            }
            fileDialog.Display();

            if (fileDialog.HasSelected())
            {
                workPath = fileDialog.GetSelected();
            }

            ImGui::End();
        }
        if (openModeChooser) {
            ImGui::Begin("Mode", &openModeChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

            ImGui::Text("Do you want to choose a reviewer or a respondent?");
            ImGui::SameLine();

            if (ImGui::Button("Return")) {
                openModeChooser = false;
                openMain = true;
            }
            if (ImGui::Button("Reviewer")) {
                openModeChooser = false;
                openPersonChooser = true;
            }
            if (ImGui::Button("Respondent")) {
                openModeChooser = false;
                openRespondent = true;
            }

            ImGui::End();
        }

        if (std::abs(scrollEditor - scrollComments) > 2.0 / y) {
            if (prevScrollEditor != scrollEditor) {
                toScrollComments = true;
            }
            else {
                toScrollEditor = true;
            }
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    std::ofstream settings("settings.txt");
    settings << workPath;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
