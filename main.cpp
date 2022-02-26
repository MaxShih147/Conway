// ! Reference: https://conwaylife.com/wiki/

#include <algorithm>
#include <iostream>
#include <codecvt>
#include <string>
#include <random>
#include <memory>
#include <vector>
#include <map>

#include "Conway.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

ImFont* g_font_12 = nullptr;
ImFont* g_font_20 = nullptr;
ImFont* g_font_24 = nullptr;
ImVec2 g_window_pos_default(400, 50);
ImVec2 g_window_pos = g_window_pos_default;
int g_frame_count = 0;

std::shared_ptr<Conway> g_conway = nullptr;

ImGuiWindowFlags g_main_window_flags = ImGuiWindowFlags_NoDecoration;


// Generate today random answer from answer list.
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-10, 10);


// ! Layout Section
// ! - Window
ImVec2 puzzelWindowSize(600, 800);
ImVec2 puzzelWindowPosition(400, 50);
// ! - Keyboard
ImVec2 keyboardSize(40, 50);
int keyboardSpacingY = 10;
int keyboardStartPositionX = 55;       // should be function of above parameters.
int keyboardStartPositionY = 560;      // should be function of above parameters.
ImVec2 keyboardStartPositionPerLine[3] // should be function of above parameters.
{
    ImVec2(keyboardStartPositionX, keyboardStartPositionY),
    ImVec2(keyboardStartPositionX + 24, keyboardStartPositionY + keyboardSize.y + keyboardSpacingY),
    ImVec2(keyboardStartPositionX + 72, keyboardStartPositionY + 2 * (keyboardSize.y + keyboardSpacingY))
};
// ! Enter Button
ImVec2 enterButtonSize(60, 50);
int enterButtonSpacingY = 10;
int enterButtonStartPositionX = 59;       // should be function of above parameters.
int enterButtonStartPositionY = keyboardStartPositionPerLine[2].y;
// ! Back Button
ImVec2 backButtonSize(60, 50);
int backButtonSpacingY = 10;
int backButtonStartPositionX = 463;       // should be function of above parameters.
int backButtonStartPositionY = keyboardStartPositionPerLine[2].y;
// ! Share Button
ImVec2 shareButtonSize(180, 50);
float shareButtonRound = 20.0f;
int shareButtonStartPositionX = 100;       // should be function of above parameters.
int shareButtonStartPositionY = 230;
// ! New Game Button
ImVec2 newGameButtonSize(180, 50);
float newGameButtonRound = 20.0f;
int newGameButtonStartPositionX = 100;       // should be function of above parameters.
int newGameButtonStartPositionY = 300;
// ! - Guess Panel
ImVec2 guessPanelSize(70, 70);
float guessPanelRound = 35.0f;
int guessPanelSpacingX = 10;
int guessPanelSpacingY = 10;
int guessPanelStartPositionX = 100;    // should be function of above parameters.
int guessPanelStartPositionY = 55;     // should be function of above parameters.

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

ImColor backgroundColor = ImColor::HSV(120.0f / 360.0f, 1.0f, 0.024f);

class ButtonColor
{
public:
    ButtonColor() {}
    ButtonColor(ImVec4 _normal, ImVec4 _hovered, ImVec4 _active)
        : normal(_normal), hovered(_hovered), active(_active)
    {
    }
    ImVec4 normal;
    ImVec4 hovered;
    ImVec4 active;
};

ButtonColor enterButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor backButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor newGameButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor shareButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);

static std::vector<std::vector<char>> lettersPerLines
{
        {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
        {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
        {'Z', 'X', 'C', 'V', 'B', 'N', 'M'}
};

static std::map<char, bool> fixedStateToCorrect
{
    {'A', false}, {'B', false}, {'C', false}, {'D', false}, {'E', false},
    {'F', false}, {'G', false}, {'H', false}, {'I', false}, {'J', false},
    {'K', false}, {'L', false}, {'M', false}, {'N', false}, {'O', false},
    {'P', false}, {'Q', false}, {'R', false}, {'S', false}, {'T', false},
    {'U', false}, {'V', false}, {'W', false}, {'X', false}, {'Y', false},
    {'Z', false}
};


std::map<std::string, ImGuiKey> keyNameMap
{
    {"A", ImGuiKey_A}, {"B", ImGuiKey_B}, {"C", ImGuiKey_C}, {"D", ImGuiKey_D}, {"E", ImGuiKey_E},
    {"F", ImGuiKey_F}, {"G", ImGuiKey_G}, {"H", ImGuiKey_H}, {"I", ImGuiKey_I}, {"J", ImGuiKey_J},
    {"K", ImGuiKey_K}, {"L", ImGuiKey_L}, {"M", ImGuiKey_M}, {"N", ImGuiKey_N}, {"O", ImGuiKey_O},
    {"P", ImGuiKey_P}, {"Q", ImGuiKey_Q}, {"R", ImGuiKey_R}, {"S", ImGuiKey_S}, {"T", ImGuiKey_T},
    {"U", ImGuiKey_U}, {"V", ImGuiKey_V}, {"W", ImGuiKey_W}, {"X", ImGuiKey_X}, {"Y", ImGuiKey_Y},
    {"Z", ImGuiKey_Z}, {"enter", ImGuiKey_Enter}, {"back", ImGuiKey_Backspace}
};

void ShowThemeEditorWindow()
{
    static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
    static bool alpha_preview = true;
    static bool alpha_half_preview = false;
    static bool drag_and_drop = true;
    static bool options_menu = true;
    static bool hdr = false;
    ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

    ImGui::Text("Color widget HSV with Alpha:");
    ImGui::ColorEdit4("MyColor##2", (float*)&backgroundColor.Value, ImGuiColorEditFlags_DisplayHSV | misc_flags);
    //backgroundColor.Value = color;
}

void ShowConwayWindow()
{
    static ImVector<ImVec2> points;
    static ImVec2 scrolling(0.0f, 0.0f);
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = false;
    static bool adding_line = false;
    static bool opt_enable_pan = false;
    const float gridStep = 20.0f;

    //ImGui::Checkbox("Enable grid", &opt_enable_grid);
    //ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
    //ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

    // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
    // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
    // To use a child window instead we could use, e.g:
    //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
    //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
    //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
    //      ImGui::PopStyleColor();
    //      ImGui::PopStyleVar();
    //      [...]
    //      ImGui::EndChild();

    // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    //ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    ImVec2 fieldSize(g_conway->GetWidth(), g_conway->GetHeight());
    ImVec2 canvas_sz(fieldSize.x * gridStep, fieldSize.y * gridStep);
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    //ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    //const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held
    const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    // Add first and second point
 /*   if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        points.push_back(mouse_pos_in_canvas);
        points.push_back(mouse_pos_in_canvas);
        adding_line = true;
    }
    if (adding_line)
    {
        points.back() = mouse_pos_in_canvas;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            adding_line = false;
    }*/

    // Pan (we use a zero mouse threshold when there's no context menu)
    // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
    const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
    if (/*is_active && */opt_enable_pan && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    //if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
    //    ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    //if (ImGui::BeginPopup("context"))
    //{
    //    if (adding_line)
    //        points.resize(points.size() - 2);
    //    adding_line = false;
    //    if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
    //    if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
    //    ImGui::EndPopup();
    //}

    // Draw grid + all lines in the canvas
    //draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    if (opt_enable_grid)
    {
        for (float x = fmodf(scrolling.x, gridStep); x < canvas_sz.x; x += gridStep)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling.y, gridStep); y < canvas_sz.y; y += gridStep)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
    }
    //for (int n = 0; n < points.Size; n += 2)
    //    draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
    //draw_list->PopClipRect();


    static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
    ImGui::ColorEdit4("Color", &colf.x);
    const ImU32 col = ImColor(colf);
    int spacing = 2;

    int sz = gridStep - 2 * spacing;
    const float rounding = sz / 5.0f;
    static float thickness = 0.0f;
    //const ImDrawFlags corners_tl_br = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;

    auto _field = g_conway->GetField();
    for (auto i = 0; i < fieldSize.x; ++i)
    {
        for (auto j = 0; j < fieldSize.y; ++j)
        {
            int x = 10 + canvas_p0.x + i * gridStep + spacing;
            int y = 10 + canvas_p0.y + j * gridStep + spacing;

            if (_field[i][j] != nullptr)
            {
                draw_list->AddRectFilled(
                    ImVec2(x + 1, y + 1),
                    ImVec2(x + sz, y + sz),
                    col,
                    rounding,
                    ImDrawFlags_None/*,
                    thickness*/
                );
            }
        }
    }

    ++g_frame_count;
    if (g_frame_count >= 40)
    {
        g_conway->Update();
        g_frame_count = 0;
    }
}

#if defined(_DEBUG)
int main()
#else
int WinMain()
#endif
{
//    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Conway - Game of Life", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_max_dev_windows = true;
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    g_font_12 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 12);
    g_font_20 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20);
    g_font_24 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 24);

    // ! Algorithm Section
    g_conway = std::make_shared<Conway>();

    extern ConwayPattern octagon_2;
    g_conway->Start(octagon_2._pattern);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_max_dev_windows)
        {
            //ImGui::SetNextWindowPos(g_window_pos);
            //ImGui::SetNextWindowSize(puzzelWindowSize);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            ImGui::SetNextWindowPos(ImVec2(.0f, .0f));
            ImGui::SetNextWindowSize(ImVec2(width, height));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(backgroundColor)); // Set window background to red
            ImGui::Begin(
                "Conway", 
                &show_max_dev_windows, 
                g_main_window_flags
            );
            ShowConwayWindow();
            ImGui::PopStyleColor();
            ImGui::End();

            //ImGui::Begin(
            //    "Theme Editor",
            //    &show_max_dev_windows/*,
            //    g_main_window_flags*/
            //);
            //ShowThemeEditorWindow();
            //ImGui::End();
        }

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}