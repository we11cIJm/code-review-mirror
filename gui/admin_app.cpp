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

#include <BlindCodeReview/git.hpp>
#include <distributer/distributer.hpp>

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

std::istream& readOneChar(std::istream& input) {
    char tmpC = 0;
    input.get(tmpC);
    return input;
}

void InputTextWithHint(const std::string& label, const std::string& hint, std::string& data, ImGuiInputTextFlags flags = 0) {
    char buffer[256];
    strncpy(buffer, data.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, sizeof(buffer), flags);
    data = buffer;
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

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(39./255, 39./255, 39./255, 1.00f);
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
    fileDialog.SetTitle("Select path to data folder");
    std::filesystem::path workPath = "";
    std::filesystem::path repoName = "";
    std::filesystem::path fileName = "";

    ImGui::FileBrowser fileDialogUrl(ImGuiFileBrowserFlags_ConfirmOnEnter);
    fileDialogUrl.SetTypeFilters({ ".txt" });
    fileDialogUrl.SetTitle("Select path to txt file with urls");
    std::filesystem::path urlPath = "";

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    editor.SetLanguageDefinition(lang);
    editor.SetReadOnly(true);
    editor.SetImGuiChildIgnored(true);

    if (std::filesystem::exists("admin_settings.txt")) {
        std::ifstream adminSettings("admin_settings.txt");
        adminSettings >> workPath >> urlPath >> globalScale;
        if (!std::filesystem::is_directory(workPath)) {
			workPath = "";
        }
        if (!std::filesystem::is_regular_file(urlPath)) {
			urlPath = "";
        }
		adminSettings.close();
    }
    else {
        std::string err = "sth went wrong";
    }

    git::Git gitt;

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

        if (openMain) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(x, y));

            ImGui::Begin("Main part", &openPersonChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);
            // possible to show statistics
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

                ImGui::Text("Didn't set path for data folder");
                if (ImGui::Button("Close")) {
                    openNoPath = false;
                }
                ImGui::End();
            }

            ImVec4 color = style.Colors[ImGuiCol_Button];
            if (workPath != "") style.Colors[ImGuiCol_Button] = ImVec4(43. / 255, 173. / 255, 114. / 255, 1.);
            if (ImGui::Button("Set path to data folder")) {
                fileDialog.Open();
            } ImGui::SameLine();
            style.Colors[ImGuiCol_Button] = color;
            fileDialog.Display();
            if (fileDialog.HasSelected())
            {
                workPath = fileDialog.GetSelected();
            }

            if (urlPath != "") style.Colors[ImGuiCol_Button] = ImVec4(43. / 255, 173. / 255, 114. / 255, 1.);
            if (ImGui::Button("Set path to url file")) {
                fileDialogUrl.Open();
            }
            style.Colors[ImGuiCol_Button] = color;
            fileDialogUrl.Display();
            if (fileDialogUrl.HasSelected())
            {
                urlPath = fileDialogUrl.GetSelected();
            }

            if (std::filesystem::exists(workPath) && std::filesystem::exists(urlPath) &&
                (!cFiles && !sFiles && !cReviews || cReviews || cFiles) && ImGui::Button("Collect files")) {
                cFiles = true;
                cReviews = false;

                std::ifstream readUrl(urlPath);

                std::string line;
                std::vector<std::string> urlVec;
                while (std::getline(readUrl, line)) {
                    urlVec.push_back(line);
                }
                git::CloneByFile(urlPath, workPath);
                git::PullAll(workPath, urlVec);
            }
            if (cFiles || cReviews) ImGui::SameLine();
            if (cFiles && ImGui::Button("Send files")) {
                sFiles = true;
                cFiles = false;

                std::ifstream readUrl(urlPath);
                std::string line;
                std::vector<std::string> urlVec;
                while (std::getline(readUrl, line)) {
                    urlVec.push_back(line);
                }
                FunctionStatus result = distributeFiles(workPath / "repos");
                git::PushAll(workPath, urlVec);
            }
            if ((sFiles || cReviews) && ImGui::Button("Collect reviews")) {
                cReviews = true;
                sFiles = false;

                std::ifstream readUrl(urlPath);
                std::string line;
                std::vector<std::string> urlVec;
                while (std::getline(readUrl, line)) {
                    urlVec.push_back(line);
                }
                git::PullAll(workPath, urlVec);
            }
            if (!cFiles && !sFiles && !cReviews && workPath != "" && urlPath != "") {
                ImGui::Text("Hint: You need to collect files for review");
            }
            if (cFiles) {
                ImGui::Text("Hint: You can send %u files or can still collect them", 0); //waiting for distribution
            }
            if (sFiles) {
                ImGui::Text("Hint: You sent %u files and can collect reviews", 0); //waiting for distribution
            }
            if (cReviews) {
                ImGui::Text("Hint: You collected %u reviews, can still collect them or continue collecting files", 0); //waiting for distribution
            }

            ImGui::End();
        }
        if (openModeChooser) {
            ImGui::SetNextWindowSize(ImVec2(x / 2, y / 2));

            ImGui::Begin("Mode", &openModeChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

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
        if (openPersonChooser) {
            ImGui::SetNextWindowSize(ImVec2(x / 2, y / 2));
            ImGui::Begin("Reviewer", &openPersonChooser, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            ImGui::Text("Choose reviewer's repo"); ImGui::SameLine();

            if (ImGui::Button("Return")) {
                openPersonChooser = false;
                openModeChooser = true;
            }
            for (const auto& entry : std::filesystem::directory_iterator(workPath)) {
                if (std::filesystem::exists(entry.path() / ".git") && ImGui::Button(reinterpret_cast<const char*>(entry.path().filename().u8string().c_str()))) {
                    repoName = entry.path().filename();
                    openPersonChooser = false;
                    openFileChooser = true;
                }
            }
            
            ImGui::End();
        }
        if (openFileChooser) {
            ImGui::SetNextWindowSize(ImVec2(x / 2, y / 2));
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
                    if (ImGui::Button(reinterpret_cast<const char*>(entry.path().filename().u8string().c_str()))) {
                        fileName = entry.path().stem();
                        openFileChooser = false;
                        openEditor = true;

                        std::ifstream readFileTo_review((workPath / repoName / "to_review" / fileName).replace_extension(".cpp"));
                        if (fileCount == 0) numLines = 0; else numLines = std::count(std::istreambuf_iterator<char>(readFileTo_review), std::istreambuf_iterator<char>(), '\n');
                        readFileTo_review.seekg(0, std::ios::beg);
                        std::string str((std::istreambuf_iterator<char>(readFileTo_review)), std::istreambuf_iterator<char>());
                        editor.SetText(str);

                        std::ifstream readFileReview((workPath / repoName / "review" / fileName).replace_extension(".txt"));
                        int num;
                        std::string comment;
                        while (std::getline(readOneChar(readFileReview >> num), comment).good()) reviewData[num] = comment;
                    }
                }
            }

            ImGui::End();
        }
        if (openEditor) {
            auto cpos = editor.GetCursorPosition();
            ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(x / 2., y), 0);

            ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

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

            fileName.replace_extension(".cpp");
            ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s |", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                editor.IsOverwrite() ? "Ovr" : "Ins",
                editor.CanUndo() ? "*" : " ",
                editor.GetLanguageDefinition().mName.c_str()); ImGui::SameLine();
            ImGui::Text("%s", reinterpret_cast<const char*>(fileName.u8string().c_str()));
            fileName.replace_extension("");

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

            ImGui::End();


            ImGui::SetNextWindowPos(ImVec2(x / 2., 0));
            ImGui::SetNextWindowSize(ImVec2(x / 2., .71 * (globalScale * 37 + 18) / 1920 * x));
            bool oi = true;

            ImGui::Begin("Main", &oi, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);

            if (ImGui::Button("Change review file")) {
                openEditor = false;
                openFileChooser = true;
            }

            ImGui::End();


            ImGui::SetNextWindowPos(ImVec2(x / 2., .71 * (globalScale * 37 + 18) / 1920 * x));
            ImGui::SetNextWindowSize(ImVec2(x / 2., y - .71 * (globalScale * 37 + 18) / 1920 * x));
            ImGui::Begin("Comments", &oi, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(.88 * globalScale * x / 1920);

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
                ImGui::SetCursorPos(ImVec2(0, globalScale * 13 / 1920 * x * (v - 1)));
                InputTextWithHint("##com " + std::to_string(v), "comment " + std::to_string(v), reviewData[v], ImGuiInputTextFlags_ReadOnly);
            }
            
            ImGui::End();
        }
        if (openRespondent) {
            ImGui::SetNextWindowSize(ImVec2(x / 2, y / 2));

            ImGui::Begin("Respondent", &openRespondent, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);
            ImGui::Text("Stay tuned");
            if (ImGui::Button("Return")) {
                openRespondent = false;
                openModeChooser = true;
            }
            //not that simple, waiting for distribution
            ImGui::End();
        }

        if (io.KeyCtrl) {
            globalScale += io.MouseWheel * .1;
            globalScale = std::max(.8f, globalScale);
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

    std::ofstream adminSettings("admin_settings.txt");
    adminSettings << workPath << std::endl << urlPath << std::endl << globalScale;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
