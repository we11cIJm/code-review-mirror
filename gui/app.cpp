#define NOMINMAX

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>

#include <imgui_filebrowser-src/imfilebrowser.h>
#include <imguicolortextedit-src/TextEditor.h>

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

/*void send() {

}*/

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(.45f, .55f, .60f, 1.00f);
    float globalScale = 1.1;

    bool toScrollEditor = false;
    bool toScrollComments = false;
    float prevScrollEditor = 0;
    float prevScrollComments = 0;
    float scrollEditor = 0;
    float scrollComments = 0;

    std::map<int, std::string> reviewData;
    int numLines = 0;

    bool openChanger = false;
    int fileCount = 0;

    ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_ConfirmOnEnter);
    std::filesystem::path fileName;
    bool open_not_a_repo = false;
    fileDialog.SetTitle("Select path to repo");
    std::filesystem::path workPath = "";

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    editor.SetLanguageDefinition(lang);
    editor.SetReadOnly(true);
    editor.SetImGuiChildIgnored(true);

    bool open = 0;

    if (std::filesystem::exists("settings.txt")) {
        std::ifstream settings("settings.txt");
        settings >> workPath;
        if (std::filesystem::exists(workPath / ".git")) {
            for (const auto& entry : std::filesystem::directory_iterator(workPath / "to_review")) {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    ++fileCount;
                    if (fileCount == 1) {
                        fileName = entry.path().stem();
                    }
                }
            }
            std::ifstream readFirstFileTo_review((workPath / "to_review" / fileName).replace_extension(".cpp"));
            std::ifstream readFirstFileReview((workPath / "review" / fileName).replace_extension(".txt"));
            int num;
            std::string comment;
            while (std::getline(readOneChar(readFirstFileReview >> num), comment).good()) reviewData[num] = comment;
            if (fileCount == 0) numLines = 0; else numLines = std::count(std::istreambuf_iterator<char>(readFirstFileTo_review), std::istreambuf_iterator<char>(), '\n');
            readFirstFileTo_review.seekg(0, std::ios::beg);
            std::string str((std::istreambuf_iterator<char>(readFirstFileTo_review)), std::istreambuf_iterator<char>());
            editor.SetText(str);
        }
        else {
            workPath = "";
        }
    }
    else {
        std::string err = "sth went wrong";
    }
    std::filesystem::path readPath = workPath;

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

        {
            ImGui::SetNextWindowPos(ImVec2(x / 2., 0));
            ImGui::SetNextWindowSize(ImVec2(x / 2., .71 * (globalScale * 37 + 18) / 1920 * x));
            bool oi = true;
            ImGui::Begin("Main", &oi, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowFontScale(globalScale * x / 1920);
            ImGui::Text("Write your comments here ");

            /*ImGui::SameLine();
            if (ImGui::Button("Send review")) {
                if (workPath == "") {
                    open = true;
                }
                else {
                    send();
                }
            }*/

            ImGui::SameLine();
            if (ImGui::Button("Set path to repo")) {
                fileDialog.Open();
            }
            fileDialog.Display();

            ImGui::SameLine();
            if (ImGui::Button("Change reviewing file")) {
                openChanger = true;
            }
            if (openChanger) {
                ImGui::SetNextWindowPos(ImVec2(.4 * x, .4 * y));
                ImGui::SetNextWindowSize(ImVec2(.2 * x, .2 * y));
                ImGui::Begin("File changer window", &openChanger, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                for (const auto& entry : std::filesystem::directory_iterator(workPath / "to_review")) {
                    if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                        if (ImGui::Button((std::string("Review file ") + entry.path().filename().string()).c_str(), ImVec2(globalScale * 120 / 1920 * x, globalScale * 13 / 1920 * x))) {
                            openChanger = false;

                            std::ofstream review((workPath / "review" / fileName).replace_extension(".txt"));
                            for (auto el : reviewData) {
                                if (el.second.size() != 0) {
                                    review << el.first << ' ' << el.second << std::endl;
                                }
                            }
                            reviewData.clear();

                            fileName = entry.path().stem();

                            std::ifstream readFirstFileReview((workPath / "review" / fileName).replace_extension(".txt"));
                            int num;
                            std::string comment;
                            while (std::getline(readOneChar(readFirstFileReview >> num), comment).good()) reviewData[num] = comment;

                            std::ifstream readFirstFileTo_review((workPath / "to_review" / fileName).replace_extension(".cpp"));
                            if (fileCount == 0) numLines = 0; else numLines = std::count(std::istreambuf_iterator<char>(readFirstFileTo_review), std::istreambuf_iterator<char>(), '\n');
                            readFirstFileTo_review.seekg(0, std::ios::beg);
                            std::string str((std::istreambuf_iterator<char>(readFirstFileTo_review)), std::istreambuf_iterator<char>());
                            editor.SetText(str);
                        }
                    }
                }
                
                if (ImGui::Button("Close")) {
                    openChanger = false;
                }
                ImGui::End();
            }

            if (workPath != readPath) {
                workPath = readPath;
                fileCount = 0;
                for (const auto& entry : std::filesystem::directory_iterator(workPath / "to_review")) {
                    if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                        ++fileCount;
                        if (fileCount == 1) {
                            fileName = entry.path().stem();
                        }
                    }
                }
                std::ifstream readFirstFileTo_review((workPath / "to_review" / fileName).replace_extension(".cpp"));
                if (fileCount == 0) numLines = 0; else numLines = std::count(std::istreambuf_iterator<char>(readFirstFileTo_review), std::istreambuf_iterator<char>(), '\n');
                readFirstFileTo_review.seekg(0, std::ios::beg);
                std::string str((std::istreambuf_iterator<char>(readFirstFileTo_review)), std::istreambuf_iterator<char>());
                editor.SetText(str);
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
            }

            int rem = 0;
            for (auto el : reviewData) {
                int v = el.first;
                ImGui::SetCursorPos(ImVec2(0, globalScale * 13 / 1920 * x * (v - 1)));
                if (ImGui::Button((std::string("Remove comment ") + std::to_string(v)).c_str(), ImVec2(globalScale * 120 / 1920 * x, globalScale * 13 / 1920 * x))) {
                    rem = v;
                }
                ImGui::SameLine();
                InputTextWithHint("##com " + std::to_string(v), "comment " + std::to_string(v), reviewData[v]);
            }
            if (rem) {
                reviewData.extract(rem);
            }
            prevScrollComments = scrollComments;
            scrollComments = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
            ImGui::End();

            if (open) {
                ImGui::SetNextWindowPos(ImVec2(.4 * x, .4 * y));
                ImGui::SetNextWindowSize(ImVec2(.2 * x, .2 * y));
                ImGui::Begin("Didn't set path for review yet", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                ImGui::SetWindowFontScale(globalScale * x / 1920);
                if (ImGui::Button("Close")) {
                    open = false;
                }
                ImGui::End();
            }
        }

        auto cpos = editor.GetCursorPosition();
        ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(x / 2., y), 0);

        if (workPath != "") {
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
            ImGui::Text("%s", fileName.u8string().c_str());
            
            fileName.replace_extension("");

            if ((ImGui::IsKeyPressed(ImGuiKey_Tab) && ImGui::GetIO().KeyCtrl)) {
                int line = editor.GetCursorPosition().mLine + 1;

                if (reviewData.find(line) == reviewData.end()) {
                    reviewData[line] = "";
                }
            }
            ImGui::BeginChild("TextEditor", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoMove);
            if (toScrollEditor) {
                ImGui::SetScrollY(scrollComments * ImGui::GetScrollMaxY());
                toScrollEditor = false;
            }
            editor.Render("TextEditor");
            prevScrollEditor = scrollEditor;
            scrollEditor = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
            ImGui::EndChild();

            ImGui::End();
        }

        if (fileDialog.HasSelected())
        {
            if (std::filesystem::exists(fileDialog.GetSelected() / ".git")) {
                readPath = fileDialog.GetSelected();
            }
            else {
                open_not_a_repo = true;
                fileDialog.Close();
            }
        }
        if (open_not_a_repo) {
            ImGui::SetNextWindowPos(ImVec2(.4 * x, .4 * y));
            ImGui::SetNextWindowSize(ImVec2(.2 * x, .2 * y));
            ImGui::Begin("Not a git repo");
            if (ImGui::Button("Close")) {
                open_not_a_repo = false;
                fileDialog.Open();
            }
            ImGui::End();
        }

        if (io.KeyCtrl) {
            globalScale += io.MouseWheel * .1f;
            globalScale = std::max(.8f, globalScale);
        }

        if (std::abs(scrollEditor - scrollComments) > 2.0 / y) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
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
    if (workPath != "") {
        std::ofstream review((workPath / "review" / fileName).replace_extension(".txt"));
        for (auto el : reviewData) {
            if (el.second.size() != 0) {
                review << el.first << ' ' << el.second << std::endl;
            }
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
