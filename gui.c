#include <raylib.h>
#include <stdio.h>
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
  const char *wtfLabelText = "WTF";
  const char *interfaceLabelText = "Interface";
  const int fontSize = 24;

  const int windowWidth = 500;
  const int windowHeight = 300;

  Vector2 wtfTextLocation = {10, 60};
  Vector2 interfaceTextLocation = {10, 100};

  AppState state = STATE_IDLE;

  SyncState syncData = {0};

  const int BAR_WIDTH = 200;
  const int btnFontSize = 20;
  const int btnPadX = 20;
  const int btnPadY = 10;

  InitWindow(windowWidth, windowHeight, title);

  // Both bars share the same X, centered in the window
  int barX = (windowWidth - BAR_WIDTH) / 2;
  // Vertically center the 20px bar against the 24px label text
  int barYOffset = (fontSize - 20) / 2;
  // Right-align labels against the bar's left edge (with a small gap)
  int labelRightEdge = barX - 10;
  int wtfLabelWidth = MeasureText(wtfLabelText, fontSize);
  int interfaceLabelWidth = MeasureText(interfaceLabelText, fontSize);

  // Button: wide enough for the longest label, centered in the window
  int w1 = MeasureText("Sync Now",  btnFontSize);
  int w2 = MeasureText("Syncing..", btnFontSize);
  int w3 = MeasureText("Sync Again", btnFontSize);
  int maxBtnTextWidth = w1 > w2 ? (w1 > w3 ? w1 : w3) : (w2 > w3 ? w2 : w3);
  int btnWidth  = maxBtnTextWidth + btnPadX * 2;
  int btnHeight = btnFontSize + btnPadY * 2;
  Rectangle syncButton = {(windowWidth - btnWidth) / 2, 230, btnWidth, btnHeight};

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, syncButton);
    bool clicked = hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color btnColor = (hovering && (state == STATE_IDLE || state == STATE_DONE)) ? DARKGRAY : GRAY;

    if(clicked && state == STATE_IDLE){
        state = STATE_SYNCING;
        syncData.state = STATE_SYNCING;
        CreateThread(NULL, 0, sync_thread, &syncData, 0, NULL);
    }
    if(clicked && state == STATE_DONE){
        syncData = (SyncState){0};
        state = STATE_SYNCING;
        syncData.state = STATE_SYNCING;
        CreateThread(NULL, 0, sync_thread, &syncData, 0, NULL);
    }
    state = (AppState)syncData.state;

    char wtfPercent[16];
    char interfacePercent[16];
    snprintf(wtfPercent, sizeof(wtfPercent), "%.0f%%", syncData.wtf_progress * 100.0f);
    snprintf(interfacePercent, sizeof(interfacePercent), "%.0f%%", syncData.interface_progress * 100.0f);

    BeginDrawing();

    ClearBackground(BLACK);
    DrawRectangleRec(syncButton, btnColor);

    // WTF bar
    DrawRectangle(barX, wtfTextLocation.y + barYOffset, BAR_WIDTH, 20, DARKGRAY);
    DrawRectangle(barX, wtfTextLocation.y + barYOffset, BAR_WIDTH * syncData.wtf_progress, 20, GREEN);
    DrawText(wtfPercent, barX + BAR_WIDTH + 8, wtfTextLocation.y, fontSize, WHITE);

    // Interface bar
    DrawRectangle(barX, interfaceTextLocation.y + barYOffset, BAR_WIDTH, 20, DARKGRAY);
    DrawRectangle(barX, interfaceTextLocation.y + barYOffset, BAR_WIDTH * syncData.interface_progress, 20, GREEN);
    DrawText(interfacePercent, barX + BAR_WIDTH + 8, interfaceTextLocation.y, fontSize, WHITE);

    // Button label — centered inside the button
    const char *btnLabel = (state == STATE_SYNCING) ? "Syncing.."
                         : (state == STATE_DONE)    ? "Sync Again"
                                                    : "Sync Now";
    int btnLabelWidth = MeasureText(btnLabel, btnFontSize);
    DrawText(btnLabel,
             syncButton.x + (syncButton.width  - btnLabelWidth) / 2,
             syncButton.y + (syncButton.height - btnFontSize)   / 2,
             btnFontSize, WHITE);

    //Labels (right-aligned so last character lines up)
    DrawText(wtfLabelText, labelRightEdge - wtfLabelWidth, wtfTextLocation.y, fontSize, WHITE);
    DrawText(interfaceLabelText, labelRightEdge - interfaceLabelWidth, interfaceTextLocation.y, fontSize, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
