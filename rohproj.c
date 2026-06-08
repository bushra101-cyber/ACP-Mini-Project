#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- Canvas dimensions --- */
#define ROWS   25
#define COLS   60
#define BLANK  '_'
#define PIXEL  '*'

/* --- Max objects stored --- */
#define MAX_OBJECTS 50

/* --- Shape type codes --- */
#define SHAPE_LINE      1
#define SHAPE_RECTANGLE 2
#define SHAPE_CIRCLE    3
#define SHAPE_TRIANGLE  4

/* --- Object descriptor --- */
typedef struct {
    int  type;
    int  id;
    int  active;
    int  x1, y1;
    int  x2, y2;
    int  x3, y3;
    int  r;
} Object;

/* --- Global state --- */
char   canvas[ROWS][COLS];
Object objects[MAX_OBJECTS];
int    objCount = 0;
int    nextId   = 1;

/* ============================================================
 *  CANVAS UTILITIES
 * ============================================================ */

void initCanvas(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = BLANK;
}

void plot(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = PIXEL;
}

void erase(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = BLANK;
}

void displayCanvas(void) {
    printf("\n");
    printf("     ");
    for (int c = 0; c < COLS; c += 5)
        printf("%-5d", c);
    printf("\n     ");
    for (int c = 0; c < COLS; c++)
        printf("-");
    printf("\n");
    for (int r = 0; r < ROWS; r++) {
        printf("%3d |", r);
        for (int c = 0; c < COLS; c++)
            printf("%c", canvas[r][c]);
        printf("|\n");
    }
    printf("     ");
    for (int c = 0; c < COLS; c++)
        printf("-");
    printf("\n\n");
}

/* ============================================================
 *  DRAWING PRIMITIVES
 * ============================================================ */

void drawLine(int r1, int c1, int r2, int c2) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        plot(r1, c1);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

void drawRectangle(int r1, int c1, int r2, int c2) {
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }
    drawLine(r1, c1, r1, c2);
    drawLine(r2, c1, r2, c2);
    drawLine(r1, c1, r2, c1);
    drawLine(r1, c2, r2, c2);
}

void drawCircle(int cr, int cc, int radius) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        int sx = x * 2;
        int sy = y * 2;
        plot(cr + x, cc + sy); plot(cr + x, cc - sy);
        plot(cr - x, cc + sy); plot(cr - x, cc - sy);
        plot(cr + y, cc + sx); plot(cr + y, cc - sx);
        plot(cr - y, cc + sx); plot(cr - y, cc - sx);
        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void drawTriangle(int r1, int c1, int r2, int c2, int r3, int c3) {
    drawLine(r1, c1, r2, c2);
    drawLine(r2, c2, r3, c3);
    drawLine(r3, c3, r1, c1);
}

/* ============================================================
 *  OBJECT MANAGEMENT
 * ============================================================ */

void drawObject(const Object *o) {
    switch (o->type) {
        case SHAPE_LINE:
            drawLine(o->y1, o->x1, o->y2, o->x2);
            break;
        case SHAPE_RECTANGLE:
            drawRectangle(o->y1, o->x1, o->y2, o->x2);
            break;
        case SHAPE_CIRCLE:
            drawCircle(o->y1, o->x1, o->r);
            break;
        case SHAPE_TRIANGLE:
            drawTriangle(o->y1, o->x1, o->y2, o->x2, o->y3, o->x3);
            break;
    }
}

void redrawAll(void) {
    initCanvas();
    for (int i = 0; i < objCount; i++)
        if (objects[i].active)
            drawObject(&objects[i]);
}

const char *shapeName(int type) {
    switch (type) {
        case SHAPE_LINE:      return "Line";
        case SHAPE_RECTANGLE: return "Rectangle";
        case SHAPE_CIRCLE:    return "Circle";
        case SHAPE_TRIANGLE:  return "Triangle";
        default:              return "Unknown";
    }
}

void listObjects(void) {
    int found = 0;
    printf("\n  ID   Type        Parameters\n");
    printf("  ---- ----------- --------------------------\n");
    for (int i = 0; i < objCount; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Object *o = &objects[i];
        printf("  %3d  %-11s", o->id, shapeName(o->type));
        switch (o->type) {
            case SHAPE_LINE:
                printf("(%d,%d) -> (%d,%d)\n",
                       o->x1,o->y1, o->x2,o->y2);
                break;
            case SHAPE_RECTANGLE:
                printf("TL(%d,%d)  BR(%d,%d)\n",
                       o->x1,o->y1, o->x2,o->y2);
                break;
            case SHAPE_CIRCLE:
                printf("Centre(%d,%d)  R=%d\n",
                       o->x1,o->y1, o->r);
                break;
            case SHAPE_TRIANGLE:
                printf("(%d,%d) (%d,%d) (%d,%d)\n",
                       o->x1,o->y1, o->x2,o->y2, o->x3,o->y3);
                break;
        }
    }
    if (!found)
        printf("  No objects on canvas.\n");
    printf("\n");
}

/* --- Input helpers --- */
void flushInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readInt(const char *prompt) {
    int val;
    printf("  %s: ", prompt);
    while (scanf("%d", &val) != 1) {
        flushInput();
        printf("  Invalid - please enter a number.\n  %s: ", prompt);
    }
    flushInput();
    return val;
}

/* ============================================================
 *  ADD OPERATIONS
 * ============================================================ */

void addLine(void) {
    if (objCount >= MAX_OBJECTS) {
        printf("  Canvas full!\n"); return;
    }
    printf("\n  --- Add Line ---\n");
    printf("  Enter start point (col, row) and end point (col, row)\n");
    Object o = {0};
    o.type   = SHAPE_LINE;
    o.id     = nextId++;
    o.active = 1;
    o.x1 = readInt("Start col");
    o.y1 = readInt("Start row");
    o.x2 = readInt("End   col");
    o.y2 = readInt("End   row");
    objects[objCount++] = o;
    drawObject(&o);
    printf("  Line added (ID=%d).\n", o.id);
}

void addRectangle(void) {
    if (objCount >= MAX_OBJECTS) {
        printf("  Canvas full!\n"); return;
    }
    printf("\n  --- Add Rectangle ---\n");
    printf("  Enter top-left (col, row) and bottom-right (col, row)\n");
    Object o = {0};
    o.type   = SHAPE_RECTANGLE;
    o.id     = nextId++;
    o.active = 1;
    o.x1 = readInt("Top-left  col");
    o.y1 = readInt("Top-left  row");
    o.x2 = readInt("Bot-right col");
    o.y2 = readInt("Bot-right row");
    objects[objCount++] = o;
    drawObject(&o);
    printf("  Rectangle added (ID=%d).\n", o.id);
}

void addCircle(void) {
    if (objCount >= MAX_OBJECTS) {
        printf("  Canvas full!\n"); return;
    }
    printf("\n  --- Add Circle ---\n");
    Object o = {0};
    o.type   = SHAPE_CIRCLE;
    o.id     = nextId++;
    o.active = 1;
    o.x1 = readInt("Centre col");
    o.y1 = readInt("Centre row");
    o.r  = readInt("Radius    ");
    objects[objCount++] = o;
    drawObject(&o);
    printf("  Circle added (ID=%d).\n", o.id);
}

void addTriangle(void) {
    if (objCount >= MAX_OBJECTS) {
        printf("  Canvas full!\n"); return;
    }
    printf("\n  --- Add Triangle ---\n");
    printf("  Enter three vertices as (col, row)\n");
    Object o = {0};
    o.type   = SHAPE_TRIANGLE;
    o.id     = nextId++;
    o.active = 1;
    o.x1 = readInt("Vertex A col");
    o.y1 = readInt("Vertex A row");
    o.x2 = readInt("Vertex B col");
    o.y2 = readInt("Vertex B row");
    o.x3 = readInt("Vertex C col");
    o.y3 = readInt("Vertex C row");
    objects[objCount++] = o;
    drawObject(&o);
    printf("  Triangle added (ID=%d).\n", o.id);
}

/* ============================================================
 *  DELETE OPERATION
 * ============================================================ */

void deleteObject(void) {
    listObjects();
    int id = readInt("Enter ID to delete (0 to cancel)");
    if (id == 0) return;
    for (int i = 0; i < objCount; i++) {
        if (objects[i].id == id && objects[i].active) {
            objects[i].active = 0;
            redrawAll();
            printf("  Object ID=%d deleted.\n", id);
            return;
        }
    }
    printf("  ID=%d not found.\n", id);
}

/* ============================================================
 *  MODIFY OPERATION
 * ============================================================ */

void modifyObject(void) {
    listObjects();
    int id = readInt("Enter ID to modify (0 to cancel)");
    if (id == 0) return;

    for (int i = 0; i < objCount; i++) {
        Object *o = &objects[i];
        if (o->id != id || !o->active) continue;

        printf("\n  Modifying %s (ID=%d) - enter new parameters:\n",
               shapeName(o->type), id);

        switch (o->type) {
            case SHAPE_LINE:
                o->x1 = readInt("New start col");
                o->y1 = readInt("New start row");
                o->x2 = readInt("New end   col");
                o->y2 = readInt("New end   row");
                break;
            case SHAPE_RECTANGLE:
                o->x1 = readInt("New top-left  col");
                o->y1 = readInt("New top-left  row");
                o->x2 = readInt("New bot-right col");
                o->y2 = readInt("New bot-right row");
                break;
            case SHAPE_CIRCLE:
                o->x1 = readInt("New centre col");
                o->y1 = readInt("New centre row");
                o->r  = readInt("New radius   ");
                break;
            case SHAPE_TRIANGLE:
                o->x1 = readInt("New vertex A col");
                o->y1 = readInt("New vertex A row");
                o->x2 = readInt("New vertex B col");
                o->y2 = readInt("New vertex B row");
                o->x3 = readInt("New vertex C col");
                o->y3 = readInt("New vertex C row");
                break;
        }
        redrawAll();
        printf("  Object ID=%d modified.\n", id);
        return;
    }
    printf("  ID=%d not found.\n", id);
}

/* ============================================================
 *  CLEAR CANVAS
 * ============================================================ */

void clearAll(void) {
    char confirm;
    printf("  Clear ALL objects? (y/n): ");
    scanf(" %c", &confirm);
    flushInput();
    if (confirm == 'y' || confirm == 'Y') {
        objCount = 0;
        nextId   = 1;
        initCanvas();
        printf("  Canvas cleared.\n");
    }
}

/* ============================================================
 *  MENUS
 * ============================================================ */

void printBanner(void) {
    printf("\n");
    printf("  === 2D GRAPHICS EDITOR (C / ACP) ===\n");
    printf("  Canvas: %d rows x %d cols  |  bg='_'  obj='*'\n\n", ROWS, COLS);
}

void addMenu(void) {
    int choice;
    do {
        printf("\n  ADD OBJECT\n");
        printf("  1. Line\n");
        printf("  2. Rectangle\n");
        printf("  3. Circle\n");
        printf("  4. Triangle\n");
        printf("  0. Back\n");
        choice = readInt("Choice");
        switch (choice) {
            case 1: addLine();       break;
            case 2: addRectangle();  break;
            case 3: addCircle();     break;
            case 4: addTriangle();   break;
            case 0: break;
            default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);
}

void mainMenu(void) {
    int choice;
    do {
        printf("\n  MAIN MENU\n");
        printf("  1. Display canvas\n");
        printf("  2. Add object\n");
        printf("  3. Delete object\n");
        printf("  4. Modify object\n");
        printf("  5. List all objects\n");
        printf("  6. Clear canvas\n");
        printf("  0. Exit\n");
        choice = readInt("Choice");
        switch (choice) {
            case 1: displayCanvas();   break;
            case 2: addMenu();         break;
            case 3: deleteObject();    break;
            case 4: modifyObject();    break;
            case 5: listObjects();     break;
            case 6: clearAll();        break;
            case 0: printf("  Goodbye!\n"); break;
            default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);
}

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    printBanner();
    initCanvas();
    mainMenu();
    return 0;
}