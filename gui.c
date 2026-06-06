#include <raylib.h>
#include"sync.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

typedef enum {
    STATE_IDLE,
    STATE_SYNCING,
    STATE_DONE,
    STATE_ERROR
} AppState;

DWORD WINAPI sync_thread(LPVOID param){
    SyncState *s = (SyncState *)param;

    sync_main(0, NULL, s);

    s->state = STATE_DONE;
        return 0;
}

int main() {
  const char *title = "WoW Addon Sync";
  const int windowWidth = 500;
  const int windowHeight = 300;

  Vector2 wtfTextLocation = {10, 20};
  Vector2 interfaceTextLocation = {10, 60};
  // Button setup
  Rectangle syncButton = {200, 230, 120, 40};

  AppState state = STATE_IDLE;

  SyncState syncData = {0};

  const int BAR_WIDTH = 200;

  InitWindow(windowWidth, windowHeight, title);
  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, syncButton);
    bool clicked = hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color btnColor = (hovering && state == STATE_IDLE) ? DARKGRAY : GRAY;

    if(clicked && state == STATE_IDLE){
        state = STATE_SYNCING;
        syncData.state = STATE_SYNCING;
        CreateThread(NULL, 0, sync_thread, &syncData, 0, NULL);
    }
    state = (AppState)syncData.state;

    BeginDrawing();

    ClearBackground(BLACK);
    DrawRectangleRec(syncButton, btnColor);
    // WTF bar
    DrawRectangle(wtfTextLocation.x + 100, wtfTextLocation.y + 10, BAR_WIDTH, 20, DARKGRAY);  // track
    DrawRectangle(wtfTextLocation.x + 100, wtfTextLocation.y + 10, BAR_WIDTH * syncData.wtf_progress, 20, GREEN);  // fill

    // Interface bar
    DrawRectangle(interfaceTextLocation.x + 210, interfaceTextLocation.y + 10, BAR_WIDTH, 20, DARKGRAY);  // track
    DrawRectangle(interfaceTextLocation.x + 210, interfaceTextLocation.y + 10, BAR_WIDTH * syncData.interface_progress, 20, GREEN);  // fill


    if(state == STATE_IDLE){
        DrawText("Sync Now", syncButton.x + 10, syncButton.y + 12, 20, WHITE);
    } else if(state == STATE_SYNCING){
        DrawText("Syncing..", syncButton.x + 10, syncButton.y + 12, 20, WHITE);
    } else if(state == STATE_DONE){
        DrawText("Sync Done!", syncButton.x + 10, syncButton.y + 12, 20, WHITE);
    }


    DrawText("WTF", 10, 20, 40, WHITE);
    DrawText("Interface", 10, 60, 40, WHITE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
