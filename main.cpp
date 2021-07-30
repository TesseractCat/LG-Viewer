extern "C" {
    #include "raylib.h"
    #include "rlgl.h"
}
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <string>
#include <regex>
#include <iostream>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    // Window Config
    const int screenWidth = 400;
    const int screenHeight = 400;
    bool fullscreen = false;
    
    // File Dropping Support
    int droppedCount = 0;
    char **droppedFiles = { 0 };
    
    // Window
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "LKG Quilt Viewer");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    SetShapesTexture(rlGetTextureDefault(), { 0.0f, 0.0f, 1.0f, 1.0f });
    
    // LKG Config
    Shader lkgFragment = LoadShader(0, "./quilt.fs");
    
    Texture2D quilt = LoadTexture("./quilt.png");
    
    float dpi = 324.0;
    float pitch = 52.56658151511047;
    float slope = -7.145505158882189;
    float center = 0.9997089252844671;
    
    int ri = 0;
    int bi = 2;
    
    float tile[2] = {(float)5,(float)9};
    
    // Initialize shader uniforms
    int quiltTexLoc = GetShaderLocation(lkgFragment, "texture1");
    int pitchLoc = GetShaderLocation(lkgFragment, "pitch");
    SetShaderValue(lkgFragment, pitchLoc, &pitch, SHADER_UNIFORM_FLOAT);
    int slopeLoc = GetShaderLocation(lkgFragment, "slope");
    SetShaderValue(lkgFragment, slopeLoc, &slope, SHADER_UNIFORM_FLOAT);
    int centerLoc = GetShaderLocation(lkgFragment, "center");
    SetShaderValue(lkgFragment, centerLoc, &center, SHADER_UNIFORM_FLOAT);
    int dpiLoc = GetShaderLocation(lkgFragment, "dpi");
    SetShaderValue(lkgFragment, dpiLoc, &dpi, SHADER_UNIFORM_FLOAT);
    int riLoc = GetShaderLocation(lkgFragment, "ri");
    SetShaderValue(lkgFragment, riLoc, &ri, SHADER_UNIFORM_INT);
    int biLoc = GetShaderLocation(lkgFragment, "bi");
    SetShaderValue(lkgFragment, biLoc, &bi, SHADER_UNIFORM_INT);
    int tileLoc = GetShaderLocation(lkgFragment, "tile");
    SetShaderValue(lkgFragment, tileLoc, tile, SHADER_UNIFORM_VEC2);

    SetTargetFPS(60);               // Set our viewer to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyPressed(KEY_F)) {
            fullscreen = !fullscreen;
            if (fullscreen) {
                SetWindowState(FLAG_WINDOW_UNDECORATED);
                SetWindowState(FLAG_WINDOW_TOPMOST);
                MaximizeWindow();
                SetWindowSize(1536, 2048);
            } else {
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
                ClearWindowState(FLAG_WINDOW_TOPMOST);
                RestoreWindow();
                SetWindowSize(400, 400);
            }
        }
        
        if (IsFileDropped()) {
            droppedFiles = GetDroppedFiles(&droppedCount);
            UnloadTexture(quilt);
            std::string fileName(droppedFiles[0]);
            
            std::regex png_regex(".png$");
            if (!std::regex_search(fileName, png_regex)) {
                //FIXME: Check for valid file types
            }
            
            std::regex qs_regex("qs(\\d+)x(\\d+).*(\\.png|\\.jpg|\\.jpeg)$");
            auto qs_regex_begin = std::sregex_iterator(fileName.begin(), fileName.end(), qs_regex);
            auto qs_regex_end = std::sregex_iterator();
            
            for (auto i = qs_regex_begin; i != qs_regex_end; i++) {
                std::smatch match = *i;
                std::cout << "[FILE LOADED]: Tiles " << match[1] << "x" << match[2] << std::endl;
                tile[0] = stoi(match[1]);
                tile[1] = stoi(match[2]);
                SetShaderValue(lkgFragment, tileLoc, tile, SHADER_UNIFORM_VEC2);
            }
            
            quilt = LoadTexture(fileName.c_str());
            ClearDroppedFiles();
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            //DrawText("Please drag and drop a quilt", 70, 180, 20, LIGHTGRAY);
            //DrawTexture(quilt, 0, 0, WHITE);
            
            BeginShaderMode(lkgFragment);
                SetShaderValueTexture(lkgFragment, quiltTexLoc, quilt);
                DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), WHITE);
            EndShaderMode();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(lkgFragment);
    UnloadTexture(quilt);
    ClearDroppedFiles();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
