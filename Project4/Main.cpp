
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include "icb_gui.h"

#define NUM_PHILOSOPHERS 5
#define M_PI 3.14159265358979323846

HANDLE semaphores[NUM_PHILOSOPHERS];
int philosopherStates[NUM_PHILOSOPHERS] = { 0 };
ICBYTES panel;
int FRM1;

int philosopherPositionsX[NUM_PHILOSOPHERS];
int philosopherPositionsY[NUM_PHILOSOPHERS];
int forkPositionsX[NUM_PHILOSOPHERS];
int forkPositionsY[NUM_PHILOSOPHERS];
int platePositionsX[NUM_PHILOSOPHERS];
int platePositionsY[NUM_PHILOSOPHERS];
int centerPlateX, centerPlateY;
int centerTablePlateX, centerTablePlateY;  


void CalculatePositions(int panelStartY, int panelHeight) {
    int centerX = 400;
    int centerY = panelStartY + (panelHeight / 2);

    centerPlateX = centerX;
    centerPlateY = centerY;
    centerTablePlateX = centerX; 
    centerTablePlateY = centerY;

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopherPositionsX[i] = centerX + 200 * cos(i * 2 * M_PI / NUM_PHILOSOPHERS);
        philosopherPositionsY[i] = centerY + 200 * sin(i * 2 * M_PI / NUM_PHILOSOPHERS);

        platePositionsX[i] = centerX + 120 * cos(i * 2 * M_PI / NUM_PHILOSOPHERS);
        platePositionsY[i] = centerY + 120 * sin(i * 2 * M_PI / NUM_PHILOSOPHERS);

        forkPositionsX[i] = (philosopherPositionsX[i] + philosopherPositionsX[(i + 1) % NUM_PHILOSOPHERS]) / 2;
        forkPositionsY[i] = (philosopherPositionsY[i] + philosopherPositionsY[(i + 1) % NUM_PHILOSOPHERS]) / 2;
    }
}


void DrawFace(int x, int y, int state) {
    int faceColor = state == 0 ? 0x0000FF : 0x00FF00;
    int whiteColor = 0xFFFFFF; 
    int eyeColor = 0x000000; 

    FillCircle(panel, x, y, 30, faceColor);

    FillCircle(panel, x - 10, y - 10, 8, whiteColor); 

    FillCircle(panel, x - 10, y - 10, 4, eyeColor);  
  
    FillCircle(panel, x + 10, y - 10, 8, whiteColor); 
    FillCircle(panel, x + 10, y - 10, 4, eyeColor);  


    FillCircle(panel, x, y, 3, eyeColor);

    if (state == 0) {
 
        Line(panel, x - 10, y + 10, x + 10, y + 10, eyeColor);
    }
    else {

        Line(panel, x - 10, y + 10, x - 5, y + 12, eyeColor);
        Line(panel, x - 5, y + 12, x, y + 13, eyeColor);
        Line(panel, x, y + 13, x + 5, y + 12, eyeColor);
        Line(panel, x + 5, y + 12, x + 10, y + 10, eyeColor);
    }
}
void DrawTable(int centerX, int centerY, int radius) {
    FillCircle(panel, centerX, centerY, radius, 0x8B4513);
}
void DrawCenterPlate() {

    int centerX = 400;
    int centerY = 400; 

    FillCircle(panel,
        centerX,    
        centerY,   
        60,         
        0xFFFF00);  
    int redCircleRadius = 30; 
    int redCircleSize = 5;  

    for (int i = 0; i < 5; i++) {
        double angle = i * 2 * M_PI / 5; 
        int x = centerX + redCircleRadius * cos(angle); 
        int y = centerY + redCircleRadius * sin(angle); 

        FillCircle(panel, x, y, redCircleSize, 0xFF0000);
    }
}
void DrawPlates() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        int plateColor = (philosopherStates[i] == 1) ? 0xFFFF00 : 0xFFFFFF; 

        FillCircle(panel, platePositionsX[i], platePositionsY[i], 30, plateColor);

        if (philosopherStates[i] == 1) {
            FillCircle(panel, platePositionsX[i], platePositionsY[i], 15, 0x8B4513);
            FillCircle(panel, platePositionsX[i], platePositionsY[i], 5, 0xFF0000);
        }
    }
}
void DrawForks() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        FillRect(panel, forkPositionsX[i] - 5, forkPositionsY[i] - 20, 10, 40, 0x000000);
    }
}


void DrawPhilosophers() {
    FillRect(panel, 0, 0, 800, 700, 0xFFFFFF);

    DrawTable(400, 400, 200);   
    DrawPlates();              
    DrawForks();               
    DrawCenterPlate();           

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        DrawFace(philosopherPositionsX[i], philosopherPositionsY[i], philosopherStates[i]);
    }

    DisplayImage(FRM1, panel);
}
DWORD WINAPI DrawLoop(LPVOID param) {
    while (1) {
        DrawPhilosophers();
        Sleep(100);
    }
    return 0;
}

DWORD WINAPI PhilosopherThread_Deadlock(LPVOID param) {
    int id = (int)param;
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;

    while (1) {
        philosopherStates[id] = 0;
        Sleep(1000);

        WaitForSingleObject(semaphores[leftFork], INFINITE);
        Sleep(50);
        WaitForSingleObject(semaphores[rightFork], INFINITE);

        philosopherStates[id] = 1;
        Sleep(2000);
    }
    return 0;
}
DWORD WINAPI PhilosopherThread_Semaphore(LPVOID param) {
    int id = (int)param;
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;

    while (1) {
        philosopherStates[id] = 0;
        Sleep(1000);

        WaitForSingleObject(semaphores[leftFork], INFINITE);
        WaitForSingleObject(semaphores[rightFork], INFINITE);

        philosopherStates[id] = 1;
        Sleep(2000);

        ReleaseSemaphore(semaphores[leftFork], 1, NULL);
        ReleaseSemaphore(semaphores[rightFork], 1, NULL);
    }
    return 0;
}

void StartSimulation_Deadlock() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        semaphores[i] = CreateSemaphore(NULL, 1, 1, NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        CreateThread(NULL, 0, PhilosopherThread_Deadlock, (LPVOID)i, 0, NULL);
    }
    CreateThread(NULL, 0, DrawLoop, NULL, 0, NULL);
}
void StartSimulation_Semaphore() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        semaphores[i] = CreateSemaphore(NULL, 1, 1, NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        CreateThread(NULL, 0, PhilosopherThread_Semaphore, (LPVOID)i, 0, NULL);
    }
    CreateThread(NULL, 0, DrawLoop, NULL, 0, NULL);
}

void ICGUI_Create() {
    ICG_MWSize(800, 800);
    ICG_MWTitle("Dining Philosophers Simulation");

    CreateImage(panel, 800, 700, ICB_UINT);
    FRM1 = ICG_FrameMedium(5, 50, 800, 700);

    CalculatePositions(50, 700);
}

void ICGUI_main() {
    ICGUI_Create();

    ICG_Button(420, 10, 180, 30, "Semaforsuz", StartSimulation_Deadlock);
    ICG_Button(200, 10, 180, 30, "Semaforlu", StartSimulation_Semaphore);
}

int main() {
    ICGUI_main();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
