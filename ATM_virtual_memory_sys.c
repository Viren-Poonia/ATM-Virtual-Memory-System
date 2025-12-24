#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define TOTAL_PAGES 6
#define FRAMES 3
#define TLB_SIZE 2

struct PageTable {
    int present;
    int frame;
};

struct TLB {
    int page;
    int frame;
};

int memory[FRAMES];
int fifo_index = 0;
int page_faults = 0;

struct PageTable pageTable[TOTAL_PAGES];
struct TLB tlb[TLB_SIZE];
int tlb_count = 0;

void pauseScreen() {
    printf("\nPress Enter to continue...");
    getchar();
    getchar();
}

void clearScreen() {
    system("cls");
}

void displayScreen(int page) {
    char *screens[] = {
        "LOGIN SCREEN",
        "PIN VERIFICATION",
        "MAIN MENU",
        "BALANCE ENQUIRY",
        "WITHDRAW CASH",
        "MINI STATEMENT"
    };

    printf("\n+-----------------------------+\n");
    printf("|      ATM SCREEN LOADED      |\n");
    printf("+-----------------------------+\n");
    printf("| %-27s |\n", screens[page]);
    printf("+-----------------------------+\n");
}

int searchTLB(int page) {
    for (int i = 0; i < tlb_count; i++)
        if (tlb[i].page == page)
            return tlb[i].frame;
    return -1;
}

void removeFromTLB(int page) {
    for (int i = 0; i < tlb_count; i++) {
        if (tlb[i].page == page) {
            for (int j = i + 1; j < tlb_count; j++)
                tlb[j - 1] = tlb[j];
            tlb_count--;
            return;
        }
    }
}

void updateTLB(int page, int frame) {
    removeFromTLB(page);

    if (tlb_count < TLB_SIZE) {
        tlb[tlb_count].page = page;
        tlb[tlb_count].frame = frame;
        tlb_count++;
    } else {
        for (int i = 1; i < TLB_SIZE; i++)
            tlb[i - 1] = tlb[i];
        tlb[TLB_SIZE - 1].page = page;
        tlb[TLB_SIZE - 1].frame = frame;
    }
}

int handlePageFault(int page) {
    int evicted = memory[fifo_index];

    if (evicted != -1) {
        pageTable[evicted].present = 0;
        pageTable[evicted].frame = -1;
        removeFromTLB(evicted);
    }

    memory[fifo_index] = page;
    pageTable[page].present = 1;
    pageTable[page].frame = fifo_index;

    int frame_used = fifo_index;
    fifo_index = (fifo_index + 1) % FRAMES;
    return frame_used;
}

void printMemory() {
    printf("\n+-----------------------------+\n");
    printf("|        MEMORY FRAMES        |\n");
    printf("+-----------------------------+\n");
    for (int i = 0; i < FRAMES; i++) {
        if (memory[i] != -1)
            printf("| Frame %d : Page %-2d         |\n", i, memory[i]);
        else
            printf("| Frame %d : Empty            |\n", i);
    }
    printf("+-----------------------------+\n");
}

void initialize() {
    for (int i = 0; i < TOTAL_PAGES; i++) {
        pageTable[i].present = 0;
        pageTable[i].frame = -1;
    }

    for (int i = 0; i < FRAMES; i++)
        memory[i] = -1;

    fifo_index = 0;
    page_faults = 0;
    tlb_count = 0;
}

void atmMenu() {
    printf("+=============================+\n");
    printf("|      ATM MAIN MENU          |\n");
    printf("+=============================+\n");
    printf("| 0. Login Screen             |\n");
    printf("| 1. PIN Verification         |\n");
    printf("| 2. Main Menu                |\n");
    printf("| 3. Balance Enquiry          |\n");
    printf("| 4. Withdraw Cash            |\n");
    printf("| 5. Mini Statement           |\n");
    printf("| 6. Exit                     |\n");
    printf("+=============================+\n");
    printf("Select Option: ");
}

int main() {
    int choice;
    initialize();

    while (1) {
        clearScreen();
        atmMenu();
        scanf("%d", &choice);

        if (choice == 6)
            break;

        if (choice < 0 || choice >= TOTAL_PAGES) {
            printf("\nInvalid Option!");
            pauseScreen();
            continue;
        }

        int frame = searchTLB(choice);

        if (frame != -1) {
            printf("\nTLB HIT!");
        } else if (pageTable[choice].present) {
            printf("\nPAGE TABLE HIT!");
            updateTLB(choice, pageTable[choice].frame);
        } else {
            printf("\nPAGE FAULT OCCURRED!");
            page_faults++;
            frame = handlePageFault(choice);
            updateTLB(choice, frame);
        }

        displayScreen(choice);
        printMemory();
        pauseScreen();
    }

    clearScreen();
    printf("ATM SESSION ENDED\n");
    printf("TOTAL PAGE FAULTS: %d\n", page_faults);
    return 0;
}